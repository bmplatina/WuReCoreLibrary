// © Platina Production 2017~2024

#include "AsyncLevelStreamingSubsystem.h"
#include "LevelListStruct.h"
#include "Engine/World.h"
#include "Engine/LevelStreamingDynamic.h"
#include "TimerManager.h"

void UAsyncLevelStreamingSubsystem::Deinitialize()
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ProgressTimerHandle);
	}
	Super::Deinitialize();
}

bool UAsyncLevelStreamingSubsystem::RequestLoadLevelByRowName(const FName RowName, const FVector Location,
                                                              const FRotator Rotation, FText& DisplayedLevelText)
{
	if (!IsValid(LevelTable))
	{
		return false;
	}

	const FLevelListStruct* Row = LevelTable->FindRow<FLevelListStruct>(RowName, TEXT("RequestLoadLevelByRowName"));
	if (Row == nullptr || Row->LevelAsset.IsNull())
	{
		return false;
	}
	DisplayedLevelText = Row->DisplayNameForLoadingScreen;

	bool bSuccess = false;
	ULevelStreamingDynamic* NewLevel = ULevelStreamingDynamic::LoadLevelInstance(
		this,
		Row->LevelAsset.ToSoftObjectPath().GetLongPackageName(),
		Location,
		Rotation,
		bSuccess);

	if (!bSuccess || NewLevel == nullptr)
	{
		return false;
	}

	// 로드만 먼저 진행, 완료 후 한꺼번에 Visible 전환 (팝인 방지)
	NewLevel->SetShouldBeVisible(false);

	PendingLevels.Add(NewLevel);
	LoadedLevelsByRow.Add(RowName, NewLevel);
	PendingLevelsOriginalCount = PendingLevels.Num() + FinishedLevels.Num();

	if (UWorld* World = GetWorld())
	{
		if (!World->GetTimerManager().IsTimerActive(ProgressTimerHandle))
		{
			World->GetTimerManager().SetTimer(
				ProgressTimerHandle, this, &UAsyncLevelStreamingSubsystem::UpdateProgress, 0.05f, true);
		}
	}

	return true;
}

void UAsyncLevelStreamingSubsystem::UnloadLevelByRowName(const FName RowName)
{
	if (const TObjectPtr<ULevelStreamingDynamic>* Found = LoadedLevelsByRow.Find(RowName))
	{
		if (ULevelStreamingDynamic* Level = Found->Get())
		{
			Level->SetIsRequestingUnloadAndRemoval(true);
		}
		LoadedLevelsByRow.Remove(RowName);
	}
}

void UAsyncLevelStreamingSubsystem::UpdateProgress()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	// 이전 틱에 이미 완료된 레벨(100%) + 이번 틱 진행중인 레벨의 부분 진행률
	float Total = static_cast<float>(FinishedLevels.Num());

	for (int32 i = PendingLevels.Num() - 1; i >= 0; --i)
	{
		ULevelStreamingDynamic* Level = PendingLevels[i];
		if (!IsValid(Level))
		{
			PendingLevels.RemoveAt(i);
			continue;
		}

		if (Level->IsLevelLoaded())
		{
			Total += 1.f;
			FinishedLevels.Add(Level);
			PendingLevels.RemoveAt(i);
			continue;
		}

		const float Pct = GetAsyncLoadPercentage(Level->GetWorldAssetPackageFName());
		Total += (Pct >= 0.f) ? (Pct / 100.f) : 0.f;
	}

	const float Progress = PendingLevelsOriginalCount > 0
		                       ? FMath::Clamp(Total / PendingLevelsOriginalCount, 0.f, 1.f)
		                       : 1.f;

	const bool bIsComplete = PendingLevels.Num() == 0;
	OnLevelStreamingProgress.Broadcast(Progress, bIsComplete);

	if (bIsComplete)
	{
		for (ULevelStreamingDynamic* Level : FinishedLevels)
		{
			if (IsValid(Level))
			{
				Level->SetShouldBeVisible(true);
			}
		}
		FinishedLevels.Empty();
		PendingLevelsOriginalCount = 0;
		World->GetTimerManager().ClearTimer(ProgressTimerHandle);
	}
}
