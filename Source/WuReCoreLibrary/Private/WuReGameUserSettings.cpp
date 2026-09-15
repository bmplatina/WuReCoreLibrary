// © Platina Production 2017~2024


#include "WuReGameUserSettings.h"

void UWuReGameUserSettings::ResetToDefaultValue()
{
/**
 * DLSS 설정 값
 */
	DLSSRenderScale = 1.f;
	bDLSSRayTracing = true;
	DLSSAntiAliasing = 0;
	DLSSModeDLSS = 0;
	DLSSModeNIS = 0;
	DLSSModeFG = 0;
	DLSSModeReflex = 0;
	
/**
 * 일반 설정 값
 */
	bIsInitLoad = true;
	CurrentLanguage = ESupportedLanguage::EN_GB;
	// ClearedChapter = 0;
	VolumeAmbient = 1.f;
	VolumeSoundFX = 1.f;
	VolumeMusic = 1.f;
	CamFOV = 90.f;
	bCamShakeEnabled = true;
	MouseSensitivity = 1.f;
}

UWuReGameUserSettings::UWuReGameUserSettings()
{
	ResetToDefaultValue();
}

void UWuReGameUserSettings::ApplySettings(bool bPromptToSave)
{
	Super::ApplySettings(bPromptToSave);
	OnSettingsApplied.Broadcast();
}

void UWuReGameUserSettings::SetToDefaults()
{
	Super::SetToDefaults();
	ResetToDefaultValue();
}

void UWuReGameUserSettings::SetCameraFOV(const float NewFOV)
{
	CamFOV = NewFOV;
	OnPlayerSettingsChanged.Broadcast(bCamShakeEnabled, NewFOV, MouseSensitivity);
}

void UWuReGameUserSettings::SetCameraShakeEnabled(const bool bEnabled)
{
	bCamShakeEnabled = bEnabled;
	OnPlayerSettingsChanged.Broadcast(bEnabled, CamFOV, MouseSensitivity);
}

void UWuReGameUserSettings::SetMouseSensitivity(const float NewSensitivity)
{
	MouseSensitivity = NewSensitivity;
	OnPlayerSettingsChanged.Broadcast(bCamShakeEnabled, CamFOV, NewSensitivity);
}
