// © Platina Production 2017~2024

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "WuReTypes.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "WuReRootWidget.generated.h"

class UWuReUserWidget;
class UNamedSlot;

/**
 *
 */
UCLASS()
class WURECORELIBRARY_API UWuReRootWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category="Widget|Stack")
	UWuReUserWidget* CreateAndAddWidgetToViewport(const TSubclassOf<UWuReUserWidget> InWidgetClass, EInputMode InputMode);
	
	UFUNCTION(BlueprintCallable, Category="Widget|Stack")
	void RemoveWidgetFromViewport(UWuReUserWidget* InWidget, EInputMode InputMode);
	
	UFUNCTION(BlueprintPure, Category="Input|Widget")
	bool GetRootHasWidget(UWuReUserWidget* TestWidget);
	
	UFUNCTION(BlueprintPure, Category="Widget")
	UCommonActivatableWidgetStack* GetWidgetStack();
	
	UFUNCTION(BlueprintCallable, Category="Widget|EscapeMenu")
	void ToggleEscMenu();	// const FInputActionValue& Value
	
	UFUNCTION(BlueprintCallable, Category="Widget|Stack")
	UWuReUserWidget* CreateAndAddWidgetToEscapeMenuStack(const TSubclassOf<UWuReUserWidget> InWidgetClass);
	
	UFUNCTION(BlueprintCallable, Category="Widget|Stack")
	void RemoveWidgetFromEscapeMenuStack(UWuReUserWidget* InWidget);
	
	

protected:
	UPROPERTY(BlueprintReadOnly, Category="Widget|Stack", meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> GameplayWidgetStack;
	
	UPROPERTY(BlueprintReadOnly, Category="Widget|Stack", meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> EscapeMenuWidgetStack;
	
	UPROPERTY(BlueprintReadOnly, Category="Widget|EscapeMenu", meta=(BindWidget))
	TObjectPtr<UNamedSlot> EscapeMenuWidgetSlot;
	UPROPERTY(EditDefaultsOnly, Category="Widget|EscapeMenu")
	TSubclassOf<UWuReUserWidget> EscapeMenuWidgetClass;
};
