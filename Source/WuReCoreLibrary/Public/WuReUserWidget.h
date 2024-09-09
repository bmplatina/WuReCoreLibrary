// © Platina Production 2017~2024

#pragma once

#include "WuReCoreLibrary.h"
#include "Blueprint/UserWidget.h"
#include "WuReGameInstance.h"
#include "WuReLocalizationStruct.h"
#include "WuRePlayerController.h"
#include "WuReUserWidget.generated.h"

/**
 * 
 */

class UWuReWidgetComponent;
class AWuRePlayerController;
class UTextBlock;
class UWuReGameInstance;
class UDataTable;

UENUM(BlueprintType)
enum class ELocalizationType : uint8
{
	Dialogue,
	UserInterface,
};

UENUM(BlueprintType)
enum class EDualSenseKey: uint8
{
	FaceKeyUp UMETA(DisplayName="Triangle"),
	FaceKeyRight UMETA(DisplayName="Circle"),
	FaceKeyDown UMETA(DisplayName="X"),
	FaceKeyLeft UMETA(DisplayName="Square"),
	LogoButton UMETA(DisplayName="PS"),
	StartButton UMETA(DisplayName="Menu"),
	SelectButton UMETA(DisplayName="Share"),
	DPadUp UMETA(DisplayName="D-Pad Up"),
	DPadDown UMETA(DisplayName="D-Pad Down"),
	DPadLeft UMETA(DisplayName="D-Pad Left"),
	DPadRight UMETA(DisplayName="D-Pad Right"),
	LShoulder UMETA(DisplayName="L1"),
	LTrigger UMETA(DisplayName="L2"),
	RShoulder UMETA(DisplayName="R1"),
	RTrigger UMETA(DisplayName="R2"),
	LStickUDLR UMETA(DisplayName="Left Stick 4-Directions"),
	LStickButton UMETA(DisplayName="L3"),
	LStickDown UMETA(DisplayName="Left Stick Down"),
	LStickLR UMETA(DisplayName="Left Stick Left-Right"),
	LStickLeft UMETA(DisplayName="Left Stick Left"),
	LStickRight UMETA(DisplayName="Left Stick Right"),
	LStickUD UMETA(DisplayName="Left Stick Up-Down"),
	LStickUp UMETA(DisplayName="Left Stick Up"),
	LStick UMETA(DisplayName="Left Stick"),
	RStickUDLR UMETA(DisplayName="Right Stick 4-Directions"),
	RStickButton UMETA(DisplayName="R3"),
	RStickDown UMETA(DisplayName="Right Stick Down"),
	RStickLR UMETA(DisplayName="Right Stick Left-Right"),
	RStickLeft UMETA(DisplayName="Right Stick Left"),
	RStickRight UMETA(DisplayName="Right Stick Right"),
	RStickUD UMETA(DisplayName="Right Stick Up-Down"),
	RStickUp UMETA(DisplayName="Right Stick Up"),
	RStick UMETA(DisplayName="Right Stick"),
	TPadClick UMETA(DisplayName="Touchpad Click"),
	TPadDown UMETA(DisplayName="Touchpad Drag to Down"),
	TPadLeft UMETA(DisplayName="Touchpad Drag to Left"),
	TPadRight UMETA(DisplayName="Touchpad Drag to Right"),
	TPadUp UMETA(DisplayName="Touchpad Drag to Up"),
	DPad UMETA(DisplayName="D-Pad"),
};

UENUM(BlueprintType)
enum class EJoystickHandler: uint8
{
	LR		UMETA(DisplayName="Left-Right Only"),
	UD		UMETA(DisplayName="Up-Down Only"),
	UDLR	UMETA(DisplayName="4 Directions"),
};

// public, protected, private field
UCLASS(DisplayName="WU:RE Base Widget")
class WURECORELIBRARY_API UWuReUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="WU:RE Widget|Sound")
	TObjectPtr<USoundBase> ClickSound;

	UPROPERTY(EditAnywhere, Category="WU:RE Widget|Design", DisplayName="DualSense Button Material Base")
	TObjectPtr<UMaterialInterface> DualSenseBtnMaterial;
	
	// Keyboard-Gamepad Controls
	virtual void KeyboardActivated();
	virtual void GamepadActivated();

	virtual void LeftStick(const float ValueX, const float ValueY, const EJoystickHandler JoystickHandleOptions);
	virtual void CheckButtonOnStarted();
	virtual void CheckButtonOnCompleted();
	virtual void BackButtonOnCompleted();
	
	// Button List Control
	UFUNCTION(BlueprintSetter, Category="WU:RE Widget|Widget Components")
	void SetWidgetComponentsToArray(TArray<UWuReWidgetComponent*> WidgetComps);

	UFUNCTION(BlueprintGetter, Category="WU:RE Widget|Widget Components")
	TArray<UWuReWidgetComponent*>& GetWidgetComponentArray();

	// Control buttons inherited from UPDButton
	UFUNCTION(BlueprintCallable, Category="WU:RE Widget|Widget Components", DisplayName="Control Widget Component Hovered State")
	virtual void ElementHoverCtl(const bool bIsUp);

	UFUNCTION(BlueprintCallable, Category="WU:RE Widget|Widget Components", DisplayName="Set Element Target Index")
	void SetElementTargetIdx(const int Index);

	UFUNCTION(BlueprintPure, Category="WU:RE Widget|Widget Components", DisplayName="Get Element Target Index")
	int& GetElementTargetIdx();

	// Update Language
	UFUNCTION(BlueprintNativeEvent, Category="WU:RE Widget|Localization")
	void UpdateLocalizedLanguage();

	UFUNCTION(BlueprintGetter, Category="WU:RE Widget|References")
	inline UWuReGameInstance* GetWuReGameInstance() const
	{
		return Cast<UWuReGameInstance>(GetGameInstance());
	}

	UFUNCTION(BlueprintGetter, Category="WU:RE Widget|References")
	inline AWuRePlayerController* GetPlayerController() const
	{
		return Cast<AWuRePlayerController>(GetWorld()->GetFirstPlayerController());
	}

	// DualSense Icon Materials
	UFUNCTION(BlueprintPure, Category="WU:RE Widget|Design")
	UMaterialInstanceDynamic* CreateGamepadIcon(const EDualSenseKey GamepadKey, const bool AddBackgroundTexture,
		const bool BackgroundTextureOnly, const FColor Color);

	UFUNCTION(BlueprintCallable, Category="WU:RE Widget|Design", DisplayName="Set Gamepad Icon")
	void SetGamepadIconToMaterial(UMaterialInstanceDynamic* Material, const EDualSenseKey GamepadKey,
		const bool AddBackgroundTexture, const bool BackgroundTextureOnly, const FColor Color);

	UFUNCTION(BlueprintCallable, Category="WU:RE Widget|Control")
	void SetMousePositionByGamepad(const float ValX, const float ValY, float Sensitivity);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// Wrapped Set Button Target Index + Set Button Hovered
	UFUNCTION(BlueprintCallable, Category="WU:RE Widget|Localization")
	void FocusOnHoveredElement(UWuReWidgetComponent* HoveredElement);

	// {0}의 부분이 Code로 대체됨
	UFUNCTION(BlueprintCallable, Category="WU:RE Widget|Localization")
	bool UpdateTextBlockLanguageUI(UTextBlock* TextBlock,
		const FString Code, const FText FormatText = FText::GetEmpty());

	bool bAutoAddWidgetToPlayerController = true;
	
	FWuReLocalizationStruct GetLocalizedDialogueStruct(const FName RowName);

	UFUNCTION(BlueprintPure, Category="WU:RE Widget|Localization", DisplayName="Get Localized Dialogue Context")
	FText GetLocalizedDialogue(const FName RowName)
	{
		return GetWuReGameInstance()->GetIsKorean()
			? GetLocalizedDialogueStruct(RowName).ContextKoKr
			: GetLocalizedDialogueStruct(RowName).ContextEnGb;
	}

	FWuReLocalizationStruct GetLocalizedUIContextStruct(const FName RowName);
	
	UFUNCTION(BlueprintPure, Category="WU:RE Widget|Localization", DisplayName="Get Localized UI Context")
	FText GetLocalizedUIContext(const FName RowName)
	{
		return GetWuReGameInstance()->GetIsKorean()
			? GetLocalizedUIContextStruct(RowName).ContextKoKr
			: GetLocalizedUIContextStruct(RowName).ContextEnGb;
	}

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBeginPlayDone);
	UPROPERTY(BlueprintAssignable, Category="WU:RE Widget|Delegates")
	FOnBeginPlayDone OnBeginPlayDone;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="WU:RE Widget|Localization")
	TObjectPtr<UDataTable> UILocalizationTable;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="WU:RE Widget|Localization")
	TObjectPtr<UDataTable> DialogueLocalizationTable;
	
private:
	TArray<UWuReWidgetComponent*> WidgetComponents;
	int FocusedBtnTarget = 0;
	
	static bool SetGamepadIcon(UMaterialInstanceDynamic* MatParam, const EDualSenseKey GamepadEnum);
};
