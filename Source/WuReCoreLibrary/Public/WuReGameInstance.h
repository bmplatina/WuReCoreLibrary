// © Platina Production 2017~2024

#pragma once

#include "WuReCoreLibrary.h"
#include "Engine/GameInstance.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "WuReGameInstance.generated.h"

/**
 * 
 */

USTRUCT(Atomic, BlueprintType)
struct FSettings
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Common Settings")
	bool IsInitLoad;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Common Settings")
	bool IsKorean;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Common Settings")
	int32 ClearedChapter;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Common Settings")
	float AmbientSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Common Settings")
	float SFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Common Settings")
	float Music;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Common Settings")
	int32 ScreenMode;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Common Settings")
	int32 ScreenResX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Common Settings")
	int32 ScreenResY;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Common Settings")
	float FrameLimit;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Common Settings")
	float CamFOV;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Common Settings")
	bool CamShake;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Common Settings")
	bool VSync;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Common Settings")
	int32 TexQuality;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Common Settings")
	int32 ShadowQuality;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Common Settings")
	int32 ReflectionQuality;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Common Settings")
	float MouseSensitivity;

	FSettings()
	{
		IsInitLoad = true;
		IsKorean = true;
		ClearedChapter = 0;
		
		AmbientSound = 1.f;
		SFX = 1.f;
		Music = 1.f;
		
		ScreenMode = 0;
		ScreenResX = 1920;
		ScreenResY = 1080;
		FrameLimit = 60;
		CamFOV = 90.f;
		CamShake = true;
		VSync = false;
		TexQuality = 0;
		ShadowQuality = 0;
		ReflectionQuality = 0;
		MouseSensitivity = 1.f;
	}
};

USTRUCT(Atomic, BlueprintType)
struct FDLSSUserSettings
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Common Settings")
    float RenderScale;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Common Settings")
	bool RayTracing;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Common Settings")
	int32 AntiAliasing;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Common Settings")
	int32 ModeDLSS;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Common Settings")
	int32 ModeNIS;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Common Settings")
	int32 ModeFG;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Common Settings")
	int32 ModeReflex;
	

	FDLSSUserSettings()
	{
		RenderScale = 1.f;
		RayTracing = true;
		AntiAliasing = 0;
		ModeDLSS = 0;
		ModeNIS = 0;
		ModeFG = 0;
		ModeReflex = 0;
	}
};

class UDataTable;

UCLASS(DisplayName="WU:RE Game Instance")
class WURECORELIBRARY_API UWuReGameInstance : public UGameInstance {
	GENERATED_BODY()

public:
	// UParadiseGameInstance();
	UPROPERTY(EditAnywhere, Category="Steamworks|Core", DisplayName="AppID")
	FString SteamAppId = "480";	// const -> constexpr
	
	// CSteamID MyID;
	UWuReGameInstance();

	// Language Settings
	UFUNCTION(BlueprintSetter, Category="Localization")
	void SetIsKorean(const bool bNewKorean, const bool bAutoUpdateWidget);
	
	UFUNCTION(BlueprintGetter, Category="Localization")
	bool GetIsKorean() const
	{
		return CommonSettings.IsKorean;
		// return bIsKorean;
	}

	UPROPERTY(EditAnywhere, Category="Localization")
	TObjectPtr<UDataTable> GlobalUILocalizationTable;
	UPROPERTY(EditAnywhere, Category="Localization")
	TObjectPtr<UDataTable> GlobalDialogueLocalizationTable;

	// Common Settings
	UFUNCTION(BlueprintGetter, Category="Core|Settings IO")
	FSettings& GetCommonSettings()
	{
		return CommonSettings;
	}

	UFUNCTION(BlueprintGetter, Category="Core")
	static bool GetIsWindows()
	{
		return UGameplayStatics::GetPlatformName().Equals(TEXT("Windows"));
	}

	UFUNCTION(BlueprintCallable, Category="Core|Settings IO")
	bool GetSettingsFromJson();
	UFUNCTION(BlueprintCallable, Category="Core|Settings IO")
	bool GetDLSSSettingsFromJson();
	UFUNCTION(BlueprintCallable, Category="Core|Settings IO")
	void SaveSettingsToJson();
	UFUNCTION(BlueprintCallable, Category="Core|Settings IO")
	void SaveDLSSSettingsToJson();

protected:
	virtual void Init() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Core|Settings IO")
	FSettings CommonSettings;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Core|Settings IO")
	FDLSSUserSettings DLSSSettings;

	UFUNCTION(BlueprintCallable, Category="Paradise Control")
	void SetMousePositionByGamepad(const float ValX, const float ValY, float Sensitivity);
private:
	UPROPERTY(EditAnywhere, meta=(allowPrivateAccess=true), Category="Volume Control", DisplayName="Sound Class Mix")
	TObjectPtr<USoundMix> SCMIX;
	UPROPERTY(EditAnywhere, meta=(allowPrivateAccess=true), Category="Volume Control")
	TObjectPtr<USoundClass> SoundAmbient;
	UPROPERTY(EditAnywhere, meta=(allowPrivateAccess=true), Category="Volume Control")
	TObjectPtr<USoundClass> SoundSFX;
	UPROPERTY(EditAnywhere, meta=(allowPrivateAccess=true), Category="Volume Control")
	TObjectPtr<USoundClass> SoundMusic;
};
