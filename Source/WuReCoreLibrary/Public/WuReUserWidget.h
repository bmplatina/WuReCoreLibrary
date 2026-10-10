// © Platina Production 2017~2024

#pragma once

#include "WuReCoreLibrary.h"
#include "CommonActivatableWidget.h"
#include "CommonInputTypeEnum.h"
#include "WuReTypes.h"
#include "WuReUserWidget.generated.h"

/**
 * 
 */

class ULocalizationSubsystem;
class UWuReWidgetComponent;
class AWuRePlayerController;
class UTextBlock;
class UWuReGameInstance;
class UDataTable;
class UInputAction;
class UInputMappingContext;
class UEnhancedInputComponent;
struct FInputActionValue;

UCLASS(Abstract, DisplayName="WU:RE Base Widget")
class WURECORELIBRARY_API UWuReUserWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
	
	void SetDesiredInputMode(const EInputMode InInputMode);
	
	UPROPERTY(EditAnywhere, Category="WU:RE Widget|Sound")
	TObjectPtr<USoundBase> ClickSound;

	UPROPERTY(EditAnywhere, Category="WU:RE Widget|Design", DisplayName="DualSense Button Material Base")
	TObjectPtr<UMaterialInterface> DualSenseBtnMaterial;

	// Keyboard-Gamepad Controls
	UFUNCTION(BlueprintNativeEvent, Category="WU:RE Widget|Control")
	void KeyboardActivated();

	UFUNCTION(BlueprintNativeEvent, Category="WU:RE Widget|Control")
	void GamepadActivated();

	UFUNCTION(BlueprintPure, Category="WU:RE Widget|References")
	AWuRePlayerController* GetPlayerController(bool& bIsValid) const;

	// DualSense Icon Materials
	UFUNCTION(BlueprintPure, Category="WU:RE Widget|Design")
	UMaterialInstanceDynamic* CreateGamepadIcon(const EDualSenseKey GamepadKey, const bool AddBackgroundTexture,
	                                            const bool BackgroundTextureOnly,
	                                            const FColor Color = FColor(255, 255, 255));

	UFUNCTION(BlueprintCallable, Category="WU:RE Widget|Design", DisplayName="Set Gamepad Icon")
	void SetGamepadIconToMaterial(UMaterialInstanceDynamic* Material, const EDualSenseKey GamepadKey,
	                              const bool AddBackgroundTexture, const bool BackgroundTextureOnly,
	                              const FColor Color = FColor(255, 255, 255));

	UFUNCTION(BlueprintCallable, Category="WU:RE Widget|Control")
	void SetMousePositionByGamepad(const float ValX, const float ValY, float Sensitivity);

	UFUNCTION(BlueprintPure, Category="WU:RE Widget|References")
	ULocalizationSubsystem* GetLocalizationSubsystem(bool& bIsValid);

	UPROPERTY(EditDefaultsOnly, Category="WU:RE Widget|Control")
	bool bIsStaticWidget = false;

	UFUNCTION(BlueprintImplementableEvent, Category="WU:RE Widget|Input")
	void HandleBackTriggered(const FInputActionValue& Value);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	
	UFUNCTION(BlueprintCallable, Category="WU:RE Widget|Input")
	UWidget* GetFocusedWidget();

	UPROPERTY(EditDefaultsOnly, Category="WU:RE Widget|Input")
	TObjectPtr<UInputMappingContext> WidgetInputMappingContext;

	UPROPERTY(EditDefaultsOnly, Category="WU:RE Widget|Input")
	TObjectPtr<UInputAction> IA_Confirm;
	UPROPERTY(EditDefaultsOnly, Category="WU:RE Widget|Input")
	TObjectPtr<UInputAction> IA_InteractionKey;
	UPROPERTY(EditDefaultsOnly, Category="WU:RE Widget|Input")
	TObjectPtr<UInputAction> IA_Back;

	UPROPERTY(EditDefaultsOnly, Category="WU:RE Widget|Input")
	TObjectPtr<UInputAction> IA_Up;
	UPROPERTY(EditDefaultsOnly, Category="WU:RE Widget|Input")
	TObjectPtr<UInputAction> IA_Down;
	UPROPERTY(EditDefaultsOnly, Category="WU:RE Widget|Input")
	TObjectPtr<UInputAction> IA_Left;
	UPROPERTY(EditDefaultsOnly, Category="WU:RE Widget|Input")
	TObjectPtr<UInputAction> IA_Right;
	UPROPERTY(EditDefaultsOnly, Category="WU:RE Widget|Input")
	TObjectPtr<UInputAction> IA_LeftJoystick;
	UPROPERTY(EditDefaultsOnly, Category="WU:RE Widget|Input")
	TObjectPtr<UInputAction> IA_RightJoystick;

	UFUNCTION(BlueprintImplementableEvent, Category="WU:RE Widget|Input")
	void HandleConfirmCompleted(const FInputActionValue& Value);
	UFUNCTION(BlueprintImplementableEvent, Category="WU:RE Widget|Input")
	void HandleBackCompleted(const FInputActionValue& Value);
	UFUNCTION(BlueprintImplementableEvent, Category="WU:RE Widget|Input")
	void HandleLeftFaceKeyStarted(const FInputActionValue& Value);
	UFUNCTION(BlueprintImplementableEvent, Category="WU:RE Widget|Input")
	void HandleLeftFaceKeyTriggered(const FInputActionValue& Value);
	UFUNCTION(BlueprintImplementableEvent, Category="WU:RE Widget|Input")
	void HandleLeftFaceKeyCompleted(const FInputActionValue& Value);
	UFUNCTION(BlueprintImplementableEvent, Category="WU:RE Widget|Input")
	void HandleUpKeyCompleted(const FInputActionValue& Value);
	UFUNCTION(BlueprintImplementableEvent, Category="WU:RE Widget|Input")
	void HandleDownKeyCompleted(const FInputActionValue& Value);
	UFUNCTION(BlueprintImplementableEvent, Category="WU:RE Widget|Input")
	void HandleLeftKeyCompleted(const FInputActionValue& Value);
	UFUNCTION(BlueprintImplementableEvent, Category="WU:RE Widget|Input")
	void HandleRightKeyCompleted(const FInputActionValue& Value);
	UFUNCTION(BlueprintImplementableEvent, Category="WU:RE Widget|Input")
	void HandleLeftJoystickStarted(const FInputActionValue& Value);
	UFUNCTION(BlueprintImplementableEvent, Category="WU:RE Widget|Input")
	void HandleLeftJoystickTriggered(const FInputActionValue& Value);
	UFUNCTION(BlueprintImplementableEvent, Category="WU:RE Widget|Input")
	void HandleRightJoystickStarted(const FInputActionValue& Value);
	UFUNCTION(BlueprintImplementableEvent, Category="WU:RE Widget|Input")
	void HandleRightJoystickTriggered(const FInputActionValue& Value);

	UPROPERTY()
	TObjectPtr<ULocalizationSubsystem> LocalizationSubsystem;

	UPROPERTY(BlueprintAssignable, Category="WU:RE Widget|Delegates")
	FOnBeginPlayDone OnBeginPlayDone;
	
	UPROPERTY(BlueprintReadOnly, Category="WU:RE Widget|Input")
	ECommonInputType InputType = ECommonInputType::MouseAndKeyboard;

private:
	TArray<uint32> EnhancedInputBindingHandles;

	static bool SetGamepadIcon(UMaterialInstanceDynamic* MatParam, const EDualSenseKey GamepadEnum);

	UPROPERTY()
	TObjectPtr<AWuRePlayerController> PlayerController;

	UFUNCTION()
	void HandleInputMethodChanged(ECommonInputType InInputType);
	
	EInputMode InputMode = EInputMode::Default;
};
