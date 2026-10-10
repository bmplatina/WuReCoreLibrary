// © Platina Production 2017~2024

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WuRePortalStatics.generated.h"

class AWuRePortalManager;
class UBoxComponent;

/**
 * 
 */
UCLASS()
class WURECORELIBRARY_API UWuRePortalStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category="Portal", meta=(WorldContext="WorldContextObject"))
	static AWuRePortalManager* GetPortalManager(const UObject* WorldContextObject);

	/**
	 * Orientation of the exit of a Target: its forward vector is the direction of travel after the teleport.
	 * A plain Target (e.g. a TargetPoint) is its own exit. When the Target is another portal, the exit is that portal
	 * seen from its front, so its orientation is turned around 180 degrees on its up axis.
	 */
	static FQuat GetExitQuat(const AActor* Target);

	/** Converts a world location from the local space of Reference to the local space of Target (and back to world). */
	UFUNCTION(BlueprintPure, Category="Portal")
	static FVector ConvertLocationToActorSpace(FVector Location, AActor* Reference, AActor* Target);

	/** Same as ConvertLocationToActorSpace for a direction/velocity (no translation). */
	UFUNCTION(BlueprintPure, Category="Portal")
	static FVector ConvertDirectionToActorSpace(FVector Direction, AActor* Reference, AActor* Target);

	UFUNCTION(BlueprintPure, Category="Portal")
	static FRotator ConvertRotationToActorSpace(FRotator Rotation, AActor* Reference, AActor* Target);

	/** Same as ConvertRotationToActorSpace, but stays in quaternion space (no gimbal issues for the capture camera). */
	static FQuat ConvertQuatToActorSpace(const FQuat& Rotation, const AActor* Reference, const AActor* Target);

	UFUNCTION(BlueprintPure, Category="Portal")
	static bool IsPointInsideBox(FVector Point, UBoxComponent* Box);
};
