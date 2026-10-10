// © Platina Production 2017~2024

#pragma once

#include "Engine/DataTable.h"
#include "GameFramework/Pawn.h"
#include "LevelListStruct.generated.h"

USTRUCT(BlueprintType, DisplayName="WU:RE Level List Struct")
struct FLevelListStruct : public FTableRowBase
{
	GENERATED_BODY()

	// 이 row로 진입할 때 함께 스트리밍할 레벨들. 진입 시 기존 스트리밍 레벨은 전부 언로드되고 이 목록이 전부 새로 로드됨
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level Streaming")
	TArray<TSoftObjectPtr<UWorld>> LevelAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level Streaming", meta=(MultiLine="true"))
	FName TargetSpawnTag; // 목적지 레벨의 PlayerStart/TargetPoint 매칭용

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level Streaming", meta=(MultiLine="true"))
	FText DisplayNameForLoadingScreen;

	// 이 레벨로 진입할 때 스폰/Possess할 Pawn. 비워두면 Pawn 교체를 하지 않음
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level Streaming")
	TSubclassOf<APawn> PawnClassOverride;
};
