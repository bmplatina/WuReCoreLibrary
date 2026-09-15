// © Platina Production 2017~2024


#include "DLSSSettingsSubsystem.h"
#include "HAL/IConsoleManager.h"
#include "WuReGameUserSettings.h"
#include "Kismet/KismetSystemLibrary.h"
#include "RenderUtils.h"
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
			UserSettings->DLSSAntiAliasing = IConsoleManager::Get().FindConsoleVariable(TEXT("r.AntiAliasingMethod"))->
			                                                        GetInt();
			UserSettings->DLSSModeDLSS = static_cast<int>(UDLSSLibrary::GetDefaultDLSSMode());
			UserSettings->DLSSModeNIS = static_cast<int>(UNISLibrary::GetDefaultNISMode());
			UserSettings->DLSSModeFG = static_cast<int>(UStreamlineLibraryDLSSG::GetDefaultDLSSGMode());
			UserSettings->DLSSModeReflex = static_cast<int>(UStreamlineLibraryReflex::GetDefaultReflexMode());
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
#if !PLATFORM_WINDOWS
	return false; // 맥/리눅스에선 아예 인스턴스 자체를 안 만듦
#else
	TArray<UClass*> ChildClasses;
	GetDerivedClasses(GetClass(), ChildClasses, false);	
	return ChildClasses.Num() == 0  ? Super::ShouldCreateSubsystem(Outer) : false;
#endif
}

void UDLSSSettingsSubsystem::SetUpscaleDLSS(const uint8 DLSSModeLocal)
{
#if PLATFORM_WINDOWS
	/*
	 * Set DLSS
	 */
	SetDLSSMode(DLSSModeLocal);
	UNISLibrary::SetNISSharpness(0.f);

	SetAntiAliasingCVar(EBuiltInAAMode::TSR);

	UpdateDLSSDevStats();
#endif
}

void UDLSSSettingsSubsystem::SetUpscaleNIS(const uint8 DLSSModeLocal)
{
#if PLATFORM_WINDOWS
	/*
	 * Set DLSS
	 */
	SetDLSSMode(DLSSModeLocal);
	UNISLibrary::SetNISSharpness(0.f);
	SetAntiAliasingCVar(EBuiltInAAMode::TSR);

	UpdateDLSSDevStats();
#endif
}

void UDLSSSettingsSubsystem::SetUpscaleBuiltIn()
{
#if PLATFORM_WINDOWS
	/*
	 * Built-in upscaler
	 */
	UNISLibrary::SetNISMode(UNISMode::Off);
	UNISLibrary::SetNISSharpness(0.f);
	UKismetSystemLibrary::ExecuteConsoleCommand(this, TEXT("r.ScreenPercentage 100"));
	SetAntiAliasingCVar(EBuiltInAAMode::TAA);
#endif
}

void UDLSSSettingsSubsystem::ApplyNewSettings(float SelectedRenderScale, bool bIsRayTracingChecked,
                                              int32 SelectedAAMode,
                                              UDLSSMode SelectedDLSSMode, UNISMode SelectedNISMode,
                                              int32 SelectedFGMode,
                                              int32 SelectedReflexMode, int32 SelectedUpscalingMode)
{
	if (!IsValid(UserSettings)) return;
#if PLATFORM_WINDOWS
	UserSettings->DLSSRenderScale = SelectedRenderScale;
	UserSettings->bDLSSRayTracing = bIsRayTracingChecked;
	UserSettings->DLSSAntiAliasing = SelectedAAMode;
	UserSettings->DLSSModeDLSS = static_cast<uint8>(SelectedDLSSMode);
	UserSettings->DLSSModeNIS = static_cast<uint8>(SelectedNISMode);
	UserSettings->DLSSModeFG = SelectedFGMode;
	UserSettings->DLSSModeReflex = SelectedReflexMode;
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
	switch (static_cast<EUpscaleMode>(SelectedUpscalingMode))
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
	EBuiltInAAMode BuiltInAAMode = SelectedAAMode
		                               ? EBuiltInAAMode::TAA
		                               : EBuiltInAAMode::Off;
	SetAntiAliasingCVar(BuiltInAAMode);
	EnableDLAA(false);

	/*
	 * DLSS Mode
	 */
	SetDLSSMode(static_cast<uint8>(SelectedDLSSMode));
	UpdateDLSSDevStats();

	/*
	 * NIS Mode
	 */
	UNISLibrary::SetNISMode(SelectedNISMode);

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
	GetWorld()->Exec(GetWorld(), *(TEXT("r.AntiAliasingMethod ") + FString::FromInt(UserSettings->DLSSAntiAliasing)),
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

void UDLSSSettingsSubsystem::SetAntiAliasingCVar(const EBuiltInAAMode AAMode)
{
	UKismetSystemLibrary::ExecuteConsoleCommand(
		this, FString::Printf(TEXT("r.AntiAliasingMethod %d"), static_cast<int>(AAMode)));
}

void UDLSSSettingsSubsystem::EnableDLAA(const bool bDLAAEnabled)
{
#if PLATFORM_WINDOWS
	SetDLSSNoAuto(UDLSSMode::Off, bDLAAEnabled, false);
#endif
}


void UDLSSSettingsSubsystem::SetDLSSMode(const uint8 DLSSModeLocal)
{
	if (!IsValid(UserSettings)) return;
#if PLATFORM_WINDOWS
	if (static_cast<UDLSSMode>(DLSSModeLocal) == UDLSSMode::Auto)
	{
		FIntPoint ScreenResolution = UserSettings->GetScreenResolution();
		FVector2D ScreenResolutionVector2D = FVector2D(ScreenResolution.X, ScreenResolution.Y);

		SetDLSSAuto(ScreenResolutionVector2D, true, false, true);
	}
	else
	{
		SetDLSSNoAuto(static_cast<UDLSSMode>(DLSSModeLocal), false, true);
	}
#endif
}

void UDLSSSettingsSubsystem::SetDLSSNoAuto(const UDLSSMode NewDLSSMode, const bool bDLAAEnabled,
                                           const bool bRestoreFullResWhenDisabled)
{
#if PLATFORM_WINDOWS
	UDLSSLibrary::EnableDLSS(!(NewDLSSMode == UDLSSMode::Off) || bDLAAEnabled);
	if (NewDLSSMode != UDLSSMode::Off || bDLAAEnabled || bRestoreFullResWhenDisabled)
	{
		bool bDummy;
		float dummy = 0.f;
		float OptimalScreenPercent = 0.f;

		UDLSSLibrary::GetDLSSModeInformation(NewDLSSMode, FVector2D::Zero(), bDummy, OptimalScreenPercent, bDummy,
		                                     dummy, dummy, dummy);

		float ScreenPercentage = (NewDLSSMode == UDLSSMode::Off) || bDLAAEnabled
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
