// © Platina Production 2017~2024

#pragma once

#include "WuReCoreLibrary.h"
#include "WuReUserWidget.h"
#include "WuReWidgetComponent.generated.h"

class UTextBlock;

/**
 * 
 */
UCLASS(DisplayName="WU:RE Widget Component")
class WURECORELIBRARY_API UWuReWidgetComponent : public UWuReUserWidget
{
	GENERATED_BODY()

public:
	virtual UWidget* GetWidgetComponent();

	UFUNCTION(BlueprintCallable, Category="Widget Component")
	virtual void SetElementHovered(bool bIsBtnHovered);
	UFUNCTION(BlueprintCallable, Category="Widget Component")
	virtual void SetElementClicked(bool bIsBtnClicked);

	UFUNCTION(BlueprintCallable, Category="Widget Component")
	virtual UTextBlock* GetComponentText();

	// On LR Arrow or LR Button Clicked
	virtual void OnControlLR(const bool bIsLeft);
};