// © Platina Production 2017~2024

#include "WuRePlayerController.h"
#include "CommonInputSubsystem.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "SceneView.h"
#include "WuReLocalPlayer.h"
#include "WuRePortalManager.h"
#include "WuReRootWidget.h"
#include "WuReUserWidget.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/GameViewportClient.h"

FMatrix AWuRePlayerController::GetCameraProjectionMatrix()
{
	FMatrix ProjectionMatrix = FMatrix::Identity;

	if (const ULocalPlayer* ControllerLocalPlayer = GetLocalPlayer())
	{
		if (ControllerLocalPlayer->ViewportClient != nullptr)
		{
			FSceneViewProjectionData PlayerProjectionData;

			if (ControllerLocalPlayer->GetProjectionData(ControllerLocalPlayer->ViewportClient->Viewport,
			                                             PlayerProjectionData))
			{
				ProjectionMatrix = PlayerProjectionData.ProjectionMatrix;
			}
		}
	}

	return ProjectionMatrix;
}

void AWuRePlayerController::PerformCameraCut()
{
	if (LocalPlayer != nullptr)
	{
		LocalPlayer->PerformCameraCut();
	}

	// The engine's own flag: keeps seamless teleports working when UWuReLocalPlayer is not the project's LocalPlayerClass
	if (PlayerCameraManager != nullptr)
	{
		PlayerCameraManager->SetGameCameraCutThisFrame();
	}
}

void AWuRePlayerController::UpdateCameraManager(float DeltaSeconds)
{
	Super::UpdateCameraManager(DeltaSeconds);

	// The final POV of this frame is now known (after every actor tick, shakes included): capture the portal with it
	if (PortalManager != nullptr)
	{
		PortalManager->Update(DeltaSeconds);
	}
}

void AWuRePlayerController::BeginPlay()
{
	Super::BeginPlay();
	SetTickableWhenPaused(true);
	if (UCommonInputSubsystem* CommonInputSubsystemLocal = UCommonInputSubsystem::Get(GetLocalPlayer()))
	{
		CurrentInputType = CommonInputSubsystemLocal->GetCurrentInputType();
		BroadcastOnInputMethodChanged(CurrentInputType);

		CommonInputSubsystemLocal->OnInputMethodChangedNative.AddUObject(
			this, &AWuRePlayerController::BroadcastOnInputMethodChanged);
	}

	if (IsValid(EscapeMenuWidgetStaticClass))
	{
		EscapeMenuWidget = CreateWidget<UWuReUserWidget>(this, EscapeMenuWidgetStaticClass);
	}

	// Portal
	LocalPlayer = Cast<UWuReLocalPlayer>(GetLocalPlayer());

	// Only the machine that renders the view needs the capture (no LocalPlayer/viewport on a dedicated server)
	if (IsLocalPlayerController())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;

		PortalManager = GetWorld()->SpawnActor<AWuRePortalManager>(AWuRePortalManager::StaticClass(),
		                                                           FVector::ZeroVector,
		                                                           FRotator::ZeroRotator,
		                                                           SpawnParams);

		if (PortalManager != nullptr)
		{
			PortalManager->SetControllerOwner(this);
			PortalManager->Init();
		}
	}

	OnBeginPlayDone.Broadcast();
}

void AWuRePlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(PortalManager))
	{
		PortalManager->Destroy();
		PortalManager = nullptr;
	}

	if (UCommonInputSubsystem* Subsystem = UCommonInputSubsystem::Get(GetLocalPlayer()))
	{
		Subsystem->OnInputMethodChangedNative.RemoveAll(this);
	}
	Super::EndPlay(EndPlayReason);
}

void AWuRePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (const ULocalPlayer* LocalPlayerLocal = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* SubsystemLocal =
			LocalPlayerLocal->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (InputMappingContext)
			{
				SubsystemLocal->AddMappingContext(InputMappingContext, 0);
			}
		}
	}

	if (UEnhancedInputComponent* EnhancedInputCompLocal = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (IsValid(IA_Escape))
		{
			EnhancedInputCompLocal->BindAction(IA_Escape, ETriggerEvent::Completed, this,
			                                   &AWuRePlayerController::ToggleEscMenu);
		}
	}
}

void AWuRePlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AWuRePlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}


bool AWuRePlayerController::IsLocationInViewport(const FVector& Location)
{
	FVector2D ScreenPosition;

	if (ProjectWorldLocationToScreen(Location, ScreenPosition))
	{
		int32 ViewportSizeX, ViewportSizeY;
		GetViewportSize(ViewportSizeX, ViewportSizeY);

		return ScreenPosition.X > 0 && ScreenPosition.X < ViewportSizeX && ScreenPosition.Y > 0 && ScreenPosition.Y <
			ViewportSizeY;
	}

	return false;
}

UWuReRootWidget* AWuRePlayerController::GetRootWidget()
{
	if (!IsValid(RootWidget))
	{
		if (IsValid(RootWidgetStaticClass))
		{
			RootWidget = CreateWidget<UWuReRootWidget>(this, RootWidgetStaticClass);
			RootWidget->AddToViewport(1);
			WURELOG(Display, TEXT("BeginPlay: PlayerController=%s created RootWidget=%s"), *GetName(),
			        *RootWidget->GetName());
		}
	}

	return RootWidget;
}

void AWuRePlayerController::GetAllWuReWidgetsInRoot(TArray<UWuReUserWidget*>& OutWidgets)
{
	TArray<UCommonActivatableWidget*> WidgetsLocal = GetRootWidget()->GetWidgetStack()->GetWidgetList();
	for (UCommonActivatableWidget* Widget : WidgetsLocal)
	{
		if (UWuReUserWidget* WuReWidget = Cast<UWuReUserWidget>(Widget))
		{
			OutWidgets.Add(WuReWidget);
		}
	}
}

UWuReUserWidget* AWuRePlayerController::GetActivatedWuReWidget()
{
	return Cast<UWuReUserWidget>(GetRootWidget()->GetWidgetStack()->GetActiveWidget());
}

void AWuRePlayerController::ToggleEscMenu()
{
	GetRootWidget()->ToggleEscMenu();
}

void AWuRePlayerController::BroadcastOnInputMethodChanged(ECommonInputType InInputType)
{
	CurrentInputType = InInputType;
	WURELOG(Warning, TEXT("Input method changed: %s"), *UEnum::GetValueAsString(CurrentInputType));
	OnInputMethodChangedNative.Broadcast(CurrentInputType);
}
