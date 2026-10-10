// © Platina Production 2017~2024


#include "WuReSubtitleWorldSubsystem.h"

#include "LocalizationSubsystem.h"
#include "WuReLocalizationStruct.h"
#include "Engine/GameInstance.h"

/* 목표
 * SubTitleTextBlock의 초기 Visibility는 ESlateVisibility::Collapsed이다.
 * SubtitleTextBlock은 UTextBlock* 포인터이다.
목표 1. SubTitleTextBlock->GetVisibility == ESlateVisibility::Collapsed일 때 이 함수가 호출되면:
	SubtitleTextBlock의 텍스트를 PendingDialogs[0].DialogCode로 지정한다.
	PendingDialogs[0].Duration초 후, Visibility를 ESlateVisibility::Collapsed로 바꾼다.
	이후 PendingDialog.RemoveAt(0)을 통해 재생한 자막 정보를 제거한다.
목표 2. SubTitleTextBlock->GetVisibility == ESlateVisibility::Visible일 때 이 함수가 호출되면: (즉 이 함수가 다시 호출되는 시점이 자막이 이미 재생 중일 때)
	SubtitleEntryPoint의 파라미터를 PendingDialogs.Add로 저장한다.
	이미 재생 중인 자막이 끝나기를 기다렸다가, 자막이 끝나면 목표 1의 기능을 수행한다.
*/

void UWuReSubtitleWorldSubsystem::RegisterSubtitle(FName InRowNameForDialog, AActor* ActorToAttachAudio)
{
	if (IsValid(GetWorld()))
	{
		// Dialog 정보를 추가
		PendingDialogs.Add(FSubtitleInfo(InRowNameForDialog, ActorToAttachAudio));

		// 이미 자막이 재생 중이지 않다면 자막을 보여줌
		if (!bIsSubtitlePlaying)
		{
			ShowSubtitle();
		}
	}
}

void UWuReSubtitleWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (UGameInstance* GameInstanceLocal = GetWorld()->GetGameInstance())
	{
		LocalizationSubsystem = GameInstanceLocal->GetSubsystem<ULocalizationSubsystem>();
	}
}

bool UWuReSubtitleWorldSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
	// return Super::DoesSupportWorldType(WorldType);
}

bool UWuReSubtitleWorldSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer)) return false;   // 여기서 DoesSupportWorldType도 확인됨
	
	const UWorld* World = Cast<UWorld>(Outer);
	const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;

	return IsValid(GameInstance) && IsValid(GameInstance->GetSubsystem<ULocalizationSubsystem>());
}

void UWuReSubtitleWorldSubsystem::ShowSubtitle()
{
	if (HasPendingSubtitle()) // 실행 대기 중인 자막이 있을 때
	{
		if (!bIsSubtitlePlaying)
		{
			// 자막을 표시하고 텍스트 설정
			// SubtitleTextBlock->SetVisibility(ESlateVisibility::Visible);
			// SubtitleTextBlock->SetText(FText::FromName(PendingDialogs[0].RowName));
			bIsSubtitlePlaying = true;
			OnSubtitleStateChanged.Broadcast(PendingDialogs[0].RowName);
			// SubtitleTextBlock.Get()->SetText(UPDLocalization::GetLocalizedDialogue(PendingDialogs[0].DialogCode, GetParadiseGameInstance()->GetIsKorean()));


			FWuReLocalizationStruct TargetStruct = LocalizationSubsystem->GetLocalizedContextStruct(
				PendingDialogs[0].RowName, ELocalizationType::Dialogue);

			// Voiceline은 Soft 참조라 IsValid()/Get()은 이미 로드된 경우에만 유효하므로 직접 로드한다.
			USoundBase* Voiceline = TargetStruct.Voiceline.LoadSynchronous();

			if (IsValid(Voiceline))
			{
				if (const AActor* SoundActor = PendingDialogs[0].ActorToAttach)
				{
					UGameplayStatics::SpawnSoundAttached(Voiceline, SoundActor->GetRootComponent());
				}
				else
				{
					UGameplayStatics::PlaySound2D(this, Voiceline);
				}
				// PendingDialogs[0].AudioComponent->Play();
			}
			// 자막이 끝나면 호출할 함수 설정

			float SubtitleDuration = IsValid(Voiceline) ? Voiceline->GetDuration() : TargetStruct.ManualDuration;

			if (SubtitleDuration > 0.f)
			{
				GetWorld()->GetTimerManager().SetTimer(
					SubtitleTimerHandle,
					this,
					&UWuReSubtitleWorldSubsystem::HideSubtitle,
					SubtitleDuration,
					false);
			}
			else
			{
				HideSubtitle();
			}
		}
		// 이미 자막이 재생 중일 때, 대기열에 추가된 자막을 그대로 유지
	}
}

void UWuReSubtitleWorldSubsystem::HideSubtitle()
{
	if (HasPendingSubtitle())
	{
		// 첫 번째 자막 제거
		PendingDialogs.RemoveAt(0);
		
		// 자막 숨기기
		bIsSubtitlePlaying = false;
		
		OnSubtitleStateChanged.Broadcast(NAME_None);
		// SubtitleTextBlock->SetVisibility(ESlateVisibility::Collapsed);

		

		// 다음 자막 표시
		if (HasPendingSubtitle())
		{
			if (SubtitleWaitingTime > 0.f)
			{
				GetWorld()->GetTimerManager().SetTimer(
				SubtitleTimerHandle,
				this,
				&UWuReSubtitleWorldSubsystem::ShowSubtitle,
				SubtitleWaitingTime,
				false);
			}
			else
			{
				ShowSubtitle();
			}
			// ShowSubtitle();
		}
		else
		{
			OnPendingDialogsEmpty.Broadcast();
		}
	}
}
