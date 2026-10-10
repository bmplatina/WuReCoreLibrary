// © Platina Production 2017~2024


#include "WuRePortalManager.h"

#include "EngineUtils.h"
#include "WuReCoreLibrary.h"
#include "WuRePlayerCharacter.h"
#include "WuRePlayerController.h"
#include "WuRePortal.h"
#include "WuRePortalStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/Pawn.h"

namespace
{
	// Portals further away than that are never captured
	constexpr double MaxPortalDistance = 4096.0;

	// How far behind its plane a portal is still considered in front of the camera (cm)
	constexpr double BehindPlaneTolerance = 150.0;

	// Render target size relative to the viewport (~59%), to reduce the performance impact
	constexpr float PortalResolutionScale = 1.0f / 1.7f;
}

// Sets default values
AWuRePortalManager::AWuRePortalManager()
{
	PrimaryActorTick.bCanEverTick = false;
	PortalTexture = nullptr;
	UpdateDelay = 1.1f;

	PreviousScreenSizeX = 0;
	PreviousScreenSizeY = 0;
}

void AWuRePortalManager::RequestTeleportByPortal(AWuRePortal* Portal, AActor* TargetToTeleport)
{
	if (IsValid(Portal) && IsValid(TargetToTeleport))
	{
		if (IsValid(ControllerOwner))
		{
			ControllerOwner->PerformCameraCut();
		}

		Portal->TeleportActor(TargetToTeleport);

		// The capture jumps as well: reset its temporal history on the next capture
		if (SceneCapture != nullptr)
		{
			SceneCapture->bCameraCutThisFrame = true;
		}

		//-----------------------------------
		//Force update
		//-----------------------------------
		// The capture itself happens in Update(), once the camera manager has computed this frame's POV
		AWuRePortal* FuturePortal = UpdatePortalsInWorld();

		if (FuturePortal != nullptr)
		{
			FuturePortal->ForceTick(); //Force update before the player render its view since he just teleported
		}
	}
}

void AWuRePortalManager::SetControllerOwner(AWuRePlayerController* NewOwner)
{
	ControllerOwner = NewOwner;

	if (AWuRePlayerCharacter* CharacterLocal = Cast<AWuRePlayerCharacter>(NewOwner->GetCharacter()))
	{
		Character = CharacterLocal;
	}
}

void AWuRePortalManager::Init()
{
	//------------------------------------------------
	//Create Camera
	//------------------------------------------------
	SceneCapture = NewObject<USceneCaptureComponent2D>(this, USceneCaptureComponent2D::StaticClass(),
	                                                   *FString("PortalSceneCapture"));

	SceneCapture->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
	SceneCapture->RegisterComponent();

	SceneCapture->bCaptureEveryFrame = false;
	SceneCapture->bCaptureOnMovement = false;
	SceneCapture->LODDistanceFactor = 3; //Force bigger LODs for faster computations
	SceneCapture->TextureTarget = nullptr;
	SceneCapture->bEnableClipPlane = true;
	SceneCapture->bUseCustomProjectionMatrix = true;
	SceneCapture->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDRNoAlpha;

	//Setup Post-Process of SceneCapture (optimization : disable Motion Blur, etc)
	FPostProcessSettings CaptureSettings;

	CaptureSettings.bOverride_AmbientOcclusionQuality = true;
	CaptureSettings.bOverride_MotionBlurAmount = true;
	CaptureSettings.bOverride_SceneFringeIntensity = true;
	CaptureSettings.bOverride_FilmGrainIntensity = true;
	CaptureSettings.bOverride_ScreenSpaceReflectionQuality = true;

	CaptureSettings.AmbientOcclusionQuality = 0.0f; //0=lowest quality..100=maximum quality
	CaptureSettings.MotionBlurAmount = 0.0f; //0 = disabled
	CaptureSettings.SceneFringeIntensity = 0.0f; //0 = disabled
	CaptureSettings.FilmGrainIntensity = 0.0f; //0 = disabled
	CaptureSettings.ScreenSpaceReflectionQuality = 0.0f; //0 = disabled

	// CaptureSettings.bOverride_ScreenPercentage = true;
	// CaptureSettings.ScreenPercentage = 100.0f;

	SceneCapture->PostProcessSettings = CaptureSettings;

	//------------------------------------------------
	//Create RTT Buffer
	//------------------------------------------------
	GeneratePortalTexture();
}

void AWuRePortalManager::Update(float DeltaTime)
{
	//-----------------------------------
	// Generate Portal texture ?
	//-----------------------------------
	UpdateDelay += DeltaTime;

	if (UpdateDelay > 1.0f)
	{
		UpdateDelay = 0.0f;
		GeneratePortalTexture();
	}

	//-----------------------------------
	// Find portals in the level and update them
	//-----------------------------------
	AWuRePortal* Portal = UpdatePortalsInWorld();

	if (Portal != nullptr)
	{
		UpdateCapture(Portal);
	}
}

AWuRePortal* AWuRePortalManager::UpdatePortalsInWorld()
{
	if (ControllerOwner == nullptr)
	{
		return nullptr;
	}

	if (!Character)
	{
		Character = Cast<AWuRePlayerCharacter>(ControllerOwner->GetCharacter());
	}

	if (!Character)
	{
		return nullptr;
	}

	//-----------------------------------
	// Update Portal actors in the world (and active one if nearby)
	//-----------------------------------
	AWuRePortal* ActivePortal = nullptr;
	FVector PlayerLocation = Character->GetActorLocation();
	float Distance = 4096.0f;

	for (TActorIterator<AWuRePortal> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		AWuRePortal* Portal = *ActorItr;

		// Streamed-in portals can be iterated before their BeginPlay ran (material instance not created yet)
		if (!Portal->HasActorBegunPlay())
		{
			continue;
		}

		FVector PortalLocation = Portal->GetActorLocation();
		FVector PortalNormal = -1 * Portal->GetActorForwardVector();

		// Reset Portal
		Portal->ClearRTT();
		Portal->SetActive(false);

		// Find the closest Portal when the player is Standing in front of
		float NewDistance = FMath::Abs(FVector::Dist(PlayerLocation, PortalLocation));

		if (NewDistance < Distance)
		{
			Distance = NewDistance;
			ActivePortal = Portal;
		}
	}

	return ActivePortal;
}

void AWuRePortalManager::UpdateCapture(AWuRePortal* Portal)
{
	if (ControllerOwner == nullptr)
	{
		return;
	}

	//-----------------------------------
	// Update SceneCapture (discard if there is no active portal)
	//-----------------------------------
	if (SceneCapture != nullptr
		&& PortalTexture != nullptr
		&& Portal != nullptr
		&& ControllerOwner->PlayerCameraManager != nullptr)
	{
		// Final POV of this frame (camera shakes, modifiers, view target blends included),
		// the same one the player view and GetCameraProjectionMatrix() are built from
		const APlayerCameraManager* CameraManager = ControllerOwner->PlayerCameraManager;
		const FVector ViewLocation = CameraManager->GetCameraLocation();
		const FQuat ViewQuat = CameraManager->GetCameraRotation().Quaternion();

		AActor* Target = Portal->GetTarget();

		//Place the SceneCapture to the Target
		if (Target != nullptr)
		{
			//-------------------------------
			// Compute new location in the space of the target actor
			// (which may not be aligned to world)
			//-------------------------------
			FVector NewLocation = UWuRePortalStatics::ConvertLocationToActorSpace(ViewLocation,
				Portal,
				Target);

			SceneCapture->SetWorldLocation(NewLocation);

			//-------------------------------
			//Compute new Rotation in the space of the
			//Target location (same exit orientation as the location)
			//-------------------------------
			const FQuat NewWorldQuat = UWuRePortalStatics::ConvertQuatToActorSpace(ViewQuat, Portal, Target);

			//Update SceneCapture rotation
			SceneCapture->SetWorldRotation(NewWorldQuat);

			//-------------------------------
			//Clip Plane : to ignore objects between the
			//SceneCapture and the Target of the portal
			//(only the exit side of the Target is kept)
			//-------------------------------
			const FVector ExitForward = UWuRePortalStatics::GetExitQuat(Target).GetForwardVector();
			SceneCapture->ClipPlaneNormal = ExitForward;
			SceneCapture->ClipPlaneBase = Target->GetActorLocation() - ExitForward * 1.5f; // Offset to avoid visible pixel border
		}

		// Switch on the valid Portal
		Portal->SetActive(true);

		// Assign the Render Target
		Portal->SetRTT(PortalTexture);
		SceneCapture->TextureTarget = PortalTexture;

		// Get the Projection Matrix
		SceneCapture->CustomProjectionMatrix = ControllerOwner->GetCameraProjectionMatrix();

		// Say Cheeeeese !
		SceneCapture->CaptureScene();
	}
}

void AWuRePortalManager::GeneratePortalTexture()
{
	int32 CurrentSizeX = 1920;
	int32 CurrentSizeY = 1080;

	if (ControllerOwner != nullptr)
	{
		ControllerOwner->GetViewportSize(CurrentSizeX, CurrentSizeY);
	}

	// Use a smaller size than the current
	// screen to reduce the performance impact
	CurrentSizeX = FMath::Clamp(FMath::RoundToInt(CurrentSizeX * PortalResolutionScale), 128, 1920);
	CurrentSizeY = FMath::Clamp(FMath::RoundToInt(CurrentSizeY * PortalResolutionScale), 128, 1080);

	if (PortalTexture != nullptr
		&& CurrentSizeX == PreviousScreenSizeX
		&& CurrentSizeY == PreviousScreenSizeY)
	{
		return;
	}

	PreviousScreenSizeX = CurrentSizeX;
	PreviousScreenSizeY = CurrentSizeY;

	// Create the RenderTarget if it does not exist
	if (PortalTexture == nullptr)
	{
		// Create new RTT
		PortalTexture = NewObject<UTextureRenderTarget2D>(
			this,
			UTextureRenderTarget2D::StaticClass(),
			TEXT("PortalRenderTarget")
		);
		check(PortalTexture);

		PortalTexture->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA16f;
		PortalTexture->Filter = TextureFilter::TF_Bilinear;
		PortalTexture->SizeX = CurrentSizeX;
		PortalTexture->SizeY = CurrentSizeY;
		PortalTexture->ClearColor = FLinearColor::Black;
		PortalTexture->bForceLinearGamma = true; // The capture is HDR scene color, which is linear
		PortalTexture->AddressX = TextureAddress::TA_Clamp;
		PortalTexture->AddressY = TextureAddress::TA_Clamp;

		// Not needed since the texture is displayed on screen directly
		// in some engine versions this can even lead to crashes (notably 4.24/4.25)
		PortalTexture->bAutoGenerateMips = false;

		// This force the engine to create the render target
		// with the parameters we defined just above
		PortalTexture->UpdateResource();
	}
	// Resize the RenderTarget if it already exists
	else
	{
		PortalTexture->ResizeTarget(CurrentSizeX, CurrentSizeY);
	}
}
