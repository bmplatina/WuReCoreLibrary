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

UCLASS(DisplayName="WU:RE DLSS Settings Subsystem")
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
	void ApplyNewSettings(float SelectedRenderScale, bool bIsRayTracingChecked, int32 SelectedAAMode,
	                  UDLSSMode SelectedDLSSMode, UNISMode SelectedNISMode, int32 SelectedFGMode,
	                  int32 SelectedReflexMode, int32 SelectedUpscalingMode);

	void ApplySettingsFromUserSettings();
	
	UFUNCTION(BlueprintPure, Category="DLSS|Settings")
	FDLSSCapabilities GetDLSSCapabilities() const;

private:
	void SetUpscaleDLSS(const uint8 DLSSModeLocal);
	void SetUpscaleNIS(const uint8 DLSSModeLocal);
	void SetUpscaleBuiltIn();
	void SetAntiAliasingCVar(const EBuiltInAAMode AAMode);
	void EnableDLAA(const bool bDLAAEnabled);
	void SetDLSSMode(const uint8 DLSSModeLocal);
	void SetDLSSNoAuto(const UDLSSMode NewDLSSMode, const bool bDLAAEnabled, const bool bRestoreFullResWhenDisabled);
	void UpdateDLSSDevStats();
	void SetDLSSAuto(const FVector2D ScreenResolution, const bool bDLSSAuto, const bool bDLAAEnabled,
	                 const bool bRestoreFullResWhenDisabled);

	UPROPERTY(BlueprintReadOnly, Category="DLSS", meta=(AllowPrivateAccess = "true"))
	FDLSSCapabilities DLSSCapabilities;
};
