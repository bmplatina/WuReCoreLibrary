// © Platina Production 2017~2024

#pragma once

#include "CoreMinimal.h"
#include "CommonTextBlock.h"
#include "WuReUserWidget.h"
#include "WuReTextBlock.generated.h"

/**
 * Details 패널의 Text 필드에 로컬라이제이션 DataTable의 RowName(텍스트 코드)을 입력하면,
 * 현재 언어 설정에 맞는 번역된 텍스트를 실제로 표시하는 UCommonTextBlock.
 * RowName을 찾지 못하면 입력한 코드 문자열을 그대로 표시한다 (누락 확인용).
 */
UCLASS(meta=(DisplayName="WU:RE Localized Text Block"))
class WURECORELIBRARY_API UWuReTextBlock : public UCommonTextBlock
{
	GENERATED_BODY()

public:
	virtual void SetText(FText InText) override;

	UPROPERTY(EditAnywhere, Category="WU:RE Widget|Localization")
	ELocalizationType LocalizationCategory = ELocalizationType::UserInterface;

protected:
	// Details 패널 값(=코드)이 설정될 때도, 런타임에 SetText가 호출될 때도 이 경로를 통해 실제 표시 텍스트가 결정된다.
	virtual TAttribute<FText> GetDisplayText() override;

	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

private:
	FText ResolveLocalizedText() const;
	
	UFUNCTION()
	void HandleLocaleChanged(ESupportedLanguage NewLanguage);

	UPROPERTY()
	TObjectPtr<ULocalizationSubsystem> LocalizationSubsystem;
};
