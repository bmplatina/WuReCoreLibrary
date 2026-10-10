// © Platina Production 2017~2024

#pragma once

#include "CoreMinimal.h"
#include "Engine/LocalPlayer.h"
#include "WuReLocalPlayer.generated.h"

class FSceneViewFamily;
class FViewElementDrawer;

/**
 * LocalPlayer used by the portal system.
 * Must be assigned in Project Settings > Engine > General Settings > Default Classes > Local Player Class.
 * (AWuRePlayerController::PerformCameraCut also falls back to the engine's own camera-cut flag when this class is not in use.)
 */
UCLASS()
class WURECORELIBRARY_API UWuReLocalPlayer : public ULocalPlayer
{
	GENERATED_BODY()

public:
	virtual FSceneView* CalcSceneView(FSceneViewFamily* ViewFamily, FVector& OutViewLocation,
	                                  FRotator& OutViewRotation, FViewport* Viewport,
	                                  FViewElementDrawer* ViewDrawer = nullptr,
	                                  int32 StereoViewIndex = INDEX_NONE) override;

	/** Flags the view rendered during the current engine frame as a camera cut (resets TAA/TSR/motion blur history). */
	void PerformCameraCut();

private:
	/** Engine frame in which the cut was requested. Using the frame counter instead of a bool means every view calculated this frame is flagged (e.g. both stereo eyes). */
	uint64 CameraCutFrame = MAX_uint64;
};
