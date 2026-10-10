// © Platina Production 2017~2024


#include "WuReSubtitleTextBlock.h"
#include "WuReSubtitleWorldSubsystem.h"

UWuReSubtitleTextBlock::UWuReSubtitleTextBlock()
{
	LocalizationCategory = ELocalizationType::Dialogue;
	SetVisibility(ESlateVisibility::Collapsed);
}

TSharedRef<SWidget> UWuReSubtitleTextBlock::RebuildWidget()
{
	const TSharedRef<SWidget> Widget = Super::RebuildWidget();
	
	if (IsDesignTime()) return Widget;           // 디자이너 제외 + 중복 구독 방지
	
	if (UWuReSubtitleWorldSubsystem* Sub = GetWorld()->GetSubsystem<UWuReSubtitleWorldSubsystem>())
	{
		SubtitleWorldSubsystem = Sub;
		SubtitleWorldSubsystem->OnSubtitleStateChanged.AddUniqueDynamic(this, &UWuReSubtitleTextBlock::HandleSubtitleStateChanged);
	}
	return Widget;
}

void UWuReSubtitleTextBlock::ReleaseSlateResources(bool bReleaseChildren)
{
	if (SubtitleWorldSubsystem.IsValid())
	{
		SubtitleWorldSubsystem->OnSubtitleStateChanged.RemoveAll(this);
	}
	Super::ReleaseSlateResources(bReleaseChildren);
}

void UWuReSubtitleTextBlock::HandleSubtitleStateChanged(const FName RowName)
{
	if (RowName.IsNone())
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		SetText(FText::FromName(RowName));
	}
}
