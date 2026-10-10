// © Platina Production 2017~2024

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WuRePlayerCharacter.generated.h"

class UCameraComponent;

UCLASS()
class WURECORELIBRARY_API AWuRePlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AWuRePlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Portal")
	TObjectPtr<USceneComponent> CameraAnchor;

	/** Volume of the portal, meant for the Blueprint's IsPointInsideBox checks. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Portal")
	TObjectPtr<UCameraComponent> Camera;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintPure, Category="WU:RE Character|Camera")
	UCameraComponent* GetCameraComponent();
};
