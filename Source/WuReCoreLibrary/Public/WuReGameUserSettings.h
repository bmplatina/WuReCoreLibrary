// © Platina Production 2017~2024

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "WuReTypes.h"
#include "WuReGameUserSettings.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, DisplayName="WU:RE Game User Settings")
class WURECORELIBRARY_API UWuReGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

private:
	void ResetToDefaultValue();

public:
	UWuReGameUserSettings();
	virtual void ApplySettings(bool bPromptToSave = true) override;
	virtual void SetToDefaults() override;

	DECLARE_MULTICAST_DELEGATE(FOnSettingsApplied);
	FOnSettingsApplied OnSettingsApplied;

	/**
	 * DLSS 설정 값
	 */
	UPROPERTY(config, BlueprintReadWrite, Category="Settings|DLSS")
	float DLSSRenderScale;
	UPROPERTY(config, BlueprintReadWrite, Category="Settings|DLSS")
	bool bDLSSRayTracing;
	UPROPERTY(config, BlueprintReadWrite, Category="Settings|DLSS")
	EBuiltInAAMode DLSSAntiAliasing;
	UPROPERTY(config, BlueprintReadWrite, Category="Settings|DLSS")
	UWuReDLSSMode DLSSModeDLSS;
	UPROPERTY(config, BlueprintReadWrite, Category="Settings|DLSS")
	UWuReNISMode DLSSModeNIS;
	UPROPERTY(config, BlueprintReadWrite, Category="Settings|DLSS")
	EWuReStreamlineDLSSGMode DLSSModeFG;
	UPROPERTY(config, BlueprintReadWrite, Category="Settings|DLSS")
	EWuReStreamlineReflexMode DLSSModeReflex;
	UPROPERTY(config, BlueprintReadWrite, Category="Settings|DLSS")
	EUpscaleMode DLSSModeUpscaling = EUpscaleMode::BuiltIn;

	/**
	 * 일반 설정 값
	 */
	UPROPERTY(config, BlueprintReadOnly, Category="Settings")
	bool bIsInitLoad;
	UPROPERTY(config, BlueprintReadOnly, Category="Settings|Localization")
	ESupportedLanguage CurrentLanguage;
	// UPROPERTY(config)
	// int32 ClearedChapter;

	UPROPERTY(config)
	float VolumeAmbient;
	UPROPERTY(config)
	float VolumeSoundFX;
	UPROPERTY(config)
	float VolumeMusic;


private:
	UPROPERTY(config, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Settings|Camera")
	float CamFOV;
	UPROPERTY(config, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Settings|Camera")
	bool bCamShakeEnabled;
	UPROPERTY(config, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Settings|Input")
	float MouseSensitivity;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPlayerSettingsChanged, bool, bCameraShakeEnabled, float,
	                                             CameraFieldOfiew, float, MouseSensitivity);

	UPROPERTY(BlueprintAssignable, Category = "Settings|Delegates")
	FOnPlayerSettingsChanged OnPlayerSettingsChanged;
	
	UFUNCTION(BlueprintCallable, Category="Settings|Camera")
	void SetCameraFOV(const float NewFOV);
	UFUNCTION(BlueprintCallable, Category="Settings|Camera")
	void SetCameraShakeEnabled(const bool bEnabled);
	UFUNCTION(BlueprintCallable, Category="Settings|Input")
	void SetMouseSensitivity(const float NewSensitivity);
};
