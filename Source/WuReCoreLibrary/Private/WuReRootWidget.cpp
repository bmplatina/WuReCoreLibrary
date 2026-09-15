// © Platina Production 2017~2024


#include "WuReRootWidget.h"

#include "InputActionValue.h"
#include "WuReUserWidget.h"
#include "Components/NamedSlot.h"

UWuReUserWidget* UWuReRootWidget::CreateAndAddWidgetToViewport(const TSubclassOf<UWuReUserWidget> InWidgetClass,
                                                               EInputMode InputMode)
{
	// RebuildWidget()이 아직 한 번도 호출되지 않았다면(=이 위젯이 아직 AddToViewport되지 않았다면)
	// GameplayWidgetStack이 비어있을 수 있으므로, 여기서 강제로 Slate 위젯을 빌드해 보장합니다.
	TakeWidget();

	if (!ensure(GameplayWidgetStack))
	{
		return nullptr;
	}

	if (APlayerController* OwningPlayer = GetOwningPlayer())
	{
		if (InputMode == EInputMode::GameOnly)
		{
			OwningPlayer->SetInputMode(FInputModeGameOnly());
		}
		else if (InputMode == EInputMode::WidgetOnly)
		{
			OwningPlayer->SetInputMode(FInputModeUIOnly());
		}
		else if (InputMode == EInputMode::GameAndWidget)
		{
			OwningPlayer->SetInputMode(FInputModeGameAndUI());
		}
	}

	return GameplayWidgetStack->AddWidget<UWuReUserWidget>(InWidgetClass);
}

void UWuReRootWidget::RemoveWidgetFromViewport(UWuReUserWidget* InWidget, EInputMode InputMode)
{
	if (!ensure(GameplayWidgetStack))
	{
		return;
	}

	if (InWidget)
	{
		GameplayWidgetStack->RemoveWidget(*InWidget);
		if (APlayerController* OwningPlayer = GetOwningPlayer())
		{
			if (InputMode == EInputMode::GameOnly)
			{
				OwningPlayer->SetInputMode(FInputModeGameOnly());
			}
			else if (InputMode == EInputMode::WidgetOnly)
			{
				OwningPlayer->SetInputMode(FInputModeUIOnly());
			}
			else if (InputMode == EInputMode::GameAndWidget)
			{
				OwningPlayer->SetInputMode(FInputModeGameAndUI());
			}
		}
	}
}

UWuReUserWidget* UWuReRootWidget::CreateAndAddWidgetToEscapeMenuStack(const TSubclassOf<UWuReUserWidget> InWidgetClass)
{
	// RebuildWidget()이 아직 한 번도 호출되지 않았다면(=이 위젯이 아직 AddToViewport되지 않았다면)
	// GameplayWidgetStack이 비어있을 수 있으므로, 여기서 강제로 Slate 위젯을 빌드해 보장합니다.
	TakeWidget();

	if (!ensure(EscapeMenuWidgetStack))
	{
		return nullptr;
	}

	return EscapeMenuWidgetStack->AddWidget<UWuReUserWidget>(InWidgetClass);
}

void UWuReRootWidget::RemoveWidgetFromEscapeMenuStack(UWuReUserWidget* InWidget)
{
	if (!ensure(EscapeMenuWidgetStack))
	{
		return;
	}

	if (InWidget)
	{
		EscapeMenuWidgetStack->RemoveWidget(*InWidget);
	}
}


UCommonActivatableWidgetStack* UWuReRootWidget::GetWidgetStack()
{
	return GameplayWidgetStack;
}

bool UWuReRootWidget::GetRootHasWidget(UWuReUserWidget* TestWidget)
{
	TArray<UCommonActivatableWidget*> AllWidgets = GameplayWidgetStack->GetWidgetList();;

	for (UCommonActivatableWidget* Widget : AllWidgets)
	{
		if (Widget == TestWidget)
		{
			return true;
		}
	}
	return false;
}

void UWuReRootWidget::ToggleEscMenu()
{
	if (!IsValid(EscapeMenuWidgetClass)) return;
	
	if (UCommonActivatableWidget* ActiveWidgetFromEscapeMenuStack = EscapeMenuWidgetStack->GetActiveWidget())
	{
		if (ActiveWidgetFromEscapeMenuStack->GetClass()->IsChildOf(EscapeMenuWidgetClass)) // Escape 메뉴만이 스택에 활성 상태일 때
		{
			EscapeMenuWidgetStack->ClearWidgets(); // 스택 flush
			GetOwningPlayer()->SetInputMode(FInputModeGameOnly());
			return;
		}
	}

	UWuReUserWidget* ActivatedWidget = Cast<UWuReUserWidget>(GameplayWidgetStack->GetActiveWidget());

	// 먼저 Esc 관련 위젯 스택에 위젯이 있으면 그거부터 지운다
	if (EscapeMenuWidgetStack->GetWidgetList().Num() > 0)
	{
		Cast<UWuReUserWidget>(EscapeMenuWidgetStack->GetActiveWidget())->HandleBackTriggered(FInputActionValue());
	}
	else
	{
		if (!ActivatedWidget || ActivatedWidget->bIsStaticWidget) // 뒤로가기 기능이 없는 정적 위젯일 때
		{
			UWuReUserWidget* EscapeMenuWidgetLocal = CreateAndAddWidgetToEscapeMenuStack(EscapeMenuWidgetClass);
			GetOwningPlayer()->SetInputMode(FInputModeUIOnly());
		}
		else
		{
			ActivatedWidget->HandleBackTriggered(FInputActionValue());
		}
	}
}
