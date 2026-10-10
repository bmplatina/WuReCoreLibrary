// © Platina Production 2017~2024

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WuRePortalManager.generated.h"

class AWuRePlayerCharacter;
class AWuRePlayerController;
class AWuRePortal;
class USceneCaptureComponent2D;
class UTextureRenderTarget2D;

/**
 * Spawned by the player controller. Owns the single scene capture shared by every portal, detects the player
 * crossing a portal and renders the view through the closest portal the player is looking at.
 */
UCLASS()
class WURECORELIBRARY_API AWuRePortalManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AWuRePortalManager();

	// Called by a Portal actor when wanting to teleport something
	UFUNCTION(BlueprintCallable, Category="Portal")
	void RequestTeleportByPortal(AWuRePortal* Portal, AActor* TargetToTeleport);

	// Save a reference to the PlayerControler
	void SetControllerOwner(AWuRePlayerController* NewOwner);

	// Various setup that happens during spawn
	void Init();

	// Manual Tick
	void Update(float DeltaTime);

	// Find all the portals in world and update them
	// returns the most valid/usable one for the Player
	AWuRePortal* UpdatePortalsInWorld();

	// Update SceneCapture
	void UpdateCapture(AWuRePortal* Portal);

private:
	//Function to create the Portal render target
	void GeneratePortalTexture();

	UPROPERTY()
	USceneCaptureComponent2D* SceneCapture;

	UPROPERTY(transient)
	UTextureRenderTarget2D* PortalTexture;

	UPROPERTY()
	AWuRePlayerController* ControllerOwner;

	int32 PreviousScreenSizeX;
	int32 PreviousScreenSizeY;

	float UpdateDelay;
	
	UPROPERTY()
	TObjectPtr<AWuRePlayerCharacter> Character;
};
