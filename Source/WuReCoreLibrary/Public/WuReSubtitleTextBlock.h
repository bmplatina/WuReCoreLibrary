// © Platina Production 2017~2024

#pragma once

#include "CoreMinimal.h"
#include "WuReTextBlock.h"
#include "WuReSubtitleTextBlock.generated.h"

class UWuReSubtitleWorldSubsystem;
/**
 * 
 */
UCLASS(meta=(DisplayName="WU:RE Localized Subtitle Text Block"))
class WURECORELIBRARY_API UWuReSubtitleTextBlock : public UWuReTextBlock
{
	GENERATED_BODY()
	
public:
	UWuReSubtitleTextBlock();
	
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	
private:
	UPROPERTY()
	TWeakObjectPtr<UWuReSubtitleWorldSubsystem> SubtitleWorldSubsystem;
	
	UFUNCTION()
	void HandleSubtitleStateChanged(const FName RowName);
};
