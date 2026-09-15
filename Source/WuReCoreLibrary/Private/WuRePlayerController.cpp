// © Platina Production 2017~2024

#include "WuRePlayerController.h"
#include "CommonInputSubsystem.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "WuReRootWidget.h"
#include "WuReUserWidget.h"

void AWuRePlayerController::BeginPlay()
{
	Super::BeginPlay();
	SetTickableWhenPaused(true);
	if (UCommonInputSubsystem* CommonInputSubsystemLocal = UCommonInputSubsystem::Get(GetLocalPlayer()))
	{
		CommonInputSubsystemLocal->OnInputMethodChangedNative.AddUObject(
			this, &AWuRePlayerController::BroadcastOnInputMethodChanged);
	}

	if (IsValid(RootWidgetStaticClass))
	{
		RootWidget = CreateWidget<UWuReRootWidget>(this, RootWidgetStaticClass);
		RootWidget->AddToViewport();
	}

	if (IsValid(EscapeMenuWidgetStaticClass))
	{
		EscapeMenuWidget = CreateWidget<UWuReUserWidget>(this, EscapeMenuWidgetStaticClass);
	}

	OnBeginPlayDone.Broadcast();
}

void AWuRePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (const ULocalPlayer* LocalPlayerLocal = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* SubsystemLocal =
			LocalPlayerLocal->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (InputMappingContext)
			{
				SubsystemLocal->AddMappingContext(InputMappingContext, 0);
			}
		}
	}

	if (UEnhancedInputComponent* EnhancedInputCompLocal = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputCompLocal->BindAction(IA_Escape, ETriggerEvent::Completed, this,
										   &AWuRePlayerController::ToggleEscMenu);
	}
}

void AWuRePlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AWuRePlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}


bool AWuRePlayerController::IsLocationInViewport(const FVector& Location)
{
	FVector2D ScreenPosition;

	if (ProjectWorldLocationToScreen(Location, ScreenPosition))
	{
		int32 ViewportSizeX, ViewportSizeY;
		GetViewportSize(ViewportSizeX, ViewportSizeY);

		return ScreenPosition.X > 0 && ScreenPosition.X < ViewportSizeX && ScreenPosition.Y > 0 && ScreenPosition.Y <
			ViewportSizeY;
	}

	return false;
}

UWuReRootWidget* AWuRePlayerController::GetRootWidget()
{
	return RootWidget;
}

void AWuRePlayerController::GetAllWuReWidgetsInRoot(TArray<UWuReUserWidget*>& OutWidgets)
{
	TArray<UCommonActivatableWidget*> WidgetsLocal = RootWidget->GetWidgetStack()->GetWidgetList();
	for (UCommonActivatableWidget* Widget : WidgetsLocal)
	{
		if (UWuReUserWidget* WuReWidget = Cast<UWuReUserWidget>(Widget))
		{
			OutWidgets.Add(WuReWidget);
		}
	}
}

UWuReUserWidget* AWuRePlayerController::GetActivatedWuReWidget()
{
	return Cast<UWuReUserWidget>(RootWidget->GetWidgetStack()->GetActiveWidget());
}

void AWuRePlayerController::ToggleEscMenu()
{
	if (IsValid(RootWidget))
	{
		RootWidget->ToggleEscMenu();
	}
}

void AWuRePlayerController::BroadcastOnInputMethodChanged(ECommonInputType InInputType)
{
	CurrentInputType = InInputType;
	OnInputMethodChangedNative.Broadcast(InInputType);
}
