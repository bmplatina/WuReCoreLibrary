// © Platina Production 2017~2024

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "WuReSubtitleWorldSubsystem.generated.h"

class ULocalizationSubsystem;
struct FWuReLocalizationStruct;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSubtitleStateChanged, const FName, SubtitleTextRowName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPendingDialogsEmpty);

USTRUCT(Atomic)
struct FSubtitleInfo
{
	GENERATED_BODY()

public:
	FSubtitleInfo(FName InRowName, AActor* InActorToAttachSound)
	{
		RowName = InRowName;
		ActorToAttach = InActorToAttachSound;
	}
	
	FSubtitleInfo()
	{
		RowName = NAME_None;
		ActorToAttach = nullptr;
	}
	
	FName RowName;
	
	UPROPERTY()
	TObjectPtr<AActor> ActorToAttach;
};

/**
 * 
 */
UCLASS(Config=Game, meta=(DisplayName="WU:RE Subtitle World Subsystem"))
class WURECORELIBRARY_API UWuReSubtitleWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category="Subtitle|Events", DisplayName="Enqueue Subtitle")
	void RegisterSubtitle(FName InRowNameForDialog, AActor* ActorToAttachAudio);
	
	UFUNCTION(BlueprintPure, Category="Subtitle|Events")
	bool HasPendingSubtitle() const
	{
		return PendingDialogs.Num() > 0;
	}
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	UPROPERTY(BlueprintAssignable, Category="Subtitle|Event")
	FOnPendingDialogsEmpty OnPendingDialogsEmpty;
	
	UPROPERTY(BlueprintAssignable, Category="Subtitle|Event")
	FOnSubtitleStateChanged OnSubtitleStateChanged;
	
	UPROPERTY(EditDefaultsOnly, Category="Subtitle", Config)
	float SubtitleWaitingTime = 0.1f;
	
protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	
private:
	UPROPERTY()
	TArray<FSubtitleInfo> PendingDialogs;
	
	UPROPERTY()
	TObjectPtr<ULocalizationSubsystem> LocalizationSubsystem;
	
	FTimerHandle SubtitleTimerHandle;
	
	void ShowSubtitle();
	void HideSubtitle();
	
	bool bIsSubtitlePlaying = false;
};
