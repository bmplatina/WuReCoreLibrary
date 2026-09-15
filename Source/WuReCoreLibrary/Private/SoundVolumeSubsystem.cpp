// © Platina Production 2017~2024


#include "SoundVolumeSubsystem.h"

#include "WuReGameUserSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "Engine/World.h"

void USoundVolumeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UserSettings = Cast<UWuReGameUserSettings>(UGameUserSettings::GetGameUserSettings());
	UGameplayStatics::SetBaseSoundMix(GetWorld(), BaseSoundMix);

	if (IsValid(UserSettings))
	{
		if (UserSettings->bIsInitLoad)
		{
			UserSettings->VolumeAmbient = SoundAmbient->Properties.Volume;
			UserSettings->VolumeSoundFX = SoundSFX->Properties.Volume;
			UserSettings->VolumeMusic = SoundMusic->Properties.Volume;
			UserSettings->bIsInitLoad = false;
			UserSettings->SaveConfig();
		}
		else
		{
			UGameplayStatics::SetSoundMixClassOverride(this, BaseSoundMix, SoundAmbient,
			                                           UserSettings->VolumeAmbient);
			UGameplayStatics::SetSoundMixClassOverride(this, BaseSoundMix, SoundSFX,
			                                           UserSettings->VolumeSoundFX);
			UGameplayStatics::SetSoundMixClassOverride(this, BaseSoundMix, SoundMusic,
			                                           UserSettings->VolumeMusic);
		}
	}
}

void USoundVolumeSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool USoundVolumeSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!IsValid(BaseSoundMix) || !IsValid(SoundAmbient) || !IsValid(SoundSFX) || !IsValid(SoundMusic))
		return false;

	return Super::ShouldCreateSubsystem(Outer);
}

void USoundVolumeSubsystem::SetAmbientVolume(const float Volume, const bool bApplyToConfig)
{
	UGameplayStatics::SetSoundMixClassOverride(this, BaseSoundMix, SoundAmbient, Volume);
	UGameplayStatics::PushSoundMixModifier(this, BaseSoundMix);
	UserSettings->VolumeAmbient = Volume;
	if (bApplyToConfig) ApplyVolumeToConfig();
}

void USoundVolumeSubsystem::SetSFXVolume(const float Volume, const bool bApplyToConfig)
{
	UGameplayStatics::SetSoundMixClassOverride(this, BaseSoundMix, SoundSFX, Volume);
	UGameplayStatics::PushSoundMixModifier(this, BaseSoundMix);
	UserSettings->VolumeSoundFX = Volume;
	if (bApplyToConfig) ApplyVolumeToConfig();
}

void USoundVolumeSubsystem::SetMusicVolume(const float Volume, const bool bApplyToConfig)
{
	UGameplayStatics::SetSoundMixClassOverride(this, BaseSoundMix, SoundMusic, Volume);
	UGameplayStatics::PushSoundMixModifier(this, BaseSoundMix);
	UserSettings->VolumeMusic = Volume;
	if (bApplyToConfig) ApplyVolumeToConfig();
}

float USoundVolumeSubsystem::GetAmbientVolume()
{
	return UserSettings->VolumeAmbient;
}

float USoundVolumeSubsystem::GetSFXVolume()
{
	return UserSettings->VolumeSoundFX;
}

float USoundVolumeSubsystem::GetMusicVolume()
{
	return UserSettings->VolumeMusic;
}

void USoundVolumeSubsystem::ApplyVolumeToConfig()
{
	UserSettings->SaveConfig();
}
