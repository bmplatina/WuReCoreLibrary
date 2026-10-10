// © Platina Production 2017~2024

#include "AsyncLevelStreamingSubsystem.h"
#include "LevelListStruct.h"
#include "Engine/World.h"
#include "Engine/Level.h"
#include "Engine/DataTable.h"
#include "Engine/LevelStreamingDynamic.h"
#include "TimerManager.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	// 언로드 대기 구간이 전체 진행률에서 차지하는 비율 (0~10%)
	constexpr float UnloadProgressBand = 0.1f;
	// 언로드 대기 진행률 연출에 쓰는 예상 소요 시간(실측 불가라 근사치)
	constexpr float UnloadEstimatedSeconds = 0.5f;
	// 로드 완료 후 Visible 전환을 기다리는 최대 시간. 넘기면 경고 로그를 남기고 그냥 완료 처리
	constexpr float VisibilityWaitTimeoutSeconds = 5.f;

	// Row의 LevelAsset 배열에서 유효한 레벨 패키지 이름을 (중복 제거해서) 순서대로 모음
	void GatherPackages(const FLevelListStruct& Row, TArray<FName>& OutPackages)
	{
		for (const TSoftObjectPtr<UWorld>& Asset : Row.LevelAsset)
		{
			if (Asset.IsNull())
			{
				continue;
			}
			OutPackages.AddUnique(Asset.ToSoftObjectPath().GetLongPackageFName());
		}
	}
}

void UAsyncLevelStreamingSubsystem::Deinitialize()
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ProgressTimerHandle);
		World->GetTimerManager().ClearTimer(UnloadWaitTimerHandle);
	}
	Super::Deinitialize();
}

bool UAsyncLevelStreamingSubsystem::RequestLoadLevelByRowName(const FName RowName, const FVector Location,
                                                              const FRotator Rotation, FText& DisplayedLevelText)
{
	if (!IsValid(LevelTable))
	{
		WURELOG(Error, TEXT("RequestLoadLevelByRowName: LevelTable is not set on this subsystem"));
		return false;
	}

	const FLevelListStruct* Row = LevelTable->FindRow<FLevelListStruct>(RowName, TEXT("RequestLoadLevelByRowName"));
	if (Row == nullptr)
	{
		WURELOG(Error, TEXT("RequestLoadLevelByRowName: RowName '%s' not found in LevelTable"), *RowName.ToString());
		return false;
	}

	TArray<FName> NewPackages;
	GatherPackages(*Row, NewPackages);
	if (NewPackages.IsEmpty())
	{
		WURELOG(Error, TEXT("RequestLoadLevelByRowName: Row '%s' has no valid LevelAsset"), *RowName.ToString());
		return false;
	}
	DisplayedLevelText = Row->DisplayNameForLoadingScreen;

	// 같은 row가 이미 로딩/큐잉 중이면 중복 요청 무시 (바인딩된 위젯은 진행 중인 요청의 완료 브로드캐스트를 받음)
	if (LoadingRowName == RowName)
	{
		return true;
	}

	// 새 집합과 겹치는 레벨도 포함해서, 현재 추적 중인 레벨을 전부 언로드 (전부 다시 로드)
	TArray<FName> ToUnload = TargetPackages.Array();
	for (const TPair<FName, TObjectPtr<ULevelStreamingDynamic>>& Pair : LoadedLevels)
	{
		ToUnload.AddUnique(Pair.Key);
	}
	for (const FName& Package : ToUnload)
	{
		UnloadPackage(Package);
	}
	if (!ToUnload.IsEmpty())
	{
		ActiveLevelRowName = NAME_None;
	}

	TargetPackages.Empty();
	TargetPackages.Append(NewPackages);
	LoadingRowName = RowName;
	ReadyPawn.Reset();

	// 언로드가 진행 중이면(이전 요청 것 포함): 완전히 사라진 뒤에 이 요청을 이어서 시작
	LevelsPendingUnload.RemoveAll([](const TObjectPtr<ULevelStreamingDynamic>& Level)
	{
		return !IsValid(Level) || !Level->IsLevelLoaded();
	});
	const bool bMustWaitForUnload = !LevelsPendingUnload.IsEmpty() || !ToUnload.IsEmpty();
	if (bMustWaitForUnload)
	{
		bHasQueuedLoad = true;
		QueuedRowName = RowName;
		QueuedLocation = Location;
		QueuedRotation = Rotation;

		if (UWorld* World = GetWorld())
		{
			UnloadWaitStartTime = World->GetTimeSeconds();
			OnLevelStreamingProgress.Broadcast(0.f, false);

			if (!World->GetTimerManager().IsTimerActive(UnloadWaitTimerHandle))
			{
				World->GetTimerManager().SetTimer(
					UnloadWaitTimerHandle, this, &UAsyncLevelStreamingSubsystem::CheckUnloadThenStartQueuedLoad,
					0.05f, true);
			}
		}

		return true;
	}

	bCameFromUnloadWait = false;
	return BeginLoad(RowName, NewPackages, Location, Rotation);
}

void UAsyncLevelStreamingSubsystem::CheckUnloadThenStartQueuedLoad()
{
	UWorld* World = GetWorld();

	LevelsPendingUnload.RemoveAll([](const TObjectPtr<ULevelStreamingDynamic>& Level)
	{
		return !IsValid(Level) || !Level->IsLevelLoaded();
	});

	// 아직 언로드 중인 레벨이 있으면: 경과 시간 기반으로 근사 진행률(0~UnloadProgressBand)을 브로드캐스트하고 다음 틱에 다시 확인
	if (!LevelsPendingUnload.IsEmpty())
	{
		if (World != nullptr)
		{
			const float Elapsed = static_cast<float>(World->GetTimeSeconds() - UnloadWaitStartTime);
			const float UnloadProgress = FMath::Clamp(Elapsed / UnloadEstimatedSeconds, 0.f, 0.95f) *
				UnloadProgressBand;
			OnLevelStreamingProgress.Broadcast(UnloadProgress, false);
		}
		return;
	}

	if (World != nullptr)
	{
		World->GetTimerManager().ClearTimer(UnloadWaitTimerHandle);
	}

	// 언로드 완료 = 언로드 구간의 끝. 이 지점부터 로드 구간(UnloadProgressBand~1.0)이 이어짐
	OnLevelStreamingProgress.Broadcast(UnloadProgressBand, false);

	if (!bHasQueuedLoad)
	{
		return;
	}
	bHasQueuedLoad = false;

	const FLevelListStruct* Row = IsValid(LevelTable)
		                              ? LevelTable->FindRow<FLevelListStruct>(
			                              QueuedRowName, TEXT("CheckUnloadThenStartQueuedLoad"))
		                              : nullptr;
	if (Row == nullptr)
	{
		WURELOG(Error, TEXT("CheckUnloadThenStartQueuedLoad: Row '%s' no longer available"), *QueuedRowName.ToString());
		LoadingRowName = NAME_None;
		return;
	}

	TArray<FName> NewPackages;
	GatherPackages(*Row, NewPackages);
	bCameFromUnloadWait = true;
	BeginLoad(QueuedRowName, NewPackages, QueuedLocation, QueuedRotation);
}

bool UAsyncLevelStreamingSubsystem::BeginLoad(const FName RowName, const TArray<FName>& NewPackages,
                                              const FVector Location, const FRotator Rotation)
{
	VisibilityWaitStartTime = -1.0;

	int32 WantedCount = 0;
	for (const FName& Package : NewPackages)
	{
		// 정상 흐름에서는 요청 시 전부 언로드되므로 비어 있음. 방어용으로 이미 추적 중인 패키지는 중복 로드하지 않음
		if (LoadedLevels.Contains(Package))
		{
			continue;
		}
		++WantedCount;

		bool bSuccess = false;
		ULevelStreamingDynamic* NewLevel = ULevelStreamingDynamic::LoadLevelInstance(
			this, Package.ToString(), Location, Rotation, bSuccess);

		if (!bSuccess || NewLevel == nullptr)
		{
			WURELOG(Error, TEXT("BeginLoad: LoadLevelInstance failed for Row '%s' (package '%s')"),
			        *RowName.ToString(), *Package.ToString());
			continue;
		}

		// 로드만 먼저 진행, 완료 후 한꺼번에 Visible 전환 (팝인 방지)
		NewLevel->SetShouldBeVisible(false);

		PendingLevels.Add(NewLevel);
		LoadedLevels.Add(Package, NewLevel);
	}

	if (PendingLevels.IsEmpty())
	{
		if (WantedCount > 0)
		{
			// 로드하려 했지만 하나도 시작하지 못함
			LoadingRowName = NAME_None;
			bCameFromUnloadWait = false;
			return false;
		}

		// 필요한 레벨이 전부 이미 로드돼 있음: 로드할 게 없으니 즉시 완료
		bCameFromUnloadWait = false;
		FinalizeRow(RowName);
		OnLevelStreamingProgress.Broadcast(1.f, true);
		return true;
	}

	PendingLevelsOriginalCount = PendingLevels.Num() + FinishedLevels.Num();

	if (UWorld* World = GetWorld())
	{
		if (!World->GetTimerManager().IsTimerActive(ProgressTimerHandle))
		{
			World->GetTimerManager().SetTimer(
				ProgressTimerHandle, this, &UAsyncLevelStreamingSubsystem::UpdateProgress, 0.05f, true);
		}
	}

	return true;
}

void UAsyncLevelStreamingSubsystem::FinalizeRow(const FName RowName)
{
	LoadingRowName = NAME_None;
	ActiveLevelRowName = RowName;
	ApplyPawnForRow(RowName);
	RestorePawnAwaitingPlacement();
	NotifyPlayerPawnReady();
}

void UAsyncLevelStreamingSubsystem::UnloadPackage(const FName PackageName)
{
	TargetPackages.Remove(PackageName);

	const TObjectPtr<ULevelStreamingDynamic>* Found = LoadedLevels.Find(PackageName);
	if (Found == nullptr)
	{
		return;
	}

	ULevelStreamingDynamic* Level = Found->Get();
	LoadedLevels.Remove(PackageName);
	if (!IsValid(Level))
	{
		return;
	}

	Level->SetIsRequestingUnloadAndRemoval(true);
	LevelsPendingUnload.AddUnique(Level);

	const auto IsSameLevel = [Level](const TObjectPtr<ULevelStreamingDynamic>& Other) { return Other == Level; };
	PendingLevels.RemoveAll(IsSameLevel);
	FinishedLevels.RemoveAll(IsSameLevel);
}

void UAsyncLevelStreamingSubsystem::UnloadLevelByRowName(const FName RowName)
{
	if (IsValid(LevelTable))
	{
		if (const FLevelListStruct* Row = LevelTable->FindRow<FLevelListStruct>(RowName, TEXT("UnloadLevelByRowName")))
		{
			TArray<FName> Packages;
			GatherPackages(*Row, Packages);
			for (const FName& Package : Packages)
			{
				UnloadPackage(Package);
			}
		}
	}

	if (ActiveLevelRowName == RowName)
	{
		ActiveLevelRowName = NAME_None;
	}
	if (LoadingRowName == RowName)
	{
		LoadingRowName = NAME_None;
	}
}

void UAsyncLevelStreamingSubsystem::UnloadCurrentLevel()
{
	UnloadLevelByRowName(ActiveLevelRowName);
}

void UAsyncLevelStreamingSubsystem::TryApplyPawnForCurrentLevel()
{
	// 타깃 레벨에 바로 PIE했을 때만 개입 (정상 플로우는 RequestLoadLevelByRowName -> FinalizeRow를 거침)
	// (AWuReGameModeBase를 쓰면 Pawn은 이미 월드 BeginPlay 전에 스폰/Possess돼 있으므로 여기서는 재사용 + 위치 보정만 일어남)
	const UWorld* World = GetWorld();
	if (World != nullptr && World->IsPlayInEditor())
	{
		ActiveLevelRowName = *UGameplayStatics::GetCurrentLevelName(this);
		ApplyPawnForRow(ActiveLevelRowName);
	}

	NotifyPlayerPawnReady();
}

bool UAsyncLevelStreamingSubsystem::IsPlayerPawnReady() const
{
	return ReadyPawn.IsValid() && LoadingRowName.IsNone() && !bHasQueuedLoad;
}

FName UAsyncLevelStreamingSubsystem::GetEffectiveRowName() const
{
	if (!ActiveLevelRowName.IsNone())
	{
		return ActiveLevelRowName;
	}

	const FName PersistentLevelName(*UGameplayStatics::GetCurrentLevelName(this));
	return FindRow(PersistentLevelName, TEXT("GetEffectiveRowName"), false) ? PersistentLevelName : NAME_None;
}

TSubclassOf<APawn> UAsyncLevelStreamingSubsystem::GetPawnClassForRow(const FName RowName) const
{
	const FLevelListStruct* Row = FindRow(RowName, TEXT("GetPawnClassForRow"), false);
	return Row ? Row->PawnClassOverride : nullptr;
}

AActor* UAsyncLevelStreamingSubsystem::FindSpawnPointForRow(const FName RowName) const
{
	const FLevelListStruct* Row = FindRow(RowName, TEXT("FindSpawnPointForRow"), false);
	if (Row == nullptr || Row->TargetSpawnTag.IsNone())
	{
		return nullptr;
	}

	TArray<const ULevel*> RowLevels;
	GatherRowLevels(*Row, RowLevels);
	return FindSpawnPointInLevels(*Row, RowLevels);
}

const FLevelListStruct* UAsyncLevelStreamingSubsystem::FindRow(const FName RowName, const TCHAR* ContextString,
                                                               bool bWarnIfMissing) const
{
	if (!IsValid(LevelTable) || RowName.IsNone())
	{
		return nullptr;
	}
	return LevelTable->FindRow<FLevelListStruct>(RowName, ContextString, bWarnIfMissing);
}

void UAsyncLevelStreamingSubsystem::GatherRowLevels(const FLevelListStruct& Row, TArray<const ULevel*>& OutLevels) const
{
	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	TArray<FName> Packages;
	GatherPackages(Row, Packages);
	for (const FName& Package : Packages)
	{
		if (const TObjectPtr<ULevelStreamingDynamic>* Found = LoadedLevels.Find(Package))
		{
			if (const ULevel* LoadedLevel = (*Found) ? (*Found)->GetLoadedLevel() : nullptr)
			{
				OutLevels.Add(LoadedLevel);
			}
		}
		// 서브시스템 스트리밍을 거치지 않고(예: 타깃 레벨에 바로 PIE) 이미 퍼시스턴트 레벨로 열려 있는 경우
		// PIE 월드의 패키지 이름은 "UEDPIE_0_" 같은 프리픽스가 붙으므로 비교 전에 제거
		else if (World->PersistentLevel != nullptr &&
		         FName(*UWorld::RemovePIEPrefix(World->PersistentLevel->GetOutermost()->GetName())) == Package)
		{
			OutLevels.Add(World->PersistentLevel);
		}
	}
}

AActor* UAsyncLevelStreamingSubsystem::FindSpawnPointInLevels(const FLevelListStruct& Row,
                                                              const TArray<const ULevel*>& Levels, int32* OutCount)
{
	AActor* SpawnPoint = nullptr;
	int32 Count = 0;
	if (!Row.TargetSpawnTag.IsNone())
	{
		for (const ULevel* Level : Levels)
		{
			for (AActor* Actor : Level->Actors)
			{
				if (IsValid(Actor) && !Actor->IsA<APawn>() && Actor->ActorHasTag(Row.TargetSpawnTag))
				{
					++Count;
					if (SpawnPoint == nullptr)
					{
						SpawnPoint = Actor;
					}
				}
			}
		}
	}

	if (OutCount != nullptr)
	{
		*OutCount = Count;
	}
	return SpawnPoint;
}

void UAsyncLevelStreamingSubsystem::PreparePawnBeforeVisible(const FName RowName)
{
	const FLevelListStruct* Row = FindRow(RowName, TEXT("PreparePawnBeforeVisible"));
	if (Row == nullptr || Row->PawnClassOverride == nullptr)
	{
		return;
	}

	UWorld* World = GetWorld();
	APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;
	if (PlayerController == nullptr)
	{
		return;
	}

	// 같은 클래스 Pawn을 이미 조종 중이면 그대로 재사용 (Visible 후 ApplyPawnForRow에서 이동만)
	APawn* OldPawn = PlayerController->GetPawn();
	if (IsValid(OldPawn) && OldPawn->IsA(Row->PawnClassOverride))
	{
		return;
	}

	// 로드됐지만 아직 월드에 추가되지 않은 레벨: 액터 객체는 있지만 BeginPlay/레벨 Transform 적용 전
	const AActor* SpawnPoint = nullptr;
	FTransform SpawnPointLevelTransform = FTransform::Identity;
	for (const ULevelStreamingDynamic* StreamingLevel : FinishedLevels)
	{
		const ULevel* LoadedLevel = IsValid(StreamingLevel) ? StreamingLevel->GetLoadedLevel() : nullptr;
		if (LoadedLevel == nullptr)
		{
			continue;
		}

		for (const AActor* Actor : LoadedLevel->Actors)
		{
			if (!IsValid(Actor))
			{
				continue;
			}

			// 레벨에 배치된 Pawn을 Possess하는 row는 Visible 후(초기화 완료 후) ApplyPawnForRow에 맡김
			if (Actor->IsA(Row->PawnClassOverride))
			{
				return;
			}

			if (SpawnPoint == nullptr && !Row->TargetSpawnTag.IsNone() && !Actor->IsA<APawn>() &&
				Actor->ActorHasTag(Row->TargetSpawnTag))
			{
				SpawnPoint = Actor;
				SpawnPointLevelTransform = StreamingLevel->LevelTransform;
			}
		}
	}

	// 목적지 액터의 월드 위치는 레벨 Transform이 아직 적용 전이라 직접 계산 (정확한 위치는 Visible 후 ApplyPawnForRow에서 재배치)
	FTransform SpawnTransform = FTransform::Identity;
	if (SpawnPoint != nullptr && SpawnPoint->GetRootComponent() != nullptr)
	{
		const FTransform EstimatedTransform = SpawnPoint->GetRootComponent()->GetRelativeTransform() *
			SpawnPointLevelTransform;
		SpawnTransform = FTransform(FRotator(0.f, EstimatedTransform.Rotator().Yaw, 0.f),
		                            EstimatedTransform.GetLocation());
	}
	else if (IsValid(OldPawn))
	{
		SpawnTransform = FTransform(OldPawn->GetActorRotation(), OldPawn->GetActorLocation());
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	APawn* NewPawn = World->SpawnActor<APawn>(Row->PawnClassOverride, SpawnTransform, SpawnParams);
	if (NewPawn == nullptr)
	{
		return;
	}

	// 바닥(새 레벨)이 아직 월드에 없으므로 배치될 때까지 이동(중력 포함)을 멈춰둠
	if (UPawnMovementComponent* MovementComponent = NewPawn->GetMovementComponent())
	{
		MovementComponent->Deactivate();
	}
	PawnAwaitingPlacement = NewPawn;

	PlayerController->Possess(NewPawn);
	if (IsValid(OldPawn))
	{
		OldPawn->Destroy();
	}

	WURELOG(Display, TEXT("PreparePawnBeforeVisible: Row '%s' pre-spawned pawn=%s at %s"),
	        *RowName.ToString(), *NewPawn->GetName(), *SpawnTransform.GetLocation().ToString());
}

void UAsyncLevelStreamingSubsystem::RestorePawnAwaitingPlacement()
{
	if (APawn* Pawn = PawnAwaitingPlacement.Get())
	{
		if (UPawnMovementComponent* MovementComponent = Pawn->GetMovementComponent())
		{
			MovementComponent->Activate();
			MovementComponent->StopMovementImmediately();
		}
	}
	PawnAwaitingPlacement.Reset();
}

void UAsyncLevelStreamingSubsystem::NotifyPlayerPawnReady()
{
	const UWorld* World = GetWorld();
	const APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;
	APawn* Pawn = PlayerController ? PlayerController->GetPawn() : nullptr;

	ReadyPawn = Pawn;
	if (IsValid(Pawn))
	{
		OnPlayerPawnReady.Broadcast(Pawn);
	}
}

void UAsyncLevelStreamingSubsystem::ApplyPawnForRow(const FName RowName)
{
	const FLevelListStruct* Row = FindRow(RowName, TEXT("ApplyPawnForRow"));
	if (Row == nullptr)
	{
		return;
	}

	// PawnClassOverride가 비어 있어도 TargetSpawnTag가 있으면 "현재 Pawn을 그 위치로 이동"은 수행
	const bool bWantsPawn = Row->PawnClassOverride != nullptr;
	if (!bWantsPawn && Row->TargetSpawnTag.IsNone())
	{
		return;
	}

	UWorld* World = GetWorld();
	APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;
	if (World == nullptr || PlayerController == nullptr)
	{
		return;
	}

	// 이 row에 속한 레벨들 "안에서만" 탐색 (다른 로드된 레벨의 동명 태그/동일 클래스 Pawn과 헷갈리지 않도록)
	TArray<const ULevel*> RowLevels;
	GatherRowLevels(*Row, RowLevels);
	if (RowLevels.IsEmpty())
	{
		WURELOG(Warning, TEXT("ApplyPawnForRow: no loaded level found for Row '%s'"), *RowName.ToString());
		return;
	}

	// - TargetSpawnTag가 붙은 (Pawn이 아닌) 액터 = 스폰/이동 목적지. Actor Tags 배열에 있어야 함 (PlayerStart의 PlayerStartTag 아님)
	// - 레벨에 이미 같은 클래스의 Pawn이 있으면 새로 스폰하지 않고 그걸 Possess (TargetSpawnTag가 붙은 Pawn 우선)
	int32 SpawnPointCount = 0;
	const AActor* SpawnPoint = FindSpawnPointInLevels(*Row, RowLevels, &SpawnPointCount);
	APawn* ExistingPawn = nullptr;
	bool bExistingHasTag = false;
	for (const ULevel* RowLevel : RowLevels)
	{
		for (AActor* Actor : RowLevel->Actors)
		{
			APawn* CandidatePawn = bWantsPawn ? Cast<APawn>(Actor) : nullptr;
			if (!IsValid(CandidatePawn) || !CandidatePawn->IsA(Row->PawnClassOverride))
			{
				continue;
			}
			const bool bHasSpawnTag = !Row->TargetSpawnTag.IsNone() && CandidatePawn->ActorHasTag(Row->TargetSpawnTag);

			// 이미 다른 컨트롤러(AI 등)가 잡고 있는 Pawn은 제외
			AController* CandidateController = CandidatePawn->GetController();
			if (CandidateController != nullptr && CandidateController != PlayerController)
			{
				continue;
			}

			if (bHasSpawnTag && !bExistingHasTag)
			{
				ExistingPawn = CandidatePawn;
				bExistingHasTag = true;
			}
			else if (ExistingPawn == nullptr)
			{
				ExistingPawn = CandidatePawn;
			}
		}
	}

	if (!Row->TargetSpawnTag.IsNone())
	{
		if (SpawnPoint == nullptr)
		{
			WURELOG(Warning, TEXT("ApplyPawnForRow: Row '%s' has no actor tagged '%s' in its levels (check Actor Tags)"),
			        *RowName.ToString(), *Row->TargetSpawnTag.ToString());
		}
		else if (SpawnPointCount > 1)
		{
			WURELOG(Warning, TEXT("ApplyPawnForRow: Row '%s' has %d actors tagged '%s'; using '%s'"),
			        *RowName.ToString(), SpawnPointCount, *Row->TargetSpawnTag.ToString(), *SpawnPoint->GetName());
		}
	}

	// 스케일은 복사하지 않고 위치/회전만 사용 (PlayerStart 등의 스케일이 Pawn에 전이되는 것 방지)
	const FVector SpawnLocation = SpawnPoint ? SpawnPoint->GetActorLocation() : FVector::ZeroVector;
	const FRotator SpawnRotation = SpawnPoint ? SpawnPoint->GetActorRotation() : FRotator::ZeroRotator;

	APawn* OldPawn = PlayerController->GetPawn();
	APawn* PawnToPossess = ExistingPawn;
	bool bSpawned = false;
	if (PawnToPossess == nullptr)
	{
		// 같은 클래스 Pawn을 이미 조종 중이면 새로 스폰하지 않고 재사용 (다른 액터가 잡아둔 Pawn 참조가 끊기지 않도록)
		if (bWantsPawn && !(IsValid(OldPawn) && OldPawn->IsA(Row->PawnClassOverride)))
		{
			// 목적지가 없으면 기존 Pawn 위치에 스폰 (월드 원점에 떨어지는 것 방지)
			const FTransform SpawnTransform = SpawnPoint
				                                  ? FTransform(SpawnRotation, SpawnLocation)
				                                  : (OldPawn ? FTransform(OldPawn->GetActorRotation(), OldPawn->GetActorLocation())
					                                  : FTransform::Identity);

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			PawnToPossess = World->SpawnActor<APawn>(Row->PawnClassOverride, SpawnTransform, SpawnParams);
			bSpawned = true;
		}
		else
		{
			// Pawn 교체 없이(또는 같은 클래스 Pawn 재사용) 현재 Pawn만 목적지로 이동
			PawnToPossess = OldPawn;
		}
	}
	if (PawnToPossess == nullptr)
	{
		return;
	}

	if (OldPawn != PawnToPossess)
	{
		if (OldPawn != nullptr)
		{
			OldPawn->Destroy();
		}
		PlayerController->Possess(PawnToPossess);
	}

	// 새로 스폰한 게 아니고, 목적지가 있고, Possess 대상이 "태그 붙은 Pawn" 자체가 아니면 목적지로 이동
	const bool bIsTaggedPawn = PawnToPossess == ExistingPawn && bExistingHasTag;
	if (SpawnPoint != nullptr && !bSpawned && !bIsTaggedPawn)
	{
		PawnToPossess->SetActorLocationAndRotation(SpawnLocation, SpawnRotation, false, nullptr,
		                                           ETeleportType::TeleportPhysics);
	}
	WURELOG(Display, TEXT("ApplyPawnForRow: Row '%s' pawn=%s (%s) spawnPoint=%s at %s"),
	        *RowName.ToString(), *PawnToPossess->GetName(),
	        bSpawned ? TEXT("spawned") : (PawnToPossess == ExistingPawn ? TEXT("existing in level") : TEXT("current")),
	        SpawnPoint ? *SpawnPoint->GetName() : TEXT("none"), *PawnToPossess->GetActorLocation().ToString());
	if (SpawnPoint != nullptr && !bIsTaggedPawn)
	{
		// 카메라가 Control Rotation을 따르므로 바라보는 방향도 목적지 Yaw로 맞춤
		PlayerController->SetControlRotation(FRotator(0.f, SpawnRotation.Yaw, 0.f));
	}
}

void UAsyncLevelStreamingSubsystem::UpdateProgress()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	// 이전 레벨 언로드를 기다리는 중이거나, 진행할 로드 대상이 없으면 아무것도 하지 않음
	if (bHasQueuedLoad)
	{
		return;
	}
	if (LoadingRowName.IsNone())
	{
		World->GetTimerManager().ClearTimer(ProgressTimerHandle);
		return;
	}

	// 이전 틱에 이미 완료된 레벨(100%) + 이번 틱 진행중인 레벨의 부분 진행률
	float Total = static_cast<float>(FinishedLevels.Num());

	for (int32 i = PendingLevels.Num() - 1; i >= 0; --i)
	{
		ULevelStreamingDynamic* Level = PendingLevels[i];
		if (!IsValid(Level))
		{
			PendingLevels.RemoveAt(i);
			continue;
		}

		if (Level->IsLevelLoaded())
		{
			Total += 1.f;
			FinishedLevels.Add(Level);
			PendingLevels.RemoveAt(i);
			continue;
		}

		const float Pct = GetAsyncLoadPercentage(Level->GetWorldAssetPackageFName());
		Total += (Pct >= 0.f) ? (Pct / 100.f) : 0.f;
	}

	const float LoadPhaseProgress = PendingLevelsOriginalCount > 0
		                                ? FMath::Clamp(Total / PendingLevelsOriginalCount, 0.f, 1.f)
		                                : 1.f;

	// 언로드 대기를 거쳐온 경우, 로드 구간은 [UnloadProgressBand, 1.0] 범위로 압축해서 이어붙임
	const float UnloadOffset = bCameFromUnloadWait ? UnloadProgressBand : 0.f;
	const float Progress = UnloadOffset + LoadPhaseProgress * (1.f - UnloadOffset);

	if (PendingLevels.Num() > 0)
	{
		OnLevelStreamingProgress.Broadcast(Progress, false);
		return;
	}

	// 로드가 끝났어도 Visible 전환(AddToWorld)은 이후 몇 프레임에 걸쳐 진행됨.
	// 레벨 Transform(오프셋)이 적용되고 BeginPlay가 끝난 뒤에야 Pawn 배치가 정확하므로 전부 Visible이 될 때까지 대기
	if (VisibilityWaitStartTime < 0.0)
	{
		VisibilityWaitStartTime = World->GetTimeSeconds();

		// Visible 전환 = 새 레벨 액터들의 BeginPlay. 그 전에 Pawn을 준비해 둬야 BeginPlay에서 플레이어를 찾을 수 있음
		PreparePawnBeforeVisible(LoadingRowName);
	}

	bool bAllVisible = true;
	for (ULevelStreamingDynamic* Level : FinishedLevels)
	{
		if (IsValid(Level))
		{
			Level->SetShouldBeVisible(true);
			bAllVisible &= Level->IsLevelVisible();
		}
	}
	const bool bTimedOut = World->GetTimeSeconds() - VisibilityWaitStartTime > VisibilityWaitTimeoutSeconds;
	if (!bAllVisible && !bTimedOut)
	{
		OnLevelStreamingProgress.Broadcast(Progress, false);
		return;
	}
	if (bTimedOut && !bAllVisible)
	{
		WURELOG(Warning, TEXT("UpdateProgress: Row '%s' levels did not become visible in %.1fs; finalizing anyway"),
		        *LoadingRowName.ToString(), VisibilityWaitTimeoutSeconds);
	}

	const FName CompletedRow = LoadingRowName;
	FinishedLevels.Empty();
	PendingLevelsOriginalCount = 0;
	bCameFromUnloadWait = false;
	VisibilityWaitStartTime = -1.0;
	World->GetTimerManager().ClearTimer(ProgressTimerHandle);

	// Pawn 배치까지 끝낸 뒤에 완료를 알려서, 로딩 위젯이 사라진 순간 이미 목적지에 있도록 함
	FinalizeRow(CompletedRow);
	OnLevelStreamingProgress.Broadcast(1.f, true);
}
