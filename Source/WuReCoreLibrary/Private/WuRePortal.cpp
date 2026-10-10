// © Platina Production 2017~2024


#include "WuRePortal.h"

#include "WuRePlayerCharacter.h"
#include "WuRePlayerController.h"
#include "WuRePortalStatics.h"
#include "GameFramework/PawnMovementComponent.h"

AWuRePortal::AWuRePortal()
{
	PrimaryActorTick.bCanEverTick = true;
	bIsActive = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent->Mobility = EComponentMobility::Static;

	PortalRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("PortalRootComponent"));
	PortalRootComponent->SetupAttachment(GetRootComponent());
	PortalRootComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	PortalRootComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	PortalRootComponent->Mobility = EComponentMobility::Movable;
}

void AWuRePortal::BeginPlay()
{
	Super::BeginPlay();
}

void AWuRePortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AWuRePortal::IsActive()
{
	return bIsActive;
}

void AWuRePortal::SetActive(bool NewActive)
{
	bIsActive = NewActive;
}

AActor* AWuRePortal::GetTarget()
{
	return Target;
}

void AWuRePortal::SetTarget(AActor* NewTarget)
{
	Target = NewTarget;
}

bool AWuRePortal::IsPointInFrontOfPortal(FVector Point, FVector PortalLocation, FVector PortalNormal)
{
	FPlane PortalPlane = FPlane(PortalLocation, PortalNormal);
	float PortalDot = PortalPlane.PlaneDot(Point);

	// If < 0 means we are behind the Plane
	// See: http://api.unrealengine.com/INT/API/Runtime/Core/Math/FPlane/PlaneDot/index.html
	return (PortalDot >= 0);
}

bool AWuRePortal::IsPointCrossingPortal(FVector Point, FVector PortalLocation, FVector PortalNormal)
{
	FVector IntersectionPoint;
	FPlane PortalPlane = FPlane(PortalLocation, PortalNormal);
	float PortalDot = PortalPlane.PlaneDot(Point);
	bool IsCrossing = false;
	bool IsInFront = PortalDot >= 0;

	bool IsIntersect = FMath::SegmentPlaneIntersection(LastPosition,
	                                                   Point,
	                                                   PortalPlane,
	                                                   IntersectionPoint);

	// Did we intersect the portal since last Location ?
	// If yes, check the direction : crossing forward means we were in front and now at the back
	// If we crossed backward, ignore it (similar to Prey 2006)
	if (IsIntersect && !IsInFront && LastInFront)
	{
		IsCrossing = true;
	}

	// Store values for next check
	LastInFront = IsInFront;
	LastPosition = Point;

	return IsCrossing;
}

void AWuRePortal::TeleportActor(AActor* ActorToTeleport)
{
	if (ActorToTeleport == nullptr || Target == nullptr)
	{
		return;
	}

	//-------------------------------
	//Retrieve and save Player Velocity
	//(from the Movement Component)
	//-------------------------------
	FVector SavedVelocity = FVector::ZeroVector;
	AWuRePlayerCharacter* Character = nullptr;

	if (ActorToTeleport->IsA(AWuRePlayerCharacter::StaticClass()))
	{
		Character = Cast<AWuRePlayerCharacter>(ActorToTeleport);

		SavedVelocity = Character->GetMovementComponent()->Velocity;
	}

	// -------------------------------
	// Compute and apply new location
	// -------------------------------
	FHitResult HitResult;
	FVector NewLocation = UWuRePortalStatics::ConvertLocationToActorSpace(ActorToTeleport->GetActorLocation(),
	                                                                      this,
	                                                                      Target);

	ActorToTeleport->SetActorLocation(NewLocation,
	                                  false,
	                                  &HitResult,
	                                  ETeleportType::TeleportPhysics);

	// -------------------------------
	// Compute and apply new rotation
	// -------------------------------
	FRotator NewRotation = UWuRePortalStatics::ConvertRotationToActorSpace(ActorToTeleport->GetActorRotation(),
	                                                                       this,
	                                                                       Target);

	// Apply new rotation
	ActorToTeleport->SetActorRotation(NewRotation);

	// -------------------------------
	// If we are teleporting a character we need to
	// update its controller as well and reapply its velocity
	// -------------------------------
	if (ActorToTeleport->IsA(AWuRePlayerCharacter::StaticClass()))
	{
		// Update Controller
		AWuRePlayerController* PC = Cast<AWuRePlayerController>(Character->GetController());

		if (PC != nullptr)
		{
			NewRotation = UWuRePortalStatics::ConvertRotationToActorSpace(PC->GetControlRotation(),
			                                                              this,
			                                                              Target);

			PC->SetControlRotation(NewRotation);
		}

		// Reapply Velocity (reoriented through the same transform as location/rotation)
		Character->GetMovementComponent()->Velocity = UWuRePortalStatics::ConvertDirectionToActorSpace(SavedVelocity, this, Target);
	}

	//Cleanup Teleport
	LastPosition = NewLocation;
}

void AWuRePortal::ForceTick_Implementation()
{
}

void AWuRePortal::SetRTT_Implementation(UTexture* RenderTexture)
{
}

void AWuRePortal::ClearRTT_Implementation()
{
}
