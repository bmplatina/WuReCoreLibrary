// © Platina Production 2017~2024


#include "WuReUserWidget.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/GameInstance.h"
#include "LocalizationSubsystem.h"
#include "Components/TextBlock.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "WuRePlayerController.h"

AWuRePlayerController* UWuReUserWidget::GetPlayerController(bool& bIsValid) const
{
	bIsValid = IsValid(PlayerController);
	return PlayerController;
}

void UWuReUserWidget::NativeConstruct()
{
	Super::NativeConstruct();
	LocalizationSubsystem = Cast<ULocalizationSubsystem>(GetGameInstance()->GetSubsystem<ULocalizationSubsystem>());

	PlayerController = Cast<AWuRePlayerController>(GetOwningPlayer());
	PlayerController->OnInputMethodChangedNative.AddDynamic(this, &UWuReUserWidget::HandleInputMethodChanged);

	OnBeginPlayDone.Broadcast();
}

void UWuReUserWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UWuReUserWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	bool bIsValid = false;
	AWuRePlayerController* PC = GetPlayerController(bIsValid);
	if (!bIsValid || !PC) return;

	if (const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (WidgetInputMappingContext)
			{
				Subsystem->AddMappingContext(WidgetInputMappingContext, /*Priority=*/1);
			}
		}
	}

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PC->InputComponent))
	{
		if (IsValid(IA_Confirm))
		{
			EnhancedInputBindingHandles.Add(
				EIC->BindAction(IA_Confirm, ETriggerEvent::Completed,
				                this, &UWuReUserWidget::HandleConfirmCompleted).GetHandle());
		}
		if (IsValid(IA_Up))
		{
			EnhancedInputBindingHandles.Add(
				EIC->BindAction(IA_Up, ETriggerEvent::Completed,
				                this, &UWuReUserWidget::HandleUpKeyCompleted).GetHandle());
		}
		if (IsValid(IA_Down))
		{
			EnhancedInputBindingHandles.Add(
				EIC->BindAction(IA_Down, ETriggerEvent::Completed,
								this, &UWuReUserWidget::HandleDownKeyCompleted).GetHandle());
		}
		if (IsValid(IA_Left))
		{
			EnhancedInputBindingHandles.Add(
				EIC->BindAction(IA_Left, ETriggerEvent::Completed,
								this, &UWuReUserWidget::HandleLeftKeyCompleted).GetHandle());
		}
		if (IsValid(IA_Right))
		{
			EnhancedInputBindingHandles.Add(
				EIC->BindAction(IA_Right, ETriggerEvent::Completed,
								this, &UWuReUserWidget::HandleRightKeyCompleted).GetHandle());
		}
	}
}

void UWuReUserWidget::NativeOnDeactivated()
{
	bool bIsValid = false;
	if (AWuRePlayerController* PC = GetPlayerController(bIsValid); bIsValid && PC)
	{
		if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PC->InputComponent))
		{
			for (const uint32 Handle : EnhancedInputBindingHandles)
			{
				EIC->RemoveBindingByHandle(Handle);
			}
		}
		EnhancedInputBindingHandles.Reset();

		if (const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (WidgetInputMappingContext)
				{
					Subsystem->RemoveMappingContext(WidgetInputMappingContext);
				}
			}
		}
	}
	Super::NativeOnDeactivated();
}

void UWuReUserWidget::KeyboardActivated_Implementation()
{
}

void UWuReUserWidget::GamepadActivated_Implementation()
{
}

UMaterialInstanceDynamic* UWuReUserWidget::CreateGamepadIcon(const EDualSenseKey GamepadKey,
                                                             const bool AddBackgroundTexture,
                                                             const bool BackgroundTextureOnly, const FColor Color)
{
	if (IsValid(DualSenseBtnMaterial.Get()))
	{
		const TWeakObjectPtr<UMaterialInstanceDynamic> MaterialInstanceDynamic = UMaterialInstanceDynamic::Create(
			DualSenseBtnMaterial.Get(), nullptr);
		if (MaterialInstanceDynamic.IsValid())
		{
			if (SetGamepadIcon(MaterialInstanceDynamic.Get(), GamepadKey))
			{
				MaterialInstanceDynamic.Get()->SetScalarParameterValue(FName("AddBackgroundTex"),
				                                                       AddBackgroundTexture ? 1.f : 0.f);
				MaterialInstanceDynamic.Get()->SetScalarParameterValue(FName("BackgroundTexOnly"),
				                                                       BackgroundTextureOnly ? 1.f : 0.f);
				MaterialInstanceDynamic.Get()->SetVectorParameterValue(FName("Color"),
				                                                       FVector(Color.R / 255.f, Color.G / 255.f,
				                                                               Color.B / 255.f));
				return MaterialInstanceDynamic.Get();
			}
		}
	}
	return nullptr;
}

void UWuReUserWidget::SetGamepadIconToMaterial(UMaterialInstanceDynamic* Material, const EDualSenseKey GamepadKey,
                                               const bool AddBackgroundTexture, const bool BackgroundTextureOnly,
                                               const FColor Color)
{
	if (Material)
	{
		Material->SetScalarParameterValue(FName("AddBackgroundTex"), AddBackgroundTexture ? 1.f : 0.f);
		Material->SetScalarParameterValue(FName("BackgroundTexOnly"), BackgroundTextureOnly ? 1.f : 0.f);
		Material->SetVectorParameterValue(FName("Color"), FVector(Color.R / 255.f, Color.G / 255.f, Color.B / 255.f));
		SetGamepadIcon(Material, GamepadKey);
	}
}

void UWuReUserWidget::SetMousePositionByGamepad(const float ValX, const float ValY, float Sensitivity)
{
	if (ValX != 0.f || ValY != 0.f)
	{
		Sensitivity *= GetWorld()->GetDeltaSeconds(); // UGameplayStatics::GetWorldDeltaSeconds(GetWorld());

		float PosX, PosY;
		FVector2D ScreenSize;
		GEngine->GameViewport->GetViewportSize(ScreenSize);
		UGameplayStatics::GetPlayerController(this, 0)->GetMousePosition(PosX, PosY);
		PosX = FMath::Clamp(PosX + ValX * Sensitivity, 0, ScreenSize.X);
		PosY = FMath::Clamp(PosY + ValY * -1.f * Sensitivity, 0, ScreenSize.Y);
		UGameplayStatics::GetPlayerController(this, 0)->
			SetMouseLocation(static_cast<int>(PosX), static_cast<int>(PosY));
	}
}

ULocalizationSubsystem* UWuReUserWidget::GetLocalizationSubsystem(bool& bIsValid)
{
	if (IsValid(LocalizationSubsystem))
	{
		bIsValid = true;
		return LocalizationSubsystem;
	}

	LocalizationSubsystem = Cast<ULocalizationSubsystem>(GetGameInstance()->GetSubsystem<ULocalizationSubsystem>());
	bIsValid = IsValid(LocalizationSubsystem);

	return LocalizationSubsystem;
}

bool UWuReUserWidget::SetGamepadIcon(UMaterialInstanceDynamic* MatParam, const EDualSenseKey GamepadEnum)
{
	if (MatParam)
	{
		FVector2D UVMapOffset;
		switch (GamepadEnum)
		{
		case EDualSenseKey::FaceKeyUp:
			UVMapOffset.X = 0.f;
			UVMapOffset.Y = 0.f;
			break;
		case EDualSenseKey::FaceKeyRight:
			UVMapOffset.X = 1.f / 7.f;
			UVMapOffset.Y = 0.f;
			break;
		case EDualSenseKey::FaceKeyDown:
			UVMapOffset.X = 2.f / 7.f;
			UVMapOffset.Y = 0.f;
			break;
		case EDualSenseKey::FaceKeyLeft:
			UVMapOffset.X = 3.f / 7.f;
			UVMapOffset.Y = 0.f;
			break;
		case EDualSenseKey::LogoButton:
			UVMapOffset.X = 4.f / 7.f;
			UVMapOffset.Y = 0.f;
			break;
		case EDualSenseKey::StartButton:
			UVMapOffset.X = 5.f / 7.f;
			UVMapOffset.Y = 0.f;
			break;
		case EDualSenseKey::SelectButton:
			UVMapOffset.X = 6.f / 7.f;
			UVMapOffset.Y = 0.f;
			break;
		case EDualSenseKey::DPadUp:
			UVMapOffset.X = 0.f;
			UVMapOffset.Y = 1.f / 6.f;
			break;
		case EDualSenseKey::DPadDown:
			UVMapOffset.X = 1.f / 7.f;
			UVMapOffset.Y = 1.f / 6.f;
			break;
		case EDualSenseKey::DPadLeft:
			UVMapOffset.X = 2.f / 7.f;
			UVMapOffset.Y = 1.f / 6.f;
			break;
		case EDualSenseKey::DPadRight:
			UVMapOffset.X = 3.f / 7.f;
			UVMapOffset.Y = 1.f / 6.f;
			break;
		case EDualSenseKey::LShoulder:
			UVMapOffset.X = 4.f / 7.f;
			UVMapOffset.Y = 1.f / 6.f;
			break;
		case EDualSenseKey::LTrigger:
			UVMapOffset.X = 5.f / 7.f;
			UVMapOffset.Y = 1.f / 6.f;
			break;
		case EDualSenseKey::RShoulder:
			UVMapOffset.X = 6.f / 7.f;
			UVMapOffset.Y = 1.f / 6.f;
			break;
		case EDualSenseKey::RTrigger:
			UVMapOffset.X = 0.f;
			UVMapOffset.Y = 2.f / 6.f;
			break;
		case EDualSenseKey::LStickUDLR:
			UVMapOffset.X = 1.f / 7.f;
			UVMapOffset.Y = 2.f / 6.f;
			break;
		case EDualSenseKey::LStickButton:
			UVMapOffset.X = 2.f / 7.f;
			UVMapOffset.Y = 2.f / 6.f;
			break;
		case EDualSenseKey::LStickDown:
			UVMapOffset.X = 3.f / 7.f;
			UVMapOffset.Y = 2.f / 6.f;
			break;
		case EDualSenseKey::LStickLR:
			UVMapOffset.X = 4.f / 7.f;
			UVMapOffset.Y = 2.f / 6.f;
			break;
		case EDualSenseKey::LStickLeft:
			UVMapOffset.X = 5.f / 7.f;
			UVMapOffset.Y = 2.f / 6.f;
			break;
		case EDualSenseKey::LStickRight:
			UVMapOffset.X = 6.f / 7.f;
			UVMapOffset.Y = 2.f / 6.f;
			break;
		case EDualSenseKey::LStickUD:
			UVMapOffset.X = 0.f;
			UVMapOffset.Y = 0.5f;
			break;
		case EDualSenseKey::LStickUp:
			UVMapOffset.X = 1.f / 7.f;
			UVMapOffset.Y = 0.5f;
			break;
		case EDualSenseKey::LStick:
			UVMapOffset.X = 2.f / 7.f;
			UVMapOffset.Y = 0.5f;
			break;
		case EDualSenseKey::RStickUDLR:
			UVMapOffset.X = 3.f / 7.f;
			UVMapOffset.Y = 0.5f;
			break;
		case EDualSenseKey::RStickButton:
			UVMapOffset.X = 4.f / 7.f;
			UVMapOffset.Y = 0.5f;
			break;
		case EDualSenseKey::RStickDown:
			UVMapOffset.X = 5.f / 7.f;
			UVMapOffset.Y = 0.5f;
			break;
		case EDualSenseKey::RStickLR:
			UVMapOffset.X = 6.f / 7.f;
			UVMapOffset.Y = 0.5f;
			break;
		case EDualSenseKey::RStickLeft:
			UVMapOffset.X = 0.f;
			UVMapOffset.Y = 4.f / 6.f;
			break;
		case EDualSenseKey::RStickRight:
			UVMapOffset.X = 1.f / 7.f;
			UVMapOffset.Y = 4.f / 6.f;
			break;
		case EDualSenseKey::RStickUD:
			UVMapOffset.X = 2.f / 7.f;
			UVMapOffset.Y = 4.f / 6.f;
			break;
		case EDualSenseKey::RStickUp:
			UVMapOffset.X = 3.f / 7.f;
			UVMapOffset.Y = 4.f / 6.f;
			break;
		case EDualSenseKey::RStick:
			UVMapOffset.X = 4.f / 7.f;
			UVMapOffset.Y = 4.f / 6.f;
			break;
		case EDualSenseKey::TPadClick:
			UVMapOffset.X = 5.f / 7.f;
			UVMapOffset.Y = 4.f / 6.f;
			break;
		case EDualSenseKey::TPadDown:
			UVMapOffset.X = 6.f / 7.f;
			UVMapOffset.Y = 4.f / 6.f;
			break;
		case EDualSenseKey::TPadLeft:
			UVMapOffset.X = 0.f;
			UVMapOffset.Y = 5.f / 6.f;
			break;
		case EDualSenseKey::TPadRight:
			UVMapOffset.X = 1.f / 7.f;
			UVMapOffset.Y = 5.f / 6.f;
			break;
		case EDualSenseKey::TPadUp:
			UVMapOffset.X = 2.f / 7.f;
			UVMapOffset.Y = 5.f / 6.f;
			break;
		case EDualSenseKey::DPad:
			UVMapOffset.X = 3.f / 7.f;
			UVMapOffset.Y = 5.f / 6.f;
			break;
		default:
			UVMapOffset = FVector2D();
		}
		MatParam->SetScalarParameterValue(TEXT("uTile"), UVMapOffset.X);
		MatParam->SetScalarParameterValue(TEXT("vTile"), UVMapOffset.Y);
		return true;
	}
	return false;
}

void UWuReUserWidget::HandleInputMethodChanged(ECommonInputType InInputType)
{
	if (InInputType == ECommonInputType::Gamepad)
	{
		GamepadActivated_Implementation();
	}
	else if (InInputType == ECommonInputType::MouseAndKeyboard)
	{
		KeyboardActivated_Implementation();
	}
}
