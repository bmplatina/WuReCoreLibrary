// © Platina Production 2017~2024

#include "WuReAsyncWaitForPlayerPawn.h"
#include "AsyncLevelStreamingSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

UWuReAsyncWaitForPlayerPawn* UWuReAsyncWaitForPlayerPawn::WaitForPlayerPawn(const UObject* WorldContextObject)
{
	UWuReAsyncWaitForPlayerPawn* Action = NewObject<UWuReAsyncWaitForPlayerPawn>();
	Action->World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (const UWorld* ContextWorld = Action->World.Get())
	{
		if (const UGameInstance* GameInstance = ContextWorld->GetGameInstance())
		{
			Action->Subsystem = GameInstance->GetSubsystem<UAsyncLevelStreamingSubsystem>();
		}
	}
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UWuReAsyncWaitForPlayerPawn::Activate()
{
	UWorld* ContextWorld = World.Get();
	if (ContextWorld == nullptr)
	{
		SetReadyToDestroy();
		return;
	}

	UAsyncLevelStreamingSubsystem* LevelStreamingSubsystem = Subsystem.Get();
	if (LevelStreamingSubsystem != nullptr && !LevelStreamingSubsystem->IsPlayerPawnReady())
	{
		LevelStreamingSubsystem->OnPlayerPawnReady.AddDynamic(this, &UWuReAsyncWaitForPlayerPawn::HandlePlayerPawnReady);
		return;
	}

	// 이미 준비됨(또는 서브시스템 없음): 노드의 다음 실행 핀이 먼저 진행되도록 다음 틱에 현재 Pawn으로 완료
	ContextWorld->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		const UWorld* TimerWorld = World.Get();
		const APlayerController* PlayerController = TimerWorld ? TimerWorld->GetFirstPlayerController() : nullptr;
		Finish(PlayerController ? PlayerController->GetPawn() : nullptr);
	}));
}

void UWuReAsyncWaitForPlayerPawn::HandlePlayerPawnReady(APawn* Pawn)
{
	Finish(Pawn);
}

void UWuReAsyncWaitForPlayerPawn::Finish(APawn* Pawn)
{
	if (UAsyncLevelStreamingSubsystem* LevelStreamingSubsystem = Subsystem.Get())
	{
		LevelStreamingSubsystem->OnPlayerPawnReady.RemoveDynamic(this, &UWuReAsyncWaitForPlayerPawn::HandlePlayerPawnReady);
	}

	OnReady.Broadcast(Pawn);
	SetReadyToDestroy();
}
