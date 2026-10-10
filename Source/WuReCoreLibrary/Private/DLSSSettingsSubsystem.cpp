// © Platina Production 2017~2024


#include "DLSSSettingsSubsystem.h"
#include "HAL/IConsoleManager.h"
#include "WuReGameUserSettings.h"
#include "Kismet/KismetSystemLibrary.h"
#include "RenderUtils.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Engine/World.h"

// DLSS for Windows Build
#if PLATFORM_WINDOWS
#include "StreamlineLibraryDLSSG.h"
#include "StreamlineLibraryReflex.h"
#endif

void UDLSSSettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UserSettings = Cast<UWuReGameUserSettings>(UGameUserSettings::GetGameUserSettings());

#if PLATFORM_WINDOWS
	// 지원하는 DLSS 버전
	// if (!UDLSSLibrary::IsDLSSSupported())

	// 기본 SR/DLAA는 지원됨
	DLSSCapabilities.bSuperResolution = UDLSSLibrary::IsDLSSSupported();
	DLSSCapabilities.bRayReconstruction = UDLSSLibrary::IsDLSSRRSupported();
	DLSSCapabilities.bFrameGeneration = UStreamlineLibraryDLSSG::IsDLSSGSupported();
	DLSSCapabilities.bMultiFrameGeneration = UStreamlineLibraryDLSSG::IsDLSSGModeSupported(EStreamlineDLSSGMode::On2X);
	DLSSCapabilities.bReflex = UStreamlineLibraryReflex::IsReflexSupported();
#endif

	if (UserSettings)
	{
		if (UserSettings->bIsInitLoad)
		{
#if PLATFORM_WINDOWS
			// DLSS 기본 설정 값 가져와서 설정 저장소에 저장
			UserSettings->bDLSSRayTracing
				= IConsoleManager::Get().FindConsoleVariable(TEXT("r.RayTracing.ForceAllRayTracingEffects"))->GetInt()
				== -1
				&& IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.HardwareRayTracing"))->GetInt() == 1;
			UserSettings->DLSSAntiAliasing = static_cast<EBuiltInAAMode>(IConsoleManager::Get().FindConsoleVariable(TEXT("r.AntiAliasingMethod"))->
			                                                        GetInt());
			UserSettings->DLSSModeDLSS = static_cast<UWuReDLSSMode>(UDLSSLibrary::GetDefaultDLSSMode());
			UserSettings->DLSSModeNIS = static_cast<UWuReNISMode>(UNISLibrary::GetDefaultNISMode());
			UserSettings->DLSSModeFG = static_cast<EWuReStreamlineDLSSGMode>(
				UStreamlineLibraryDLSSG::GetDefaultDLSSGMode());
			UserSettings->DLSSModeReflex = static_cast<EWuReStreamlineReflexMode>(
				UStreamlineLibraryReflex::GetDefaultReflexMode());
#endif
			UserSettings->bIsInitLoad = false;
			UserSettings->SaveConfig();
		}
		else ApplySettingsFromUserSettings(); // 설정 저장소에서 DLSS 설정 값 가져와서 엔진에 적용
	}
}

void UDLSSSettingsSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool UDLSSSettingsSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
#if PLATFORM_WINDOWS
	// TArray<UClass*> ChildClasses;
	// GetDerivedClasses(GetClass(), ChildClasses, false);
	// return ChildClasses.Num() == 0 ? Super::ShouldCreateSubsystem(Outer) : false;
	return Super::ShouldCreateSubsystem(Outer);
#else
	return false; // 맥/리눅스에선 아예 인스턴스 자체를 안 만듦
#endif
}

FString UDLSSSettingsSubsystem::SetUpscaleDLSS(UWuReDLSSMode DLSSModeLocal)
{
#if PLATFORM_WINDOWS
	/*
	 * Set DLSS
	 */
	if (DLSSModeLocal == UWuReDLSSMode::Default)
	{
		DLSSModeLocal = static_cast<UWuReDLSSMode>(UDLSSLibrary::GetDefaultDLSSMode());
	}
	SetDLSSMode(DLSSModeLocal);
	UNISLibrary::SetNISSharpness(0.f);
	SetAntiAliasingCVar(EBuiltInAAMode::TSR);

	UpdateDLSSDevStats();

	return UEnum::GetValueAsString(static_cast<UDLSSMode>(DLSSModeLocal));
#else
	return TEXT("");
#endif
}

FString UDLSSSettingsSubsystem::SetUpscaleNIS(UWuReNISMode NISModeLocal)
{
#if PLATFORM_WINDOWS
	/*
	 * Set NIS
	 */

	if (NISModeLocal == UWuReNISMode::Default)
	{
		UNISLibrary::SetNISMode(UNISLibrary::GetDefaultNISMode());
	}
	else
	{
		UNISLibrary::SetNISMode(static_cast<UNISMode>(NISModeLocal));
	}
	UNISLibrary::SetNISSharpness(0.f);
	SetAntiAliasingCVar(EBuiltInAAMode::TSR);

	UpdateDLSSDevStats();

	return UEnum::GetValueAsString(static_cast<UNISMode>(NISModeLocal));
#else
	return TEXT("");
#endif
}

FString UDLSSSettingsSubsystem::SetUpscaleBuiltIn()
{
#if PLATFORM_WINDOWS
	/*
	 * Built-in upscaler
	 */
	SetDLSSMode(UWuReDLSSMode::Off);
	UNISLibrary::SetNISMode(UNISMode::Off);
	UNISLibrary::SetNISSharpness(0.f);
	UKismetSystemLibrary::ExecuteConsoleCommand(this, TEXT("r.ScreenPercentage 100"));
	SetAntiAliasingCVar(EBuiltInAAMode::TAA);
	return UEnum::GetValueAsString(EBuiltInAAMode::TAA);
#else
	return TEXT("");
#endif
}

void UDLSSSettingsSubsystem::ApplyNewSettings(float SelectedRenderScale, bool bIsRayTracingChecked,
                                              EBuiltInAAMode SelectedAAMode,
                                              UWuReDLSSMode SelectedDLSSMode, UWuReNISMode SelectedNISMode,
                                              EWuReStreamlineDLSSGMode SelectedFGMode,
                                              EWuReStreamlineReflexMode SelectedReflexMode,
                                              EUpscaleMode SelectedUpscalingMode)
{
	if (!IsValid(UserSettings)) return;
#if PLATFORM_WINDOWS
	UserSettings->DLSSRenderScale = SelectedRenderScale;
	UserSettings->bDLSSRayTracing = bIsRayTracingChecked;
	UserSettings->DLSSAntiAliasing = SelectedAAMode;
	UserSettings->DLSSModeDLSS = SelectedDLSSMode;
	UserSettings->DLSSModeNIS = SelectedNISMode;
	UserSettings->DLSSModeFG = SelectedFGMode;
	UserSettings->DLSSModeReflex = SelectedReflexMode;
	UserSettings->DLSSModeUpscaling = SelectedUpscalingMode;
	UserSettings->SaveConfig();

	/*
	 * Apply Ray Tracing
	 */
	FString RayTracing = FString::Printf(
		TEXT("r.RayTracing.ForceAllRayTracingEffects %d"), bIsRayTracingChecked ? -1 : 0);
	UKismetSystemLibrary::ExecuteConsoleCommand(this, RayTracing);
	RayTracing = FString::Printf(TEXT("r.Lumen.HardwareRayTracing  %d"), bIsRayTracingChecked ? 1 : 0);
	UKismetSystemLibrary::ExecuteConsoleCommand(this, RayTracing);

	/*
	 * Apply Render Scale
	 */
	UKismetSystemLibrary::ExecuteConsoleCommand(
		this, FString::Printf(TEXT("r.ScreenPercentage %f"), SelectedRenderScale));

	/*
	 * Apply Reflex
	 */
	EStreamlineReflexMode StreamlineReflexMode = static_cast<EStreamlineReflexMode>(SelectedReflexMode);
	UStreamlineLibraryReflex::SetReflexMode(StreamlineReflexMode);

	/*
	 * Set Upscale Mode
	 */
	switch (SelectedUpscalingMode)
	{
	case EUpscaleMode::BuiltIn:
		SetUpscaleBuiltIn();
		break;
	case EUpscaleMode::DLSS:
		SetUpscaleDLSS(UserSettings->DLSSModeDLSS);
		break;
	case EUpscaleMode::NIS:
		SetUpscaleNIS(UserSettings->DLSSModeNIS);
		break;
	default:
		break;
	}

	/*
	 * AA Mode
	 */
	EBuiltInAAMode BuiltInAAMode = SelectedAAMode;
	SetAntiAliasingCVar(BuiltInAAMode);
	EnableDLAA(false);

	/*
	 * DLSS Mode
	 */
	SetDLSSMode(SelectedDLSSMode);
	UpdateDLSSDevStats();

	/*
	 * NIS Mode
	 */
	UNISLibrary::SetNISMode(static_cast<UNISMode>(SelectedNISMode));

	/*
	 * DLSS-FG
	 */
	UStreamlineLibraryDLSSG::SetDLSSGMode(static_cast<EStreamlineDLSSGMode>(SelectedFGMode));
#endif
}

void UDLSSSettingsSubsystem::ApplySettingsFromUserSettings()
{
	if (!IsValid(UserSettings) || !IsValid(GetWorld())) return;
#if PLATFORM_WINDOWS
	// Ray Tracing
	if (UserSettings->bDLSSRayTracing)
	{
		GetWorld()->Exec(GetWorld(), TEXT("r.RayTracing.ForceAllRayTracingEffects -1"), *GLog);
		GetWorld()->Exec(GetWorld(), TEXT("r.Lumen.HardwareRaytracing 1"), *GLog);
	}
	else
	{
		GetWorld()->Exec(GetWorld(), TEXT("r.RayTracing.ForceAllRayTracingEffects 0"), *GLog);
		GetWorld()->Exec(GetWorld(), TEXT("r.Lumen.HardwareRaytracing 0"), *GLog);
	}
	GetWorld()->Exec(GetWorld(), *(TEXT("r.AntiAliasingMethod ") + FString::FromInt(static_cast<int>(UserSettings->DLSSAntiAliasing))),
	                 *GLog);

	UDLSSLibrary::SetDLSSMode(GetWorld(), static_cast<UDLSSMode>(UserSettings->DLSSModeDLSS));
	UNISLibrary::SetNISMode(static_cast<UNISMode>(UserSettings->DLSSModeNIS));
	UStreamlineLibraryDLSSG::SetDLSSGMode(static_cast<EStreamlineDLSSGMode>(UserSettings->DLSSModeFG));
	UStreamlineLibraryReflex::SetReflexMode(static_cast<EStreamlineReflexMode>(UserSettings->DLSSModeReflex));
#endif
}

FDLSSCapabilities UDLSSSettingsSubsystem::GetDLSSCapabilities() const
{
	return DLSSCapabilities;
}

TArray<UWuReDLSSMode> UDLSSSettingsSubsystem::GetSupportedDLSSModes() const
{
	TArray<UWuReDLSSMode> ReturnArray;
#if PLATFORM_WINDOWS
	const TArray<UDLSSMode> DlssModeArray = UDLSSLibrary::GetSupportedDLSSModes();
	for (UDLSSMode DlssModeLocal : DlssModeArray)
	{
		ReturnArray.Add(static_cast<UWuReDLSSMode>(DlssModeLocal));
	}
#endif
	return ReturnArray;
}

TArray<EWuReStreamlineDLSSGMode> UDLSSSettingsSubsystem::GetSupportedDLSSFGModes() const
{
	TArray<EWuReStreamlineDLSSGMode> ReturnArray;
#if PLATFORM_WINDOWS
	const TArray<EStreamlineDLSSGMode> DlssFgModeArray = UStreamlineLibraryDLSSG::GetSupportedDLSSGModes();
	for (EStreamlineDLSSGMode DlssModeLocal : DlssFgModeArray)
	{
		ReturnArray.Add(static_cast<EWuReStreamlineDLSSGMode>(DlssModeLocal));
	}
#endif
	return ReturnArray;
}

void UDLSSSettingsSubsystem::SetAntiAliasingCVar(const EBuiltInAAMode AAMode)
{
	UKismetSystemLibrary::ExecuteConsoleCommand(
		this, FString::Printf(TEXT("r.AntiAliasingMethod %d"), static_cast<int>(AAMode)));
}

void UDLSSSettingsSubsystem::EnableDLAA(const bool bDLAAEnabled)
{
#if PLATFORM_WINDOWS
	SetDLSSNoAuto(UWuReDLSSMode::Off, bDLAAEnabled, false);
#endif
}


void UDLSSSettingsSubsystem::SetDLSSMode(const UWuReDLSSMode DLSSModeLocal)
{
	if (!IsValid(UserSettings)) return;
#if PLATFORM_WINDOWS
	if (static_cast<UDLSSMode>(DLSSModeLocal) == UDLSSMode::Auto)
	{
		FVector2D ScreenResolution = UWidgetLayoutLibrary::GetViewportSize(this);
		SetDLSSAuto(ScreenResolution, true, false, true);
	}
	else
	{
		SetDLSSNoAuto(static_cast<UWuReDLSSMode>(DLSSModeLocal), false, true);
	}
#endif
}

void UDLSSSettingsSubsystem::SetDLSSNoAuto(const UWuReDLSSMode NewDLSSMode, const bool bDLAAEnabled,
                                           const bool bRestoreFullResWhenDisabled)
{
#if PLATFORM_WINDOWS
	UDLSSMode DLSSModeLocal = static_cast<UDLSSMode>(NewDLSSMode);

	UDLSSLibrary::EnableDLSS(!(DLSSModeLocal == UDLSSMode::Off) || bDLAAEnabled);
	if (DLSSModeLocal != UDLSSMode::Off || bDLAAEnabled || bRestoreFullResWhenDisabled)
	{
		bool bDummy;
		float dummy = 0.f;
		float OptimalScreenPercent = 0.f;

		UDLSSLibrary::GetDLSSModeInformation(DLSSModeLocal, FVector2D::Zero(), bDummy, OptimalScreenPercent, bDummy,
		                                     dummy, dummy, dummy);

		float ScreenPercentage = (DLSSModeLocal == UDLSSMode::Off) || bDLAAEnabled
			                         ? 100.f
			                         : OptimalScreenPercent;

		UKismetSystemLibrary::ExecuteConsoleCommand(
			this, FString::Printf(TEXT("r.ScreenPercentage %f"), ScreenPercentage));
	}
#endif
}


void UDLSSSettingsSubsystem::UpdateDLSSDevStats()
{
}

void UDLSSSettingsSubsystem::SetDLSSAuto(const FVector2D ScreenResolution, const bool bDLSSAuto,
                                         const bool bDLAAEnabled,
                                         const bool bRestoreFullResWhenDisabled)
{
#if PLATFORM_WINDOWS
	bool bDummy;
	float DummyFloat;
	float OptimalScreenPercentage = 0.f;
	UDLSSLibrary::GetDLSSModeInformation(UDLSSMode::Auto, ScreenResolution, bDummy, OptimalScreenPercentage, bDummy,
	                                     DummyFloat, DummyFloat, DummyFloat);
	UDLSSLibrary::EnableDLSS((OptimalScreenPercentage > 0.f && bDLSSAuto) || bDLAAEnabled);
	float DLAAOverrideScreenPercentage = !(OptimalScreenPercentage > 0.f && bDLSSAuto) || bDLAAEnabled
		                                     ? 100.f
		                                     : OptimalScreenPercentage;

	if ((OptimalScreenPercentage > 0.f && bDLSSAuto) || bDLAAEnabled || bRestoreFullResWhenDisabled)
	{
		UKismetSystemLibrary::ExecuteConsoleCommand(
			this, FString::Printf(TEXT("r.ScreenPercentage %f"), DLAAOverrideScreenPercentage));
	}

#endif
}
