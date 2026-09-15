// © Platina Production 2017~2024

#pragma once

#include "CommonInputTypeEnum.h"
#include "WuReTypes.h"
#include "GameFramework/PlayerController.h"
#include "WuRePlayerController.generated.h"

class UWuReUserWidget;
class UInputAction;
class UInputMappingContext;
class UCommonInputSubsystem;
class UWuReRootWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLanguageChangedEvent, bool, bIsKorean);

/**
 * 
 */
UCLASS(DisplayName="WU:RE Player Controller")
class WURECORELIBRARY_API AWuRePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void PostInitializeComponents() override;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInputMethodChangedEvent, ECommonInputType, InInputType);

	UPROPERTY(BlueprintAssignable, Category="Input|Type")
	FInputMethodChangedEvent OnInputMethodChangedNative;

	UFUNCTION(BlueprintPure, Category="Input|Type")
	ECommonInputType GetCurrentInputMethod() const
	{
		return CurrentInputType;
	}

	bool IsLocationInViewport(const FVector& Location);

	UPROPERTY(BlueprintAssignable, Category="WU:RE Widget|Delegates")
	FOnBeginPlayDone OnBeginPlayDone;

	UFUNCTION(BlueprintPure, Category="WU:RE Widget")
	UWuReRootWidget* GetRootWidget();
	
	UFUNCTION(BlueprintPure, Category="WU:RE Widget", DisplayName="Get All WU:RE Widgets")
	void GetAllWuReWidgetsInRoot(TArray<UWuReUserWidget*>& OutWidgets);

	UFUNCTION(BlueprintPure, Category="WU:RE Widget", DisplayName="Get Activated WU:RE Widgets")
	UWuReUserWidget* GetActivatedWuReWidget();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void SetupInputComponent() override;

	ECommonInputType CurrentInputType;

	UPROPERTY(EditDefaultsOnly, meta=(AllowPrivateAccess=true), Category="WU:RE Widget", DisplayName="Root Widget")
	TSubclassOf<UWuReRootWidget> RootWidgetStaticClass;
	
	UPROPERTY(EditDefaultsOnly, meta=(AllowPrivateAccess=true), Category="WU:RE Widget", DisplayName="Escape Menu Widget")
	TSubclassOf<UWuReUserWidget> EscapeMenuWidgetStaticClass;
	
	UPROPERTY(EditDefaultsOnly, Category="WU:RE Widget|Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;
	
	UPROPERTY(EditDefaultsOnly, Category="WU:RE Widget|Input")
	TObjectPtr<UInputAction> IA_Escape;
	
	UFUNCTION()
	void ToggleEscMenu();

private:
	void BroadcastOnInputMethodChanged(ECommonInputType InInputType);

	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Input|Widget")
	TObjectPtr<UWuReRootWidget> RootWidget;
	
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Input|Widget")
	TObjectPtr<UWuReUserWidget> EscapeMenuWidget;
};
