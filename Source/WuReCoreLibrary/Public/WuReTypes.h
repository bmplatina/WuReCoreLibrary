// © Platina Production 2017~2024
#pragma once

#include "CoreMinimal.h"
#include "WuReTypes.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBeginPlayDone);

UENUM(BlueprintType)
enum class ESupportedLanguage: uint8 {
	KO_KR UMETA(DisplayName = "Korean"),
	EN_GB UMETA(DisplayName = "English")
};

USTRUCT(Atomic, BlueprintType)
struct FDLSSCapabilities
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="DLSS")
	bool bSuperResolution      = false; // DLSS 2.0+  (SR/DLAA)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="DLSS")
	bool bFrameGeneration      = false; // DLSS 3.0+  (FG)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="DLSS")
	bool bRayReconstruction    = false; // DLSS 3.5+  (RR)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="DLSS")
	bool bMultiFrameGeneration = false; // DLSS 4.0+  (2x 초과 FG)
};

UENUM(BlueprintType)
enum class EAntiAliasingMode: uint8
{
	Off UMETA(DisplayName = "UI_SETTINGS_DLSS_AA_OFF"), // Off
	TAA UMETA(DisplayName = "UI_SETTINGS_DLSS_AA_TAA") // Temporal Anti-Aliasing
};

UENUM(BlueprintType)
enum class EUpscaleMode: uint8
{
	BuiltIn UMETA(DisplayName = "UI_SETTINGS_DLSS_UPSCALING_BUILTIN"), // Built-In
	DLSS UMETA(DisplayName = "UI_SETTINGS_DLSS_UPSCALING_DLSS"), // NVIDIA DLSS
	NIS UMETA(DisplayName = "UI_SETTINGS_DLSS_UPSCALING_NIS") // NVIDIA Image Scaling
};

UENUM(BlueprintType)
enum class EBuiltInAAMode: uint8
{
	Off UMETA(DisplayName = "UI_SETTINGS_DLSS_BUILTINAA_OFF"), // Off
	FXAA UMETA(DisplayName = "UI_SETTINGS_DLSS_BUILTINAA_FXAA"), // FXAA
	TAA UMETA(DisplayName = "UI_SETTINGS_DLSS_BUILTINAA_TAA"), // Temporal Anti-Aliasing
	MSAA UMETA(DisplayName = "UI_SETTINGS_DLSS_BUILTINAA_MSAA"), // MSAA (Forward shading only)
	TSR UMETA(DisplayName = "UI_SETTINGS_DLSS_BUILTINAA_TSR") // TSR
};

UENUM(BlueprintType)
enum class ELocalizationType : uint8
{
	Dialogue,
	UserInterface,
};

UENUM(BlueprintType, DisplayName="Gamepad Keys (DualSense)")
enum class EDualSenseKey: uint8
{
	FaceKeyUp UMETA(DisplayName="Triangle"),
	FaceKeyRight UMETA(DisplayName="Circle"),
	FaceKeyDown UMETA(DisplayName="X"),
	FaceKeyLeft UMETA(DisplayName="Square"),
	LogoButton UMETA(DisplayName="PS"),
	StartButton UMETA(DisplayName="Menu"),
	SelectButton UMETA(DisplayName="Share"),
	DPadUp UMETA(DisplayName="D-Pad Up"),
	DPadDown UMETA(DisplayName="D-Pad Down"),
	DPadLeft UMETA(DisplayName="D-Pad Left"),
	DPadRight UMETA(DisplayName="D-Pad Right"),
	LShoulder UMETA(DisplayName="L1"),
	LTrigger UMETA(DisplayName="L2"),
	RShoulder UMETA(DisplayName="R1"),
	RTrigger UMETA(DisplayName="R2"),
	LStickUDLR UMETA(DisplayName="Left Stick 4-Directions"),
	LStickButton UMETA(DisplayName="L3"),
	LStickDown UMETA(DisplayName="Left Stick Down"),
	LStickLR UMETA(DisplayName="Left Stick Left-Right"),
	LStickLeft UMETA(DisplayName="Left Stick Left"),
	LStickRight UMETA(DisplayName="Left Stick Right"),
	LStickUD UMETA(DisplayName="Left Stick Up-Down"),
	LStickUp UMETA(DisplayName="Left Stick Up"),
	LStick UMETA(DisplayName="Left Stick"),
	RStickUDLR UMETA(DisplayName="Right Stick 4-Directions"),
	RStickButton UMETA(DisplayName="R3"),
	RStickDown UMETA(DisplayName="Right Stick Down"),
	RStickLR UMETA(DisplayName="Right Stick Left-Right"),
	RStickLeft UMETA(DisplayName="Right Stick Left"),
	RStickRight UMETA(DisplayName="Right Stick Right"),
	RStickUD UMETA(DisplayName="Right Stick Up-Down"),
	RStickUp UMETA(DisplayName="Right Stick Up"),
	RStick UMETA(DisplayName="Right Stick"),
	TPadClick UMETA(DisplayName="Touchpad Click"),
	TPadDown UMETA(DisplayName="Touchpad Drag to Down"),
	TPadLeft UMETA(DisplayName="Touchpad Drag to Left"),
	TPadRight UMETA(DisplayName="Touchpad Drag to Right"),
	TPadUp UMETA(DisplayName="Touchpad Drag to Up"),
	DPad UMETA(DisplayName="D-Pad"),
};

UENUM(BlueprintType)
enum class EJoystickHandler: uint8
{
	LR		UMETA(DisplayName="Left-Right Only"),
	UD		UMETA(DisplayName="Up-Down Only"),
	UDLR	UMETA(DisplayName="4 Directions"),
};

UENUM(BlueprintType)
enum class EInputMode: uint8
{
	GameOnly		UMETA(DisplayName="Game Only"),
	WidgetOnly		UMETA(DisplayName="UI Only"),
	GameAndWidget	UMETA(DisplayName="Game and UI"),
};