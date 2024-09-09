// © Platina Production 2017~2024

#include "WuRePlayerController.h"
#include "CommonInputSubsystem.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "WuReGameInstance.h"
#include "WuReUserWidget.h"

void AWuRePlayerController::BeginPlay()
{
	Super::BeginPlay();
	SetTickableWhenPaused(true);
	if (UCommonInputSubsystem* CommonInputSubsystemLocal = UCommonInputSubsystem::Get(GetLocalPlayer()))
	{
		CommonInputSubsystemLocal->OnInputMethodChangedNative.AddUObject(this, &AWuRePlayerController::SendInputMethodToWidgets);
	}
	
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(InputMappingContext.Get(), 0);
	}
}

void AWuRePlayerController::PostInitializeComponents() {
	Super::PostInitializeComponents();
}

void AWuRePlayerController::OnPossess(APawn* InPawn) {
	Super::OnPossess(InPawn);
}

UWuReUserWidget* AWuRePlayerController::GetValidWidget(const bool bAdditionalConditions, const bool bIgnoreIsInViewport)
{
	for(int i = Widgets.Num()-1; i >= 0; i--)
	{
		if(Widgets.IsValidIndex(i))
		{
			if(Widgets[i]->GetVisibility() == ESlateVisibility::SelfHitTestInvisible && (Widgets[i]->IsInViewport() || bIgnoreIsInViewport) && bAdditionalConditions)
			{
				return Widgets[i];
			}
		}
	}
	return nullptr;
}

bool AWuRePlayerController::IsLocationInViewport(const FVector& Location)
{
	FVector2D ScreenPosition;

	if (ProjectWorldLocationToScreen(Location, ScreenPosition))
	{
		int32 ViewportSizeX, ViewportSizeY;
		GetViewportSize(ViewportSizeX, ViewportSizeY);

		return ScreenPosition.X > 0 && ScreenPosition.X < ViewportSizeX && ScreenPosition.Y > 0 && ScreenPosition.Y < ViewportSizeY;
	}

	return false;
}

void AWuRePlayerController::SendInputMethodToWidgets(const ECommonInputType InputEnum)
{
	CurrentInputType = InputEnum;
	switch(InputEnum)
	{
	case ECommonInputType::MouseAndKeyboard:
		for(UWuReUserWidget* Element: Widgets)
		{
			if(IsValid(Element))
			{
				Element->KeyboardActivated();
			}
		}
		break;
	case ECommonInputType::Gamepad:
		for(UWuReUserWidget* Element: Widgets)
		{
			if(IsValid(Element))
			{
				Element->GamepadActivated();
			}
		}
		break;
	default:
		break;
	}
}

void AWuRePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (UEnhancedInputComponent* EnhancedInputCompLocal = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputCompLocal->BindAction(InputActionEscapeKey.Get(), ETriggerEvent::Completed, this, &AWuRePlayerController::ToggleEscMenu);
		
		// Button Focusing by D-Pad or Left Analogue Stick
		EnhancedInputCompLocal->BindAction(InputActionLeftStick.Get(), ETriggerEvent::Started, this, &AWuRePlayerController::DirectionKey);
		EnhancedInputCompLocal->BindAction(InputActionDPadLeft.Get(), ETriggerEvent::Completed, this, &AWuRePlayerController::DPadLeft);
		EnhancedInputCompLocal->BindAction(InputActionDPadUp.Get(), ETriggerEvent::Completed, this, &AWuRePlayerController::DPadUp);
		EnhancedInputCompLocal->BindAction(InputActionDPadRight.Get(), ETriggerEvent::Completed, this, &AWuRePlayerController::DPadRight);
		EnhancedInputCompLocal->BindAction(InputActionDPadDown.Get(), ETriggerEvent::Completed, this, &AWuRePlayerController::DPadDown);
		
		// Executing Button by clicking A key
		EnhancedInputCompLocal->BindAction(InputActionGamepadCheck.Get(), ETriggerEvent::Started, this, &AWuRePlayerController::CheckBtnStarted);
		EnhancedInputCompLocal->BindAction(InputActionGamepadCheck.Get(), ETriggerEvent::Completed, this, &AWuRePlayerController::CheckBtnCompleted);
	
		// Executing Button by clicking Back key
		EnhancedInputCompLocal->BindAction(InputActionGamepadBack.Get(), ETriggerEvent::Completed, this, &AWuRePlayerController::BackBtnCompleted);
	}
}

void AWuRePlayerController::ToggleEscMenu()
{
	
}

// Gamepad LStick
void AWuRePlayerController::DirectionKey(const FInputActionValue& Value)
{
	GetValidWidget(true, true)->LeftStick(
		Value.Get<FVector2D>().X,
		CurrentInputType == ECommonInputType::Gamepad
			? Value.Get<FVector2D>().Y * -1.f
			: Value.Get<FVector2d>().Y,
		EJoystickHandler::UD);
}

// Gamepad D-Pad
void AWuRePlayerController::DPadLeft(const FInputActionValue& Value)
{
	GetValidWidget(true, true)->LeftStick(-1.f, 0.f, EJoystickHandler::UD);
}

// Gamepad D-Pad
void AWuRePlayerController::DPadUp(const FInputActionValue& Value)
{
	GetValidWidget(true, true)->LeftStick(0.f, 1.f, EJoystickHandler::UD);
}

void AWuRePlayerController::DPadRight(const FInputActionValue& Value)
{
	GetValidWidget(true, true)->LeftStick(1.f, 0.f, EJoystickHandler::UD);
}

void AWuRePlayerController::DPadDown(const FInputActionValue& Value)
{
	GetValidWidget(true, true)->LeftStick(0.f, -1.f, EJoystickHandler::UD);
}

// Gamepad Face Button Bottom
void AWuRePlayerController::CheckBtnStarted(const FInputActionValue& Value)			// On Hovered - EnhancedInput Started
{
	GetValidWidget(true, true)->CheckButtonOnStarted();
}

void AWuRePlayerController::CheckBtnCompleted(const FInputActionValue& Value)		// On Clicked - EnhancedInput Completed
{
	GetValidWidget(true, true)->CheckButtonOnCompleted();
}

void AWuRePlayerController::BackBtnCompleted(const FInputActionValue& Value)
{
	GetValidWidget(true, true)->BackButtonOnCompleted();
}
