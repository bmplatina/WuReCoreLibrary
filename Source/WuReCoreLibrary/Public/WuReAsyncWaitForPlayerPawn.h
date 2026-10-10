// © Platina Production 2017~2024

#pragma once

#include "WuReCoreLibrary.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "WuReAsyncWaitForPlayerPawn.generated.h"

class APawn;
class UAsyncLevelStreamingSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWuReWaitForPlayerPawnOutput, APawn*, Pawn);

/**
 * 레벨 진입 처리(Pawn 스폰/Possess/배치)가 끝날 때까지 기다림. 이미 끝나 있으면 다음 틱에 바로 완료.
 * BeginPlay에서 GetPlayerCharacter 대신 사용하면 스트리밍 타이밍과 관계없이 올바른 Pawn을 받음
 */
UCLASS()
class WURECORELIBRARY_API UWuReAsyncWaitForPlayerPawn : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FWuReWaitForPlayerPawnOutput OnReady;

	UFUNCTION(BlueprintCallable, Category="Level Streaming",
		meta=(BlueprintInternalUseOnly="true", WorldContext="WorldContextObject", DisplayName="Wait For Player Pawn"))
	static UWuReAsyncWaitForPlayerPawn* WaitForPlayerPawn(const UObject* WorldContextObject);

	virtual void Activate() override;

private:
	UFUNCTION()
	void HandlePlayerPawnReady(APawn* Pawn);

	void Finish(APawn* Pawn);

	TWeakObjectPtr<UWorld> World;
	TWeakObjectPtr<UAsyncLevelStreamingSubsystem> Subsystem;
};
