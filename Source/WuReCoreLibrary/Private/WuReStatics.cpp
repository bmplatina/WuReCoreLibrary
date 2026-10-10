// © Platina Production 2017~2024

#include "WuReStatics.h"
#include "WuReLevelLoadingWidget.h"
#include "WuRePlayerController.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "AsyncLevelStreamingSubsystem.h"
#include "LocalizationSubsystem.h"
#include "WuReRootWidget.h"
#include "WuReSubtitleWorldSubsystem.h"
#include "Input/CommonUIActionRouterBase.h"
#include "Kismet/GameplayStatics.h"
#include "RHI.h"
#include "RenderUtils.h"

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
			WURELOG(Error,
			        TEXT(
				        "LoadLevelByRowName: UAsyncLevelStreamingSubsystem instance not found on GameInstance (no concrete Blueprint child class assigned?)"
			        ));
			return false;
		}

		Sub->OnLevelStreamingProgress.
		     AddUniqueDynamic(LoadingWidget, &UWuReLevelLoadingWidget::SetDisplayedLoadingProgress);

		FText DisplayedLevelText = FText::GetEmpty();
		const bool bSucceed = Sub->RequestLoadLevelByRowName(LevelRowName, WorldLocationOffset, WorldRotationOffset,
		                                                     DisplayedLevelText);
		if (bSucceed)
		{
			LoadingWidget->OnDisplayedTargetLevelTextSet(DisplayedLevelText);
		}
		// 위젯 제거는 로딩이 실제로 끝났을 때(OnLevelStreamingProgress의 bIsComplete=true)
		// UWuReLevelLoadingWidget::OnLevelStreamingFinished에서 처리합니다.
		return bSucceed;
	}
	return false;
}

FName UWuReStatics::GetCurrentLevelRowName(const UObject* WorldContextObject)
{
	if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject))
	{
		UAsyncLevelStreamingSubsystem* Sub = GameInstance->GetSubsystem<UAsyncLevelStreamingSubsystem>();
		if (Sub)
		{
			return Sub->GetCurrentLevelRowName();
		}
	}
	return NAME_None;
}

UWuReUserWidget* UWuReStatics::CreateAndAddWuReWidgetToViewport(const UObject* WorldContextObject,
                                                                TSubclassOf<UWuReUserWidget> InWidgetClass,
                                                                EInputMode InputMode)
{
	if (const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject))
	{
		const ULocalPlayer* LocalPlayer = GameInstance->GetLocalPlayerByIndex(0);
		AWuRePlayerController* PlayerController = Cast<AWuRePlayerController>(LocalPlayer->PlayerController);

		UWuReRootWidget* RootWidget = PlayerController->GetRootWidget();
		UWuReUserWidget* NewWidget = RootWidget->CreateAndAddWidgetToViewport(InWidgetClass, InputMode);

		const int32 StackSize = RootWidget->GetWidgetStack() ? RootWidget->GetWidgetStack()->GetWidgetList().Num() : -1;
		WURELOG(Display,
		        TEXT("CreateAndAddWuReWidgetToViewport: RootWidget=%s created '%s' (stack size after add: %d)"),
		        *RootWidget->GetName(), NewWidget ? *NewWidget->GetName() : TEXT("null"), StackSize);

		return NewWidget;
	}
	return nullptr;
}

bool UWuReStatics::IsRootWidgetHasWuReWidget(const UObject* WorldContextObject, UWuReUserWidget* InWidget)
{
	if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject))
	{
		ULocalPlayer* LocalPlayer = GameInstance->GetLocalPlayerByIndex(0);
		AWuRePlayerController* PlayerController = Cast<AWuRePlayerController>(LocalPlayer->PlayerController);

		UWuReRootWidget* RootWidget = PlayerController->GetRootWidget();
		return RootWidget->GetRootHasWidget(InWidget);
	}
	return false;
}

bool UWuReStatics::IsWuReWidgetActivatedInViewport(const UObject* WorldContextObject, UWuReUserWidget* InWidget)
{
	if (const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject))
	{
		const ULocalPlayer* LocalPlayer = GameInstance->GetLocalPlayerByIndex(0);
		AWuRePlayerController* PlayerController = Cast<AWuRePlayerController>(LocalPlayer->PlayerController);

		UWuReRootWidget* RootWidget = PlayerController->GetRootWidget();

		if (RootWidget->GetRootHasWidget(InWidget))
		{
			return RootWidget->GetWidgetStack()->GetActiveWidget() == InWidget;
		}
	}
	return false;
}

UWuReUserWidget* UWuReStatics::GetActivatedWuReWidget(const UObject* WorldContextObject)
{
	if (const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject))
	{
		const ULocalPlayer* LocalPlayer = GameInstance->GetLocalPlayerByIndex(0);
		AWuRePlayerController* PlayerController = Cast<AWuRePlayerController>(LocalPlayer->PlayerController);

		if (UWuReRootWidget* RootWidget = PlayerController->GetRootWidget())
		{
			return Cast<UWuReUserWidget>(RootWidget->GetWidgetStack()->GetActiveWidget());
		}
	}
	return nullptr;
}

UWuReUserWidget* UWuReStatics::GetWuReWidgetOfClass(const UObject* WorldContextObject,
                                                    const TSubclassOf<UWuReUserWidget> InWidgetClass)
{
	if (!IsValid(InWidgetClass)) return nullptr;

	if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject))
	{
		ULocalPlayer* LocalPlayer = GameInstance->GetLocalPlayerByIndex(0);
		AWuRePlayerController* PlayerController = Cast<AWuRePlayerController>(LocalPlayer->PlayerController);

		if (UWuReRootWidget* RootWidget = PlayerController->GetRootWidget())
		{
			for (UCommonActivatableWidget* WidgetList : RootWidget->GetWidgetStack()->GetWidgetList())
			{
				if (WidgetList->GetClass()->IsChildOf(InWidgetClass))
				{
					return Cast<UWuReUserWidget>(WidgetList);
				}
			}
		}
	}
	return nullptr;
}

void UWuReStatics::RemoveWuReWidgetFromParent(const UObject* WorldContextObject, UWuReUserWidget* InWidget)
                                              // EInputMode PostInputMode)
{
	if (!IsValid(InWidget))
	{
		WURELOG(Warning, TEXT("RemoveWuReWidgetFromParent: InWidget is invalid"));
		return;
	}

	if (const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject))
	{
		if (const ULocalPlayer* LocalPlayer = GameInstance->GetLocalPlayerByIndex(0))
		{
			if (AWuRePlayerController* PlayerController = Cast<AWuRePlayerController>(LocalPlayer->PlayerController))
			{
				UWuReRootWidget* RootWidget = PlayerController->GetRootWidget();
				const bool bWasInStack = RootWidget->GetRootHasWidget(InWidget);
				if (!bWasInStack)
				{
					WURELOG(Warning,
					        TEXT(
						        "RemoveWuReWidgetFromParent: '%s' is not in the stack of RootWidget=%s (PlayerController=%s)"
					        ),
					        *InWidget->GetName(), *RootWidget->GetName(), *PlayerController->GetName());
				}
				if (bWasInStack)
				{
					TArray<FString> Names;
					if (UCommonActivatableWidgetStack* Stack = RootWidget->GetWidgetStack())
					{
						for (UCommonActivatableWidget* Widget : Stack->GetWidgetList())
						{
							Names.Add(IsValid(Widget) ? Widget->GetName() : TEXT("<invalid>"));
						}
						WURELOG(Warning, TEXT("RemoveWuReWidgetFromParent: current stack contents: [%s]"),
						        *FString::Join(Names, TEXT(", ")));
					}
					
					RootWidget->RemoveWidgetFromViewport(InWidget);
					// RootWidget->RemoveWidgetFromViewport(InWidget, PostInputMode);
				}
			}
		}
	}
}

UWuReUserWidget* UWuReStatics::ToggleWuReWidget(const UObject* WorldContextObject, TSubclassOf<UWuReUserWidget> InWidgetClass,
                                    bool& bIsWidgetOpened, EInputMode InputMode)
{
	bIsWidgetOpened = false;
	if (!IsValid(InWidgetClass)) return nullptr;

	const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	const ULocalPlayer* LocalPlayer = GameInstance ? GameInstance->GetLocalPlayerByIndex(0) : nullptr;
	AWuRePlayerController* PlayerController = LocalPlayer ? Cast<AWuRePlayerController>(LocalPlayer->PlayerController) : nullptr;
	UWuReRootWidget* RootWidget = PlayerController ? PlayerController->GetRootWidget() : nullptr;
	const UCommonActivatableWidgetStack* Stack = RootWidget ? RootWidget->GetWidgetStack() : nullptr;
	if (!Stack)
	{
		WURELOG(Warning, TEXT("ToggleWuReWidget: RootWidget or its widget stack is not available"));
		return nullptr;
	}

	// 표시 중인 위젯은 RemoveWidget 시 비활성화만 되고 전환이 끝난 뒤에 WidgetList에서 빠지므로,
	// 표시 중인데 비활성인 위젯(=닫히는 중)은 열린 것으로 보지 않음.
	// 스택 아래쪽 위젯도 비활성 상태지만 아직 열려 있는 위젯이므로 포함
	const UCommonActivatableWidget* DisplayedWidget = Stack->GetActiveWidget();
	UWuReUserWidget* ExistingWidget = nullptr;
	int32 NumOpenedWidgets = 0;
	for (UCommonActivatableWidget* Widget : Stack->GetWidgetList())
	{
		if (!IsValid(Widget) || (Widget == DisplayedWidget && !Widget->IsActivated())) continue;

		++NumOpenedWidgets;
		if (!ExistingWidget && Widget->GetClass()->IsChildOf(InWidgetClass))
		{
			ExistingWidget = Cast<UWuReUserWidget>(Widget);
		}
	}

	if (ExistingWidget)
	{
		RootWidget->RemoveWidgetFromViewport(ExistingWidget);

		// 남은 위젯이 있으면 그 위젯의 입력 모드를 CommonUI가 다시 적용하지만,
		// 마지막 위젯이 닫히면 CommonUI는 입력 모드를 되돌리지 않으므로 직접 게임 입력으로 복구
		if (NumOpenedWidgets <= 1)
		{
			if (UCommonUIActionRouterBase* ActionRouter = UCommonUIActionRouterBase::Get(*RootWidget))
			{
				ActionRouter->SetActiveUIInputConfig(
					FUIInputConfig(ECommonInputMode::Game, EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown,
					               true),
					RootWidget);
			}
		}
		return nullptr;
	}

	// 커서 표시/잠금은 CommonUI가 입력 모드로 결정하므로 bShowMouseCursor 대신 모드로 전달
	UWuReUserWidget* NewWidget = RootWidget->CreateAndAddWidgetToViewport(
		InWidgetClass, InputMode);
	bIsWidgetOpened = IsValid(NewWidget);
	return NewWidget;
}

void UWuReStatics::RemoveAllWuReWidgets(const UObject* WorldContextObject)
{
	if (const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject))
	{
		if (const ULocalPlayer* LocalPlayer = GameInstance->GetLocalPlayerByIndex(0))
		{
			if (AWuRePlayerController* PlayerController = Cast<AWuRePlayerController>(LocalPlayer->PlayerController))
			{
				UWuReRootWidget* RootWidget = PlayerController->GetRootWidget();
				for (UCommonActivatableWidget* Widget : RootWidget->GetWidgetStack()->GetWidgetList())
				{
					RemoveWuReWidgetFromParent(WorldContextObject, Cast<UWuReUserWidget>(Widget));
				}
				RootWidget->GetWidgetStack()->ClearWidgets();
			}
		}
	}
}

FText UWuReStatics::GetLocalizedUIText(const UObject* WorldContextObject, FName RowName, const ELocalizationType Type)
{
	if (const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject))
	{
		if (const ULocalizationSubsystem* Sub = GameInstance->GetSubsystem<ULocalizationSubsystem>())
		{
			return Sub->GetLocalizedText(RowName.ToString(), Type);
		}
	}
	return FText::GetEmpty();
}

FText UWuReStatics::FormatTextOneParam(const UObject* WorldContextObject, FText Text, FString Param, FText ToReplaceParam)
{
	FFormatNamedArguments Args;
	if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject))
	{
		if (const ULocalizationSubsystem* Sub = GameInstance->GetSubsystem<ULocalizationSubsystem>())
		{
			const FText PureText = Sub->GetLocalizedText(Text.ToString(), ELocalizationType::UserInterface);
			const FText PureReplacedParam = Sub->GetLocalizedText(ToReplaceParam.ToString(), ELocalizationType::UserInterface);
			Args.Add(Param, PureReplacedParam);
			return FText::Format(PureText, Args);
		}
	}
	Args.Add(Param, ToReplaceParam);
	return FText::Format(Text, Args);
}

bool UWuReStatics::SendSubtitle(const UObject* WorldContextObject, FName InRowNameForDialog, AActor* ActorToAttachAudio)
{
	if (const UWorld* World = WorldContextObject->GetWorld())
	{
		if (UWuReSubtitleWorldSubsystem* Sub = World->GetSubsystem<UWuReSubtitleWorldSubsystem>())
		{
			Sub->RegisterSubtitle(InRowNameForDialog, ActorToAttachAudio);
			return true;
		}
	}
	return false;
}

bool UWuReStatics::IsHardwareRayTracingSupported()
{
	return GRHISupportsRayTracing && GRHISupportsRayTracingShaders;
}

bool UWuReStatics::IsRayTracingActive()
{
	return IsRayTracingEnabled(); // 런타임 전용, 쿡 중 호출 금지
}

void UWuReStatics::SetRayTracingActive(const bool bActive)
{
	if (!IsHardwareRayTracingSupported()) return;
	
	if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.RayTracing.Enable")))
	{
		CVar->Set(bActive ? 1 : 0, ECVF_SetByGameSetting);
	}
}
