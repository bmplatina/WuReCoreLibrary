// © Platina Production 2017~2024


#include "WuReTextBlock.h"

#include "LocalizationSubsystem.h"
#include "WuReLocalizationStruct.h"
#include "Engine/GameInstance.h"


void UWuReTextBlock::SetText(FText InText)
{
	Super::SetText(InText);
}

TAttribute<FText> UWuReTextBlock::GetDisplayText()
{
	return TAttribute<FText>::CreateLambda([this]() -> FText
	{
		return ResolveLocalizedText();
	});
}

FText UWuReTextBlock::ResolveLocalizedText() const
{
	// GetText()는 위젯 생성 후 내부 슬레이트 위젯(MyTextBlock)의 현재 표시 텍스트를 반환하는데,
	// 그 텍스트는 바로 이 함수가 GetDisplayText()를 통해 바인딩한 값이라 GetText()를 다시 부르면 무한 재귀에 빠진다.
	// 그래서 원본 Text 프로퍼티를 직접 읽는다.
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	const FString Code = Text.ToString();
	PRAGMA_ENABLE_DEPRECATION_WARNINGS

	if (IsDesignTime() || !LocalizationSubsystem)
	{
		return FText::FromString(Code);
	}

	const ELocalizationType ContextType = Code.Contains(TEXT("UI"))
		                                      ? ELocalizationType::UserInterface
		                                      : ELocalizationType::Dialogue;

	return LocalizationSubsystem->GetLocalizedText(Code, ContextType);
}

TSharedRef<SWidget> UWuReTextBlock::RebuildWidget()
{
	const TSharedRef<SWidget> Widget = Super::RebuildWidget();

	if (!IsDesignTime() && GetWorld() && GetWorld()->GetGameInstance())
	{
		LocalizationSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<ULocalizationSubsystem>();
		if (LocalizationSubsystem)
		{
			LocalizationSubsystem->OnLocaleChanged.RemoveAll(this);
			LocalizationSubsystem->OnLocaleChanged.AddDynamic(this, &UWuReTextBlock::HandleLocaleChanged);
		}
	}

	return Widget;
}

void UWuReTextBlock::ReleaseSlateResources(bool bReleaseChildren)
{
	if (LocalizationSubsystem)
	{
		LocalizationSubsystem->OnLocaleChanged.RemoveAll(this);
	}

	Super::ReleaseSlateResources(bReleaseChildren);
}

void UWuReTextBlock::HandleLocaleChanged(ESupportedLanguage NewLanguage)
{
	SynchronizeProperties();
}
