// © Platina Production 2017~2024

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WuReGameUserSettings.h"
#include "SoundVolumeSubsystem.generated.h"

class USoundMix;
class USoundClass;

/**
 *
 */
UCLASS(Abstract, Blueprintable, DisplayName="WU:RE Sound Volume Subsystem")
class WURECORELIBRARY_API USoundVolumeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	// GameInstance가 Init될 때 자동 호출
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// GameInstance가 Shutdown될 때 자동 호출
	virtual void Deinitialize() override;

	// 플랫폼 등의 조건으로 아예 생성 자체를 막고 싶을 때 (선택)
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	
/**
 * 사운드 큐 프로퍼티
 */
public:
	UFUNCTION(BlueprintCallable, Category="Volume Control")
	void SetAmbientVolume(const float Volume, const bool bApplyToConfig = false);
	UFUNCTION(BlueprintCallable, Category="Volume Control", DisplayName="Set Sound Effects Volume")
	void SetSFXVolume(const float Volume, const bool bApplyToConfig = false);
	UFUNCTION(BlueprintCallable, Category="Volume Control")
	void SetMusicVolume(const float Volume, const bool bApplyToConfig = false);
	
	UFUNCTION(BlueprintPure, Category="Volume Control")
	float GetAmbientVolume();
	UFUNCTION(BlueprintPure, Category="Volume Control", DisplayName="Get Sound Effects Volume")
	float GetSFXVolume();
	UFUNCTION(BlueprintPure, Category="Volume Control")
	float GetMusicVolume();
	
	UFUNCTION(BlueprintCallable, Category="Volume Control")
	void ApplyVolumeToConfig();
	
protected:
	UPROPERTY(EditAnywhere, meta=(allowPrivateAccess=true), Category="Volume Control", DisplayName="Sound Class Mix")
	TObjectPtr<USoundMix> BaseSoundMix;
	UPROPERTY(EditAnywhere, meta=(allowPrivateAccess=true), Category="Volume Control")
	TObjectPtr<USoundClass> SoundAmbient;
	UPROPERTY(EditAnywhere, meta=(allowPrivateAccess=true), Category="Volume Control")
	TObjectPtr<USoundClass> SoundSFX;
	UPROPERTY(EditAnywhere, meta=(allowPrivateAccess=true), Category="Volume Control")
	TObjectPtr<USoundClass> SoundMusic;
	
private:
	UPROPERTY()
	TObjectPtr<UWuReGameUserSettings> UserSettings;
};
