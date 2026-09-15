// © Platina Production 2017~2024

#include "WuReStatics.h"
#include "WuReLevelLoadingWidget.h"
#include "WuRePlayerController.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "AsyncLevelStreamingSubsystem.h"
#include "WuReRootWidget.h"
#include "Kismet/GameplayStatics.h"

bool UWuReStatics::LoadLevelByRowName(const UObject* WorldContextObject,
                                      TSubclassOf<UWuReLevelLoadingWidget> LoadingWidgetClass,
                                      const FName LevelRowName,
                                      const FVector WorldLocationOffset,
                                      const FRotator WorldRotationOffset)
{
	if (!IsValid(LoadingWidgetClass))
	{
		return false;
	}

	if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject))
	{
		UAsyncLevelStreamingSubsystem* Sub = GameInstance->GetSubsystem<UAsyncLevelStreamingSubsystem>();
		TObjectPtr<UWuReUserWidget> CreatedWidget = CreateAndAddWuReWidgetToViewport(
			WorldContextObject, LoadingWidgetClass, EInputMode::GameOnly);

		TObjectPtr<UWuReLevelLoadingWidget> LoadingWidget = Cast<UWuReLevelLoadingWidget>(CreatedWidget);

		if (Sub == nullptr)
		{
			return false;
		}

		Sub->OnLevelStreamingProgress.
		     AddUniqueDynamic(LoadingWidget, &UWuReLevelLoadingWidget::SetDisplayedLoadingProgress);

		FText DisplayedLevelText = FText::GetEmpty();
		const bool bSucceed = Sub->RequestLoadLevelByRowName(LevelRowName, WorldLocationOffset, WorldRotationOffset,
		                                                     DisplayedLevelText);
		if (bSucceed)
		{
			LoadingWidget->SetDisplayedTargetLevelText(DisplayedLevelText);
			GameInstance->GetLocalPlayerByIndex(0)->PlayerController->SetInputMode(FInputModeGameOnly());
		}
		return bSucceed;
	}
	return false;
}

UWuReUserWidget* UWuReStatics::CreateAndAddWuReWidgetToViewport(const UObject* WorldContextObject,
                                                                TSubclassOf<UWuReUserWidget> InWidgetClass,
                                                                EInputMode InputMode)
{
	if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject))
	{
		ULocalPlayer* LocalPlayer = GameInstance->GetLocalPlayerByIndex(0);
		AWuRePlayerController* PlayerController = Cast<AWuRePlayerController>(LocalPlayer->PlayerController);

		return PlayerController->GetRootWidget()->CreateAndAddWidgetToViewport(
			InWidgetClass, InputMode);
	}
	return nullptr;
}

void UWuReStatics::RemoveWuReWidgetFromParent(const UObject* WorldContextObject, UWuReUserWidget* InWidget,
	EInputMode PostInputMode)
{
	if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject))
	{
		ULocalPlayer* LocalPlayer = GameInstance->GetLocalPlayerByIndex(0);
		AWuRePlayerController* PlayerController = Cast<AWuRePlayerController>(LocalPlayer->PlayerController);

		PlayerController->GetRootWidget()->RemoveWidgetFromViewport(InWidget, PostInputMode);
	}
}
