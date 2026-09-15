// © Platina Production 2017~2024

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TimerManager.h"
#include "AsyncLevelStreamingSubsystem.generated.h"

class UDataTable;
class ULevelStreamingDynamic;

UCLASS(Abstract, Blueprintable, DisplayName="WU:RE Level Streaming Subsystem")
class WURECORELIBRARY_API UAsyncLevelStreamingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLevelStreamingProgress, float, Progress, bool, bIsComplete);

	UPROPERTY(BlueprintAssignable, Category="Level Streaming")
	FOnLevelStreamingProgress OnLevelStreamingProgress;

	// 도어 액터가 RowName만 들고 있다가 호출 (테이블 조회 -> 비동기 로드)
	UFUNCTION(BlueprintCallable, Category="Level Streaming")
	bool RequestLoadLevelByRowName(const FName RowName, const FVector Location, const FRotator Rotation,
	                               FText& DisplayedLevelText);

	UFUNCTION(BlueprintCallable, Category="Level Streaming")
	void UnloadLevelByRowName(const FName RowName);

	UFUNCTION(BlueprintPure, Category="Level Streaming")
	bool IsStreamingInProgress() const { return PendingLevels.Num() > 0; }

protected:
	// BP 자식 클래스(예: BP_LevelStreamingSubsystem)의 Class Defaults에서 지정
	UPROPERTY(EditDefaultsOnly, Category="Level Streaming")
	TObjectPtr<UDataTable> LevelTable;

private:
	void UpdateProgress();

	UPROPERTY()
	TArray<TObjectPtr<ULevelStreamingDynamic>> PendingLevels;

	UPROPERTY()
	TArray<TObjectPtr<ULevelStreamingDynamic>> FinishedLevels;

	UPROPERTY()
	TMap<FName, TObjectPtr<ULevelStreamingDynamic>> LoadedLevelsByRow;

	int32 PendingLevelsOriginalCount = 0;

	FTimerHandle ProgressTimerHandle;
};
