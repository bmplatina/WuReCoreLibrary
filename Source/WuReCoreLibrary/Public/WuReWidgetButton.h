// © Platina Production 2017~2024

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "WuReTextBlock.h"
#include "WuReWidgetButton.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, DisplayName="WU:RE Button")
class WURECORELIBRARY_API UWuReWidgetButton : public UCommonButtonBase
{
	GENERATED_BODY()
	
protected:
	virtual void NativePreConstruct() override;
	
	UPROPERTY(BlueprintReadOnly, Category="Button|References", meta = (BindWidget))
	TObjectPtr<UWuReTextBlock> TextBlock;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Button|Text")
	FText Text;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Button|Text|Layout")
	FMargin TextPadding;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Button|Text|Layout")
	TEnumAsByte<EHorizontalAlignment> TextHorizontalAlignment = HAlign_Center;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Button|Text|Layout")
	TEnumAsByte<EVerticalAlignment> TextVerticalAlignment = VAlign_Center;
};
