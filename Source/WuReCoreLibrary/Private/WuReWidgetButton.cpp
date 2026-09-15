// © Platina Production 2017~2024

#include "WuReWidgetButton.h"
#include "Components/ButtonSlot.h"

void UWuReWidgetButton::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	if (IsValid(TextBlock))
	{
		TextBlock->SetText(Text);
		
		if (UButtonSlot* ButtonSlot = Cast<UButtonSlot>(TextBlock->Slot))
		{
			ButtonSlot->SetPadding(TextPadding);
			ButtonSlot->SetHorizontalAlignment(TextHorizontalAlignment);
			ButtonSlot->SetVerticalAlignment(TextVerticalAlignment);
		}
	}
}
