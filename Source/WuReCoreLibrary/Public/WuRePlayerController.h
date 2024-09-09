// © Platina Production 2017~2024

#pragma once

#include "WuReCoreLibrary.h"
#include "CommonInputTypeEnum.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "WuRePlayerController.generated.h"

class UWuReUserWidget;
class UInputAction;
class UInputMappingContext;
class UCommonInputSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLanguageChangedEvent, bool, bIsKorean);
/**
 * 
 */
UCLASS()
class WURECORELIBRARY_API AWuRePlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual void OnPossess(APawn* InPawn) override;

	UPROPERTY(BlueprintAssignable, Category="Widget|Delegate")
	FLanguageChangedEvent OnLanguageChangedNative;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Widget|References")
	TArray<UWuReUserWidget*> Widgets;
	
	
	UFUNCTION(BlueprintPure, Category="Input|Type")
	ECommonInputType GetCurrentInputMethod() const
	{
		return CurrentInputType;
	}
	
	UFUNCTION(BlueprintPure, Category="Widget|Functions", DisplayName="Get Activated Widget")
	UWuReUserWidget* GetValidWidget(const bool bAdditionalConditions, const bool bIgnoreIsInViewport);

	bool IsLocationInViewport(const FVector& Location);

protected:
	virtual void ToggleEscMenu();
	ECommonInputType CurrentInputType;
	
private:
	UFUNCTION()
	void SendInputMethodToWidgets(const ECommonInputType InputEnum);

	virtual void SetupInputComponent() override;

	UPROPERTY(EditAnywhere, Category = "Input|Mapping Context", meta=(AllowPrivateAccess="true"), DisplayName="Input Mapping Context")
	TObjectPtr<UInputMappingContext> InputMappingContext;
	UPROPERTY(EditAnywhere, Category="Input|Action", meta=(AllowPrivateAccess="true"), DisplayName="Escape Key")
	TObjectPtr<UInputAction> InputActionEscapeKey;
	UPROPERTY(EditAnywhere, Category="Input|Action", meta=(AllowPrivateAccess="true"), DisplayName="Gamepad Left Stick")
	TObjectPtr<UInputAction> InputActionLeftStick;
	UPROPERTY(EditAnywhere, Category="Input|Action", meta=(AllowPrivateAccess="true"), DisplayName="Gamepad D-Pad Left")
	TObjectPtr<UInputAction> InputActionDPadLeft;
	UPROPERTY(EditAnywhere, Category="Input|Action", meta=(AllowPrivateAccess="true"), DisplayName="Gamepad D-Pad Up")
	TObjectPtr<UInputAction> InputActionDPadUp;
	UPROPERTY(EditAnywhere, Category="Input|Action", meta=(AllowPrivateAccess="true"), DisplayName="Gamepad D-Pad Right")
	TObjectPtr<UInputAction> InputActionDPadRight;
	UPROPERTY(EditAnywhere, Category="Input|Action", meta=(AllowPrivateAccess="true"), DisplayName="Gamepad D-Pad Down")
	TObjectPtr<UInputAction> InputActionDPadDown;
	UPROPERTY(EditAnywhere, Category="Input|Action", meta=(AllowPrivateAccess="true"), DisplayName="Gamepad Check Button")
	TObjectPtr<UInputAction> InputActionGamepadCheck;
	UPROPERTY(EditAnywhere, Category="Input|Action", meta=(AllowPrivateAccess="true"), DisplayName="Gamepad Back Button")
	TObjectPtr<UInputAction> InputActionGamepadBack;
	
	// Gamepad Left Stick Dead Zone Events
	void DirectionKey(const FInputActionValue& Value);
    
	// Gamepad D-Pad Events
	void DPadLeft(const FInputActionValue& Value);
	void DPadUp(const FInputActionValue& Value);
	void DPadRight(const FInputActionValue& Value);
	void DPadDown(const FInputActionValue& Value);
	
	// Gamepad Check Button Events
	void CheckBtnStarted(const FInputActionValue& Value);	// On Pressed
	void CheckBtnCompleted(const FInputActionValue& Value);	// On Post-Pressed

	// Gamepad Back Button Events
	void BackBtnCompleted(const FInputActionValue& Value);
};
