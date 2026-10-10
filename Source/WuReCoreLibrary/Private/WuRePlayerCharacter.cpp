// © Platina Production 2017~2024


#include "WuRePlayerCharacter.h"

#include "Camera/CameraComponent.h"

// Sets default values
AWuRePlayerCharacter::AWuRePlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("CameraAnchor"));
	CameraAnchor->SetupAttachment(GetRootComponent());

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraAnchor);
}

// Called when the game starts or when spawned
void AWuRePlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AWuRePlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AWuRePlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

UCameraComponent* AWuRePlayerCharacter::GetCameraComponent()
{
	return Camera;
}
