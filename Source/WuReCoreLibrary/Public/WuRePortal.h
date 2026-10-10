// © Platina Production 2017~2024

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WuRePortal.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UTexture;

UCLASS()
class WURECORELIBRARY_API AWuRePortal : public AActor
{
	GENERATED_BODY()
	
	AWuRePortal();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// Status of the Portal (being visualized by the player or not)
	UFUNCTION(BlueprintPure, Category="Portal")
	bool IsActive();

	UFUNCTION(BlueprintCallable, Category="Portal")
	void SetActive(bool NewActive);

	// Render target to use to display the portal
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Portal")
	void ClearRTT();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Portal")
	void SetRTT(UTexture* RenderTexture);

	UFUNCTION(BlueprintNativeEvent, Category="Portal")
	void ForceTick();

	// Target of where the portal is looking
	UFUNCTION(BlueprintPure, Category="Portal")
	AActor* GetTarget();

	UFUNCTION(BlueprintCallable, Category="Portal")
	void SetTarget(AActor* NewTarget);

	// Helpers
	UFUNCTION(BlueprintCallable, Category="Portal")
	bool IsPointInFrontOfPortal(FVector Point, FVector PortalLocation, FVector PortalNormal);

	UFUNCTION(BlueprintCallable, Category="Portal")
	bool IsPointCrossingPortal(FVector Point, FVector PortalLocation, FVector PortalNormal);

	UFUNCTION(BlueprintCallable, Category="Portal")
	void TeleportActor(AActor* ActorToTeleport);

protected:
	UPROPERTY(BlueprintReadOnly, Category="Portal")
	USceneComponent* PortalRootComponent;

private:
	bool bIsActive;

	UPROPERTY()
	TObjectPtr<AActor> Target;

	// Used for Tracking movement of a point 
	FVector LastPosition;
	bool LastInFront;
};
