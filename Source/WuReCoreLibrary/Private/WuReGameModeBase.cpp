// © Platina Production 2017~2024

#include "WuReGameModeBase.h"
#include "AsyncLevelStreamingSubsystem.h"
#include "Engine/GameInstance.h"
#include "GameFramework/Pawn.h"

UClass* AWuReGameModeBase::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (const UAsyncLevelStreamingSubsystem* Subsystem = GetLevelStreamingSubsystem())
	{
		if (const TSubclassOf<APawn> RowPawnClass = Subsystem->GetPawnClassForRow(Subsystem->GetEffectiveRowName()))
		{
			return RowPawnClass;
		}
	}
	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

AActor* AWuReGameModeBase::ChoosePlayerStart_Implementation(AController* Player)
{
	if (const UAsyncLevelStreamingSubsystem* Subsystem = GetLevelStreamingSubsystem())
	{
		if (AActor* SpawnPoint = Subsystem->FindSpawnPointForRow(Subsystem->GetEffectiveRowName()))
		{
			return SpawnPoint;
		}
	}
	return Super::ChoosePlayerStart_Implementation(Player);
}

UAsyncLevelStreamingSubsystem* AWuReGameModeBase::GetLevelStreamingSubsystem() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UAsyncLevelStreamingSubsystem>() : nullptr;
}
