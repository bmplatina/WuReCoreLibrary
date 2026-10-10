// © Platina Production 2017~2024

#pragma once

#include "WuReCoreLibrary.h"
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TimerManager.h"
#include "AsyncLevelStreamingSubsystem.generated.h"

class UDataTable;
class ULevel;
class ULevelStreamingDynamic;
class AActor;
class APawn;
struct FLevelListStruct;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLevelStreamingProgress, float, Progress, bool, bIsComplete);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerPawnReady, APawn*, Pawn);

UCLASS(Abstract, Blueprintable, DisplayName="WU:RE Level Streaming Subsystem")
class WURECORELIBRARY_API UAsyncLevelStreamingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;
	
	UPROPERTY(BlueprintAssignable, Category="Level Streaming")
	FOnLevelStreamingProgress OnLevelStreamingProgress;

	// 도어 액터가 RowName만 들고 있다가 호출 (테이블 조회 -> 비동기 로드)
	// 현재 스트리밍된 레벨을 전부 언로드한 뒤(겹치는 레벨 포함), row의 LevelAsset을 전부 새로 로드
	UFUNCTION(BlueprintCallable, Category="Level Streaming")
	bool RequestLoadLevelByRowName(const FName RowName, const FVector Location, const FRotator Rotation,
	                               FText& DisplayedLevelText);

	// row에 속한 레벨들을 전부 언로드
	UFUNCTION(BlueprintCallable, Category="Level Streaming")
	void UnloadLevelByRowName(const FName RowName);
	
	UFUNCTION(BlueprintPure, Category="Level Streaming")
	FName GetCurrentLevelRowName() const { return ActiveLevelRowName; }

	UFUNCTION(BlueprintCallable, Category="Level Streaming")
	void UnloadCurrentLevel();

	UFUNCTION(BlueprintPure, Category="Level Streaming")
	bool IsStreamingInProgress() const { return PendingLevels.Num() > 0 || bHasQueuedLoad; }
	
	UFUNCTION(BlueprintCallable, Category="Level Streaming")
	void TryApplyPawnForCurrentLevel();

	// 레벨 진입 처리(Pawn 스폰/Possess/배치)가 끝났을 때. Pawn 교체가 없는 row여도 현재 조종 중인 Pawn으로 호출됨
	UPROPERTY(BlueprintAssignable, Category="Level Streaming")
	FOnPlayerPawnReady OnPlayerPawnReady;

	// 진행 중인 레벨 로드가 없고, 마지막 레벨 진입 처리 후의 Pawn이 유효한지
	UFUNCTION(BlueprintPure, Category="Level Streaming")
	bool IsPlayerPawnReady() const;

	// 현재 기준 row: 스트리밍으로 진입한 row, 없으면 퍼시스턴트 레벨 이름과 같은 row (타깃 레벨에 바로 PIE한 경우)
	FName GetEffectiveRowName() const;

	// GameMode의 기본 스폰 흐름(RestartPlayer)에서 사용
	TSubclassOf<APawn> GetPawnClassForRow(const FName RowName) const;
	AActor* FindSpawnPointForRow(const FName RowName) const;

protected:
	// BP 자식 클래스(예: BP_LevelStreamingSubsystem)의 Class Defaults에서 지정
	UPROPERTY(EditDefaultsOnly, Category="Level Streaming")
	TObjectPtr<UDataTable> LevelTable;

private:
	void UpdateProgress();

	// 지정한 레벨 패키지를 언로드 요청하고 추적 목록에서 제거
	void UnloadPackage(const FName PackageName);

	// NewPackages를 스트리밍 시작. 시작할 게 없으면 즉시 완료 처리
	bool BeginLoad(const FName RowName, const TArray<FName>& NewPackages, const FVector Location,
	               const FRotator Rotation);

	// 로딩 완료 시점 처리: ActiveLevelRowName 갱신 + Pawn 적용
	void FinalizeRow(const FName RowName);

	// Row의 레벨들(안에서만) 기존 Pawn을 찾거나 스폰해서 Possess하고, TargetSpawnTag 액터 위치로 이동
	void ApplyPawnForRow(const FName RowName);

	// 로드는 끝났지만 아직 Visible 전(=새 레벨 액터들의 BeginPlay 전)에 Pawn을 미리 스폰/Possess.
	// 새 레벨 액터들이 BeginPlay에서 GetPlayerCharacter로 올바른 Pawn을 얻도록 하기 위함
	void PreparePawnBeforeVisible(const FName RowName);

	// PreparePawnBeforeVisible에서 멈춰둔 이동을 배치 후 다시 켬
	void RestorePawnAwaitingPlacement();

	// 현재 조종 중인 Pawn으로 OnPlayerPawnReady 브로드캐스트
	void NotifyPlayerPawnReady();

	const FLevelListStruct* FindRow(const FName RowName, const TCHAR* ContextString, bool bWarnIfMissing = true) const;

	// Row에 속한 레벨들 중 현재 로드된 것 (서브레벨 스트리밍 또는 퍼시스턴트 레벨 자체)
	void GatherRowLevels(const FLevelListStruct& Row, TArray<const ULevel*>& OutLevels) const;

	// TargetSpawnTag가 붙은 (Pawn이 아닌) 첫 액터. OutCount에는 해당 액터 수
	static AActor* FindSpawnPointInLevels(const FLevelListStruct& Row, const TArray<const ULevel*>& Levels,
	                                      int32* OutCount = nullptr);

	// LevelsPendingUnload가 전부 사라졌는지 매 틱 확인하고, 끝나면 큐잉된 로드를 시작
	void CheckUnloadThenStartQueuedLoad();

	// 패키지 이름(FName) -> 스트리밍 인스턴스. 로드 완료/진행 중인 레벨 전부
	UPROPERTY()
	TMap<FName, TObjectPtr<ULevelStreamingDynamic>> LoadedLevels;

	UPROPERTY()
	TArray<TObjectPtr<ULevelStreamingDynamic>> PendingLevels;

	UPROPERTY()
	TArray<TObjectPtr<ULevelStreamingDynamic>> FinishedLevels;

	// 언로드 완료를 기다리는 이전 레벨들 (완전히 사라졌는지 폴링용)
	UPROPERTY()
	TArray<TObjectPtr<ULevelStreamingDynamic>> LevelsPendingUnload;

	int32 PendingLevelsOriginalCount = 0;

	FTimerHandle ProgressTimerHandle;
	FTimerHandle UnloadWaitTimerHandle;

	// 현재 목표(가장 최근에 요청된 row)가 소유하는 레벨 패키지들
	TSet<FName> TargetPackages;

	// 로딩이 "완료"된 대표 row. 완료 시점에만 갱신됨
	FName ActiveLevelRowName;

	// 지금 로드/큐잉 중인 row (같은 row 중복 요청 무시용)
	FName LoadingRowName;

	// 이전 레벨 언로드가 끝나면 이어서 시작할 로드 요청
	bool bHasQueuedLoad = false;
	FName QueuedRowName;
	FVector QueuedLocation = FVector::ZeroVector;
	FRotator QueuedRotation = FRotator::ZeroRotator;

	// 언로드 대기 구간의 진행률 연출용 (실제 % 정보가 없어 경과 시간 기반으로 근사)
	double UnloadWaitStartTime = 0.0;

	// 로드 완료 후 Visible 전환 대기를 시작한 시각 (-1이면 대기 중 아님)
	double VisibilityWaitStartTime = -1.0;

	// 이번 로드가 "언로드 대기"를 거쳐서 왔는지 (거쳤으면 진행률 앞부분을 언로드 구간에 할당)
	bool bCameFromUnloadWait = false;

	// Visible 전에 미리 스폰돼 이동이 꺼진 채 배치를 기다리는 Pawn
	TWeakObjectPtr<APawn> PawnAwaitingPlacement;

	// 마지막 레벨 진입 처리가 끝난 Pawn (새 로드 요청 시 초기화)
	TWeakObjectPtr<APawn> ReadyPawn;
};
