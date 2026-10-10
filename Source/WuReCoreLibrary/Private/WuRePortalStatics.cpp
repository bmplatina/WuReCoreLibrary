// © Platina Production 2017~2024


#include "WuRePortalStatics.h"

#include "WuRePortal.h"
#include "WuRePortalManager.h"
#include "WuRePlayerController.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"

FQuat UWuRePortalStatics::GetExitQuat(const AActor* Target)
{
	if (Target == nullptr)
	{
		return FQuat::Identity;
	}

	FQuat ExitQuat = Target->GetActorQuat();

	// Linking two portals: you leave the other portal through its front, facing away from it
	if (Target->IsA<AWuRePortal>())
	{
		ExitQuat = ExitQuat * FQuat(FVector::UpVector, UE_DOUBLE_PI);
	}

	return ExitQuat;
}

AWuRePortalManager* UWuRePortalStatics::GetPortalManager(const UObject* WorldContextObject)
{
	AWuRePortalManager* Manager = nullptr;

	// Retrieve the World from the Context actor
	if (WorldContextObject != nullptr && WorldContextObject->GetWorld() != nullptr)
	{
		// Find PlayerController
		AWuRePlayerController* EPC = Cast<AWuRePlayerController>(WorldContextObject->GetWorld()->GetFirstPlayerController());

		// Retrieve the Portal Manager
		if (EPC != nullptr && EPC->PortalManager != nullptr)
		{
			Manager = EPC->PortalManager;
		}
	}

	return Manager;
}

FVector UWuRePortalStatics::ConvertLocationToActorSpace(FVector Location, AActor* Reference, AActor* Target)
{
	if (Reference == nullptr || Target == nullptr)
	{
		return FVector::ZeroVector;
	}

	const FVector Direction = Location - Reference->GetActorLocation();

	return Target->GetActorLocation() + ConvertDirectionToActorSpace(Direction, Reference, Target);
}

FVector UWuRePortalStatics::ConvertDirectionToActorSpace(FVector Direction, AActor* Reference, AActor* Target)
{
	if (Reference == nullptr || Target == nullptr)
	{
		return FVector::ZeroVector;
	}

	FVector Dots;
	Dots.X = FVector::DotProduct(Direction, Reference->GetActorForwardVector());
	Dots.Y = FVector::DotProduct(Direction, Reference->GetActorRightVector());
	Dots.Z = FVector::DotProduct(Direction, Reference->GetActorUpVector());

	const FQuat ExitQuat = GetExitQuat(Target);

	return Dots.X * ExitQuat.GetForwardVector()
		+ Dots.Y * ExitQuat.GetRightVector()
		+ Dots.Z * ExitQuat.GetUpVector();
}

FQuat UWuRePortalStatics::ConvertQuatToActorSpace(const FQuat& Rotation, const AActor* Reference, const AActor* Target)
{
	if (Reference == nullptr || Target == nullptr)
	{
		return FQuat::Identity;
	}

	// Rotation relative to the reference, re-applied on top of the target
	const FQuat LocalQuat = Reference->GetActorQuat().Inverse() * Rotation;
	const FQuat NewWorldQuat = GetExitQuat(Target) * LocalQuat;

	return NewWorldQuat.GetNormalized();
}

FRotator UWuRePortalStatics::ConvertRotationToActorSpace(FRotator Rotation, AActor* Reference, AActor* Target)
{
	if (Reference == nullptr || Target == nullptr)
	{
		return FRotator::ZeroRotator;
	}

	return ConvertQuatToActorSpace(Rotation.Quaternion(), Reference, Target).Rotator();
}

bool UWuRePortalStatics::IsPointInsideBox(FVector Point, UBoxComponent* Box)
{
	if (Box != nullptr)
	{
		// From :
		// https://stackoverflow.com/questions/52673935/check-if-3d-point-inside-a-box/52674010

		const FVector Center = Box->GetComponentLocation();
		const FVector Half = Box->GetScaledBoxExtent();
		const FVector DirectionX = Box->GetForwardVector();
		const FVector DirectionY = Box->GetRightVector();
		const FVector DirectionZ = Box->GetUpVector();

		const FVector Direction = Point - Center;

		return FMath::Abs(FVector::DotProduct(Direction, DirectionX)) <= Half.X &&
			FMath::Abs(FVector::DotProduct(Direction, DirectionY)) <= Half.Y &&
			FMath::Abs(FVector::DotProduct(Direction, DirectionZ)) <= Half.Z;
	}

	return false;
}
