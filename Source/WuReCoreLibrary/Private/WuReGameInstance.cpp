// © Platina Production 2017~2024

#include "WuReGameInstance.h"
#include "JsonObjectConverter.h"
#include "WuReUserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "GameFramework/GameUserSettings.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/DataTable.h"

// DLSS for Windows Build
#if PLATFORM_WINDOWS
#include "DLSSLibrary.h"
#include "NISLibrary.h"
#include "StreamlineLibraryDLSSG.h"
#include "StreamlineLibraryReflex.h"
#endif

UWuReGameInstance::UWuReGameInstance()
{
}

void UWuReGameInstance::Init() {
	Super::Init();
	const TWeakObjectPtr<UGameUserSettings> UserSettings = UGameUserSettings::GetGameUserSettings();

	if(UserSettings.IsValid())
	{
		UGameplayStatics::SetBaseSoundMix(GetWorld(), SCMIX.Get());
	}
	if(GetSettingsFromJson())
	{
		CommonSettings.IsInitLoad = false;
		// if config.json is valid, FSettings->GameUserSettings
		if(IsValid(SCMIX.Get()) && IsValid(SoundAmbient.Get()) && IsValid(SoundSFX.Get()) && IsValid(SoundMusic.Get()) && IsValid(GetWorld()))
		{
			UGameplayStatics::SetSoundMixClassOverride(GetWorld(), SCMIX.Get(), SoundAmbient.Get(), CommonSettings.AmbientSound);
			UGameplayStatics::SetSoundMixClassOverride(GetWorld(), SCMIX.Get(), SoundSFX.Get(), CommonSettings.SFX);
			UGameplayStatics::SetSoundMixClassOverride(GetWorld(), SCMIX.Get(), SoundMusic.Get(), CommonSettings.Music);
		}
		UserSettings.Get()->SetFullscreenMode(EWindowMode::ConvertIntToWindowMode(CommonSettings.ScreenMode));
		UserSettings.Get()->SetScreenResolution(FIntPoint(CommonSettings.ScreenResX, CommonSettings.ScreenResY));
		UserSettings.Get()->SetFrameRateLimit(CommonSettings.FrameLimit);
		UserSettings.Get()->SetVSyncEnabled(CommonSettings.VSync);
		UserSettings.Get()->SetTextureQuality(CommonSettings.TexQuality);
		UserSettings.Get()->SetShadowQuality(CommonSettings.ShadowQuality);
		UserSettings.Get()->SetReflectionQuality(CommonSettings.ReflectionQuality);
		UserSettings.Get()->ApplySettings(true);
	}
	else
	{
		// if config.json is not valid, GameUserSettings (Default Settings)->FSettings
		if(IsValid(SCMIX.Get()) && IsValid(SoundAmbient.Get()) && IsValid(SoundSFX.Get()) && IsValid(SoundMusic.Get()) && IsValid(GetWorld()))
		{
			CommonSettings.AmbientSound = SoundAmbient.Get()->Properties.Volume;
			CommonSettings.SFX = SoundSFX.Get()->Properties.Volume;
			CommonSettings.Music = SoundMusic.Get()->Properties.Volume;
		}
		CommonSettings.ScreenMode = UserSettings.Get()->GetFullscreenMode();
		CommonSettings.ScreenResX = UserSettings.Get()->GetScreenResolution().X;
		CommonSettings.ScreenResY = UserSettings.Get()->GetScreenResolution().Y;
		CommonSettings.FrameLimit = UserSettings.Get()->GetFrameRateLimit();
		CommonSettings.VSync = UserSettings.Get()->IsVSyncEnabled();
		CommonSettings.TexQuality = UserSettings.Get()->GetTextureQuality();
		CommonSettings.ShadowQuality = UserSettings.Get()->GetShadowQuality();
		CommonSettings.ReflectionQuality = UserSettings.Get()->GetReflectionQuality();
	}
	
#if PLATFORM_WINDOWS
	if(GetDLSSSettingsFromJson())
	{
		// if dlss.json is valid, FDLSSUserSettings->UDLSSLibrary
		if(IsValid(GetWorld()))	// Ray Tracing
		{
			if(DLSSSettings.RayTracing)
			{
				GetWorld()->Exec(GetWorld(), TEXT("r.RayTracing.ForceAllRayTracingEffects -1"), *GLog);
				GetWorld()->Exec(GetWorld(), TEXT("r.Lumen.HardwareRaytracing 1"), *GLog);
			}
			else
			{
				GetWorld()->Exec(GetWorld(), TEXT("r.RayTracing.ForceAllRayTracingEffects 0"), *GLog);
				GetWorld()->Exec(GetWorld(), TEXT("r.Lumen.HardwareRaytracing 0"), *GLog);
			}
			GetWorld()->Exec(GetWorld(), *(TEXT("r.AntiAliasingMethod ") + FString::FromInt(DLSSSettings.AntiAliasing)), *GLog);
		}
		UDLSSLibrary::SetDLSSMode(GetWorld(), static_cast<UDLSSMode>(DLSSSettings.ModeDLSS));
		UNISLibrary::SetNISMode(static_cast<UNISMode>(DLSSSettings.ModeNIS));
		UStreamlineLibraryDLSSG::SetDLSSGMode(static_cast<UStreamlineDLSSGMode>(DLSSSettings.ModeFG));
		UStreamlineLibraryReflex::SetReflexMode(static_cast<UStreamlineReflexMode>(DLSSSettings.ModeReflex));
	}
	else
	{
		// if dlss.json is not valid, UDLSSLibrary->FDLSSUserSettings
		DLSSSettings.RayTracing
			= IConsoleManager::Get().FindConsoleVariable(TEXT("r.RayTracing.ForceAllRayTracingEffects"))->GetInt() == -1
			&& IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.HardwareRayTracing"))->GetInt() == 1;
		DLSSSettings.AntiAliasing = IConsoleManager::Get().FindConsoleVariable(TEXT("r.AntiAliasingMethod"))->GetInt();
		DLSSSettings.ModeDLSS = static_cast<int>(UDLSSLibrary::GetDefaultDLSSMode());
		DLSSSettings.ModeNIS = static_cast<int>(UNISLibrary::GetDefaultNISMode());
		DLSSSettings.ModeFG = static_cast<int>(UStreamlineLibraryDLSSG::GetDefaultDLSSGMode());
		DLSSSettings.ModeReflex = static_cast<int>(UStreamlineLibraryReflex::GetDefaultReflexMode());
	}
	
#endif
	GEngine->IsEditor()
		? FFileHelper::SaveStringToFile(SteamAppId, *(FPaths::ProjectDir() + TEXT("steam_appid.txt")))
		: FFileHelper::SaveStringToFile(SteamAppId, *(FPaths::RootDir() + TEXT("steam_appid.txt")));
	FFileHelper::SaveStringToFile(SteamAppId, *(FPaths::ProjectUserDir() + TEXT("steam_appid.txt")));
}


void UWuReGameInstance::SaveSettingsToJson()
{
	FString OutputJson;
	FJsonObjectConverter::UStructToJsonObjectString<FSettings>(CommonSettings, OutputJson);
	GEngine->IsEditor()
		? FFileHelper::SaveStringToFile(*OutputJson, *(FPaths::ProjectDir() + TEXT("config.json")))
		: FFileHelper::SaveStringToFile(*OutputJson, *(FPaths::RootDir() + TEXT("config.json")));
}

bool UWuReGameInstance::GetSettingsFromJson()
{
	FString InputJson;
	
	const bool IsFileLoadSucceeded = GEngine->IsEditor()
		? FFileHelper::LoadFileToString(InputJson, *(FPaths::ProjectDir() + TEXT("config.json")))
		: FFileHelper::LoadFileToString(InputJson, *(FPaths::RootDir() + TEXT("config.json")));
	const bool IsConvertSucceeded = FJsonObjectConverter::JsonObjectStringToUStruct<FSettings>(*InputJson, &CommonSettings);
	if(!IsFileLoadSucceeded && !IsConvertSucceeded)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("CommonSettings LOAD FAILED"));
		// Game User Settings To FSettings
		const TWeakObjectPtr<UGameUserSettings> UserSettings = UGameUserSettings::GetGameUserSettings();
		CommonSettings.ScreenMode = UserSettings.Get()->GetFullscreenMode();
		CommonSettings.ScreenResX = UserSettings.Get()->GetScreenResolution().X;
		CommonSettings.ScreenResY = UserSettings.Get()->GetScreenResolution().Y;
		CommonSettings.FrameLimit = UserSettings.Get()->GetFrameRateLimit();
		CommonSettings.VSync = UserSettings.Get()->IsVSyncEnabled();
		CommonSettings.TexQuality = UserSettings.Get()->GetTextureQuality();
		CommonSettings.ShadowQuality = UserSettings.Get()->GetShadowQuality();
		CommonSettings.ReflectionQuality = UserSettings.Get()->GetReflectionQuality();
		SaveSettingsToJson();
		return false;
	}
	return true;
}

void UWuReGameInstance::SaveDLSSSettingsToJson()
{
	FString OutputJson;
	FJsonObjectConverter::UStructToJsonObjectString<FDLSSUserSettings>(DLSSSettings, OutputJson);
	GEngine->IsEditor()
		? FFileHelper::SaveStringToFile(*OutputJson, *(FPaths::ProjectDir() + TEXT("dlss.json")))
		: FFileHelper::SaveStringToFile(*OutputJson, *(FPaths::RootDir() + TEXT("dlss.json")));
}

bool UWuReGameInstance::GetDLSSSettingsFromJson()
{
	FString InputJson;
	
	const bool IsFileLoadSucceeded = GEngine->IsEditor()
		? FFileHelper::LoadFileToString(InputJson, *(FPaths::ProjectDir() + TEXT("dlss.json")))
		: FFileHelper::LoadFileToString(InputJson, *(FPaths::RootDir() + TEXT("dlss.json")));
	const bool IsConvertSucceeded = FJsonObjectConverter::JsonObjectStringToUStruct<FDLSSUserSettings>(*InputJson, &DLSSSettings);
	if(!IsFileLoadSucceeded && !IsConvertSucceeded)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("DLSSSettings LOAD FAILED"));
		SaveDLSSSettingsToJson();
		return false;
	}
	return true;
}

void UWuReGameInstance::SetMousePositionByGamepad(const float ValX, const float ValY, float Sensitivity)
{
	if(ValX != 0.f || ValY != 0.f)
	{
		Sensitivity *= GetWorld()->GetDeltaSeconds(); // UGameplayStatics::GetWorldDeltaSeconds(GetWorld());
		const TWeakObjectPtr<AWuRePlayerController> PlayerController = Cast<AWuRePlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
		float PosX, PosY;
		FVector2D ScreenSize;
		GEngine->GameViewport->GetViewportSize(ScreenSize);
		PlayerController.Get()->GetMousePosition(PosX, PosY);
		PosX = FMath::Clamp(PosX + ValX * Sensitivity, 0, ScreenSize.X);
		PosY = FMath::Clamp(PosY + ValY * -1.f * Sensitivity, 0, ScreenSize.Y);
		PlayerController.Get()->SetMouseLocation(static_cast<int>(PosX), static_cast<int>(PosY));
	}
}

void UWuReGameInstance::SetIsKorean(const bool bNewKorean, const bool bAutoUpdateWidget)
{
	if(CommonSettings.IsKorean !=bNewKorean)
	{
		CommonSettings.IsKorean = bNewKorean;
		if(bAutoUpdateWidget)
		{
			TArray<UUserWidget*> ParadiseWidget;
			UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, ParadiseWidget, UWuReUserWidget::StaticClass());
			for(UUserWidget* Widget: ParadiseWidget)
			{
				UWuReUserWidget* PDWidget = Cast<UWuReUserWidget>(Widget);
				if(IsValid(PDWidget))
				{
					PDWidget->UpdateLocalizedLanguage();
				}
			}
		}
	}
}
