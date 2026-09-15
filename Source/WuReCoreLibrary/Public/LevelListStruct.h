// © Platina Production 2017~2024

#pragma once

#include "Engine/DataTable.h"
#include "LevelListStruct.generated.h"

USTRUCT(BlueprintType, DisplayName="WU:RE Level List Struct")
struct FLevelListStruct : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level Streaming", meta=(MultiLine="true"))
	TSoftObjectPtr<UWorld> LevelAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level Streaming", meta=(MultiLine="true"))
	FName TargetSpawnTag; // 목적지 레벨의 PlayerStart/TargetPoint 매칭용

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level Streaming", meta=(MultiLine="true"))
	FText DisplayNameForLoadingScreen;
};
