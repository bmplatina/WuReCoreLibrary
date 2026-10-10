// © Platina Production 2017~2024


#include "WuReLocalPlayer.h"

#include "SceneView.h"

FSceneView* UWuReLocalPlayer::CalcSceneView(FSceneViewFamily* ViewFamily, FVector& OutViewLocation,
                                            FRotator& OutViewRotation, FViewport* Viewport,
                                            FViewElementDrawer* ViewDrawer, int32 StereoViewIndex)
{
	FSceneView* View = Super::CalcSceneView(ViewFamily, OutViewLocation, OutViewRotation, Viewport, ViewDrawer,
	                                        StereoViewIndex);

	if (View != nullptr && CameraCutFrame == GFrameCounter)
	{
		View->bCameraCut = true;
	}

	return View;
}

void UWuReLocalPlayer::PerformCameraCut()
{
	CameraCutFrame = GFrameCounter;
}
