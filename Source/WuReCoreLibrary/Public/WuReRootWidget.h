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
	void RemoveWidgetFromViewport(UWuReUserWidget* InWidget); // , EInputMode InputMode);
	
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
	UPROPERTY(EditDefaultsOnly, Category="Widget|EscapeMenu")
	TSubclassOf<UWuReUserWidget> EscapeMenuWidgetClass;
	
private:
	// Escape 메뉴가 열려 있는 동안 게임플레이 스택의 활성 위젯을 숨기거나 다시 보이게 함
	void SetGameplayActiveWidgetVisibility(ESlateVisibility InVisibility);
};
