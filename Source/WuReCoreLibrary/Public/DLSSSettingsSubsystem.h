// © Platina Production 2017~2024

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WuReTypes.h"

#if PLATFORM_WINDOWS
#include "DLSSLibrary.h"
#include "NISLibrary.h"
#endif

#include "DLSSSettingsSubsystem.generated.h"

/**
 * 
 */

class UWuReGameUserSettings;

UCLASS(Abstract, Blueprintable, DisplayName="WU:RE DLSS Settings Subsystem")
class WURECORELIBRARY_API UDLSSSettingsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// GameInstance가 Init될 때 자동 호출
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// GameInstance가 Shutdown될 때 자동 호출
	virtual void Deinitialize() override;

	// 플랫폼 등의 조건으로 아예 생성 자체를 막고 싶을 때 (선택)
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

private:
	UPROPERTY()
	TObjectPtr<UWuReGameUserSettings> UserSettings;

	/**
	 * DLSS Methods
	 */
public:
	UFUNCTION(BlueprintCallable, Category="DLSS|Settings")
	void ApplyNewSettings(float SelectedRenderScale, bool bIsRayTracingChecked, EBuiltInAAMode SelectedAAMode,
	                      UWuReDLSSMode SelectedDLSSMode, UWuReNISMode SelectedNISMode, EWuReStreamlineDLSSGMode SelectedFGMode,
	                      EWuReStreamlineReflexMode SelectedReflexMode, EUpscaleMode SelectedUpscalingMode);

	void ApplySettingsFromUserSettings();

	UFUNCTION(BlueprintPure, Category="DLSS|Settings")
	FDLSSCapabilities GetDLSSCapabilities() const;

	UFUNCTION(BlueprintPure, Category="DLSS|Capabilities", DisplayName="Get Supported DLSS-SR Modes")
	TArray<UWuReDLSSMode> GetSupportedDLSSModes() const;

	UFUNCTION(BlueprintPure, Category="DLSS|Capabilities", DisplayName="Get Supported DLSS-FG Modes")
	TArray<EWuReStreamlineDLSSGMode> GetSupportedDLSSFGModes() const;

	UFUNCTION(BlueprintCallable, Category="DLSS|Macros")
	void EnableDLAA(const bool bDLAAEnabled);
	UFUNCTION(BlueprintCallable, Category="DLSS|Macros")
	void SetDLSSMode(const UWuReDLSSMode DLSSModeLocal);
	
	UFUNCTION(BlueprintCallable, Category="DLSS")
	FString SetUpscaleDLSS(UWuReDLSSMode DLSSModeLocal = UWuReDLSSMode::Default);
	UFUNCTION(BlueprintCallable, Category="DLSS|NIS")
	FString SetUpscaleNIS(UWuReNISMode NISModeLocal);
	UFUNCTION(BlueprintCallable, Category="DLSS")
	FString SetUpscaleBuiltIn();
	UFUNCTION(BlueprintCallable, Category="DLSS")
	void SetAntiAliasingCVar(const EBuiltInAAMode AAMode);

private:
	void SetDLSSNoAuto(const UWuReDLSSMode NewDLSSMode, const bool bDLAAEnabled,
	                   const bool bRestoreFullResWhenDisabled);
	void UpdateDLSSDevStats();
	void SetDLSSAuto(const FVector2D ScreenResolution, const bool bDLSSAuto, const bool bDLAAEnabled,
	                 const bool bRestoreFullResWhenDisabled);

	UPROPERTY(BlueprintReadOnly, Category="DLSS", meta=(AllowPrivateAccess = "true"))
	FDLSSCapabilities DLSSCapabilities;
};
