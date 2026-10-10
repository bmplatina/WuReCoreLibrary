// © Platina Production 2017~2024

#pragma once

#include "WuReCoreLibrary.h"
#include "GameFramework/GameModeBase.h"
#include "WuReGameModeBase.generated.h"

class UAsyncLevelStreamingSubsystem;

/**
 * 레벨 목록 테이블(UAsyncLevelStreamingSubsystem)의 row 설정으로 기본 Pawn/시작 위치를 정하는 GameMode.
 * 엔진 기본 스폰 흐름(Login -> RestartPlayer)은 월드 BeginPlay보다 먼저 돌기 때문에,
 * 타깃 레벨에 바로 PIE해도 레벨 액터들의 BeginPlay 시점에 이미 Pawn이 Possess돼 있음.
 */
UCLASS(DisplayName="WU:RE Game Mode Base")
class WURECORELIBRARY_API AWuReGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	// 현재 row의 PawnClassOverride. 없으면 DefaultPawnClass
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	// 현재 row의 TargetSpawnTag가 붙은 액터. 없으면 기본 PlayerStart 선택
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

private:
	UAsyncLevelStreamingSubsystem* GetLevelStreamingSubsystem() const;
};
