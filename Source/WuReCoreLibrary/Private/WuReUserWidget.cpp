// © Platina Production 2017~2024


#include "WuReUserWidget.h"
#include "Components/TextBlock.h"
#include "Engine/DataTable.h"
#include "WuReWidgetComponent.h"
#include "WuRePlayerController.h"

void UWuReUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if(AWuRePlayerController* PlayerCtlLocal = GetPlayerController())
	{
		if(!PlayerCtlLocal->Widgets.Contains(this) && bAutoAddWidgetToPlayerController)
		{
			PlayerCtlLocal->Widgets.AddUnique(this);
		}
	}
	
	// Hover 1st button
	if(WidgetComponents.IsValidIndex(0))
	{
		FocusOnHoveredElement(WidgetComponents[0]);
	}
	
	// Get & Update Localized Text
	if(!IsValid(UILocalizationTable.Get()))
	{
		UILocalizationTable = GetWuReGameInstance()->GlobalUILocalizationTable.Get();
	}
	if(!IsValid(DialogueLocalizationTable.Get()))
	{
		DialogueLocalizationTable = GetWuReGameInstance()->GlobalDialogueLocalizationTable.Get();
	}

	if(IsValid(DialogueLocalizationTable.Get()) && IsValid(UILocalizationTable.Get()))
	{
		UpdateLocalizedLanguage();
	}

	OnBeginPlayDone.Broadcast();
}

void UWuReUserWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if(AWuRePlayerController* PlayerCtlLocal = GetPlayerController())
	{
		if(PlayerCtlLocal->Widgets.Contains(this)) {
			PlayerCtlLocal->Widgets.Remove(this);
		}
	}
}

void UWuReUserWidget::FocusOnHoveredElement(UWuReWidgetComponent* HoveredElement)
{
	if(IsValid(HoveredElement))
	{
		if(WidgetComponents.Contains(HoveredElement))
		{
			for(UWuReWidgetComponent* WidgetComp: WidgetComponents)
			{
				WidgetComp->SetElementHovered(WidgetComp == HoveredElement);
			}
			SetElementTargetIdx(WidgetComponents.Find(HoveredElement));
		}
	}
}

bool UWuReUserWidget::UpdateTextBlockLanguageUI(UTextBlock* TextBlock,
	const FString Code, const FText FormatText)
{
	if(IsValid(TextBlock))
	{
		if(FormatText.IsEmpty())
		{
			// TextBlock->SetText(UPDLocalization::GetLocalizedUserInterfaceText(Code, bIsKorean));
			TextBlock->SetText(GetLocalizedUIContext(FName(Code)));
		}
		else
		{
			// TextBlock->SetText(FText::Format(FTextFormat(FormatText), UPDLocalization::GetLocalizedUserInterfaceText(Code, bIsKorean)));
			TextBlock->SetText(FText::Format(FTextFormat(FormatText), GetLocalizedUIContext(FName(Code))));
		}
		return true;
	}
	return false;
}

FWuReLocalizationStruct UWuReUserWidget::GetLocalizedDialogueStruct(const FName RowName)
{
	static const FString ContextString(TEXT("GENERAL"));
	if(IsValid(DialogueLocalizationTable.Get()))
	{
		if(FWuReLocalizationStruct* Row = DialogueLocalizationTable.Get()->FindRow<FWuReLocalizationStruct>(RowName, ContextString))
		{
			return *Row;
		}
	}
	return FWuReLocalizationStruct(FText::FromString(RowName.ToString()), FText::FromString(RowName.ToString()));
}

FWuReLocalizationStruct UWuReUserWidget::GetLocalizedUIContextStruct(const FName RowName)
{
	FString ContextString(TEXT("GENERAL"));
	if(IsValid(UILocalizationTable.Get()))
	{
		if(FWuReLocalizationStruct* Row = UILocalizationTable.Get()->FindRow<FWuReLocalizationStruct>(RowName, ContextString))
		{
			return *Row;
		}
	}
	return FWuReLocalizationStruct(FText::FromName(RowName), FText::FromName(RowName));
}

void UWuReUserWidget::SetWidgetComponentsToArray(TArray<UWuReWidgetComponent*> WidgetComps)
{
	if(WidgetComponents.IsValidIndex(0))
	{
		WidgetComponents = WidgetComps;
	}
}

TArray<UWuReWidgetComponent*>& UWuReUserWidget::GetWidgetComponentArray()
{
	return WidgetComponents;
}

void UWuReUserWidget::ElementHoverCtl(const bool bIsUp)
{
	// Get Index of PDButton to Hover
	if (bIsUp && GetElementTargetIdx() == 0)
	{
		// If IsUp is true and FocusedBtnTarget is 0,
		// reset FocusedBtnTarget to the maximum index.
		SetElementTargetIdx(WidgetComponents.Num() - 1);
	}
	else if (!bIsUp && GetElementTargetIdx() == WidgetComponents.Num() - 1)
	{
		// If IsUp is false and FocusedBtnTarget is already at the maximum index,
		// reset FocusedBtnTarget to 0.
		SetElementTargetIdx(0);
	}
	else
	{
		SetElementTargetIdx(FMath::Clamp(bIsUp ? GetElementTargetIdx() - 1 : GetElementTargetIdx() + 1, 0, WidgetComponents.Num() - 1));
	}
	// Set Ho
	for(UWuReWidgetComponent* Element: WidgetComponents)
	{
		if(IsValid(Element))
		{
			Element->SetElementHovered(Element == WidgetComponents[GetElementTargetIdx()]);
		}
	}
}

void UWuReUserWidget::SetElementTargetIdx(const int Index)
{
	FocusedBtnTarget = Index;
}

int& UWuReUserWidget::GetElementTargetIdx()
{
	return FocusedBtnTarget;
}

void UWuReUserWidget::UpdateLocalizedLanguage_Implementation()
{
	
}

UMaterialInstanceDynamic* UWuReUserWidget::CreateGamepadIcon(const EDualSenseKey GamepadKey, const bool AddBackgroundTexture,
                                                             const bool BackgroundTextureOnly, const FColor Color)
{
	if(IsValid(DualSenseBtnMaterial.Get()))
	{
		const TWeakObjectPtr<UMaterialInstanceDynamic> MaterialInstanceDynamic = UMaterialInstanceDynamic::Create(DualSenseBtnMaterial.Get(), nullptr);
		if(MaterialInstanceDynamic.IsValid())
		{
			if(SetGamepadIcon(MaterialInstanceDynamic.Get(), GamepadKey))
			{
				MaterialInstanceDynamic.Get()->SetScalarParameterValue(FName("AddBackgroundTex"), AddBackgroundTexture ? 1.f : 0.f);
				MaterialInstanceDynamic.Get()->SetScalarParameterValue(FName("BackgroundTexOnly"), BackgroundTextureOnly ? 1.f : 0.f);
				MaterialInstanceDynamic.Get()->SetVectorParameterValue(FName("Color"), FVector(Color.R/255.f, Color.G/255.f, Color.B/255.f));
				return MaterialInstanceDynamic.Get();
			}
		}
	}
	return nullptr;
}

void UWuReUserWidget::SetGamepadIconToMaterial(UMaterialInstanceDynamic* Material, const EDualSenseKey GamepadKey,
	const bool AddBackgroundTexture, const bool BackgroundTextureOnly, const FColor Color)
{
	if(Material)
	{
		Material->SetScalarParameterValue(FName("AddBackgroundTex"), AddBackgroundTexture ? 1.f : 0.f);
		Material->SetScalarParameterValue(FName("BackgroundTexOnly"), BackgroundTextureOnly ? 1.f : 0.f);
		Material->SetVectorParameterValue(FName("Color"), FVector(Color.R/255.f, Color.G/255.f, Color.B/255.f));
		SetGamepadIcon(Material, GamepadKey);
	}
}

void UWuReUserWidget::SetMousePositionByGamepad(const float ValX, const float ValY, float Sensitivity)
{
	if(ValX != 0.f || ValY != 0.f)
	{
		Sensitivity *= GetWorld()->GetDeltaSeconds(); // UGameplayStatics::GetWorldDeltaSeconds(GetWorld());
		
		float PosX, PosY;
		FVector2D ScreenSize;
		GEngine->GameViewport->GetViewportSize(ScreenSize);
		UGameplayStatics::GetPlayerController(this, 0)->GetMousePosition(PosX, PosY);
		PosX = FMath::Clamp(PosX + ValX * Sensitivity, 0, ScreenSize.X);
		PosY = FMath::Clamp(PosY + ValY * -1.f * Sensitivity, 0, ScreenSize.Y);
		UGameplayStatics::GetPlayerController(this, 0)->SetMouseLocation(static_cast<int>(PosX), static_cast<int>(PosY));
	}
}

bool UWuReUserWidget::SetGamepadIcon(UMaterialInstanceDynamic* MatParam, const EDualSenseKey GamepadEnum)
{
	if(MatParam)
	{
		FVector2D UVMapOffset;
		switch(GamepadEnum)
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
		}
		MatParam->SetScalarParameterValue(TEXT("uTile"), UVMapOffset.X);
		MatParam->SetScalarParameterValue(TEXT("vTile"), UVMapOffset.Y);
		return true;
	}
	return false;
}

// BlueprintNativeEvents
void UWuReUserWidget::LeftStick(const float ValueX, const float ValueY, const EJoystickHandler JoystickHandleOptions)
{
	if(IsValid(ClickSound.Get()))
	{
		PlaySound(ClickSound.Get());
	}
	switch(JoystickHandleOptions)
	{
	case EJoystickHandler::UD:
		if(ValueX <= 0.1f && ValueX >= -0.1f)	// Prevent Left-Right
		{
			ElementHoverCtl(ValueY > 0);
		}
		break;
	case EJoystickHandler::LR:
		if(ValueY <= 0.1f && ValueY >= -0.1f)	// Prevent Up-Down
		{
			ElementHoverCtl(ValueX < 0);
		}
		break;
	case EJoystickHandler::UDLR:
		break;
	}
}

void UWuReUserWidget::CheckButtonOnStarted()
{
	for(int i = 0; i <= WidgetComponents.Num(); i++)
	{
		if(WidgetComponents.IsValidIndex(i))
		{
			WidgetComponents[i]->SetElementClicked(i == GetElementTargetIdx());
		}
	}
}

void UWuReUserWidget::KeyboardActivated()
{
}

void UWuReUserWidget::GamepadActivated()
{
}

void UWuReUserWidget::CheckButtonOnCompleted()
{
	if(IsValid(ClickSound.Get()))
	{
		PlaySound(ClickSound.Get());
	}
}

void UWuReUserWidget::BackButtonOnCompleted()
{
	if(IsValid(ClickSound.Get()))
	{
		PlaySound(ClickSound.Get());
	}
}
