// Copyright Epic Games, Inc. All Rights Reserved.


#include "CH4_2PlayerController.h"
#include "CH4_2Pawn.h"
#include "CH4_2UI.h"
#include "EnhancedInputSubsystems.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Blueprint/UserWidget.h"
#include "CH4_2.h"
#include "Widgets/Input/SVirtualJoystick.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/GameModeBase.h"

void ACH4_2PlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	// only spawn UI on local player controllers
	if (IsLocalPlayerController())
	{
		if (SVirtualJoystick::ShouldDisplayTouchInterface())
		{
			// spawn the mobile controls widget
			MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

			if (MobileControlsWidget)
			{
				// add the controls to the player screen
				MobileControlsWidget->AddToPlayerScreen(0);

			} else {

				UE_LOG(LogCH4_2, Error, TEXT("Could not spawn mobile controls widget."));

			}
		}
		

		// spawn the UI widget and add it to the viewport
		VehicleUI = CreateWidget<UCH4_2UI>(this, VehicleUIClass);

		if (VehicleUI)
		{
			VehicleUI->AddToViewport();
			UE_LOG(LogCH4_2, Log, TEXT("Vehicle HUD created for local controller %s"), *GetName());

		} else {

			UE_LOG(LogCH4_2, Error, TEXT("Could not spawn vehicle UI widget."));

		}
	}
}

void ACH4_2PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!SVirtualJoystick::ShouldDisplayTouchInterface())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

void ACH4_2PlayerController::Tick(float Delta)
{
	Super::Tick(Delta);

	if (IsValid(VehiclePawn) && IsValid(VehicleUI))
	{
		VehicleUI->UpdateSpeed(VehiclePawn->GetChaosVehicleMovement()->GetForwardSpeed());
		VehicleUI->UpdateGear(VehiclePawn->GetChaosVehicleMovement()->GetCurrentGear());
	}
}

void ACH4_2PlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// get a pointer to the controlled pawn
	VehiclePawn = Cast<ACH4_2Pawn>(InPawn);

	// subscribe to the pawn's OnDestroyed delegate
	if (IsValid(VehiclePawn))
	{
		VehiclePawn->OnDestroyed.AddUniqueDynamic(this, &ACH4_2PlayerController::OnPawnDestroyed);
		UE_LOG(LogCH4_2, Log, TEXT("Vehicle possessed: Controller=%s Pawn=%s Location=%s"),
			*GetName(), *VehiclePawn->GetName(), *VehiclePawn->GetActorLocation().ToString());
	}
}

void ACH4_2PlayerController::OnRep_VehiclePawn()
{
	UE_LOG(LogCH4_2, Log, TEXT("Vehicle reference replicated: Controller=%s Pawn=%s"),
		*GetName(), *GetNameSafe(VehiclePawn));
}

void ACH4_2PlayerController::OnUnPossess()
{
	// Pawn detaches its controller before broadcasting OnDestroyed.
	// Keep that binding during destruction so the template respawn still runs.
	if (IsValid(VehiclePawn) && !VehiclePawn->IsActorBeingDestroyed())
	{
		VehiclePawn->OnDestroyed.RemoveDynamic(this, &ACH4_2PlayerController::OnPawnDestroyed);
	}
	VehiclePawn = nullptr;
	Super::OnUnPossess();
}

void ACH4_2PlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(ACH4_2PlayerController, VehiclePawn, COND_OwnerOnly);
}

void ACH4_2PlayerController::OnPawnDestroyed(AActor* DestroyedPawn)
{
	if (HasAuthority() && IsValid(GetWorld()) && !GetWorld()->bIsTearingDown)
	{
		if (AGameModeBase* GameMode = GetWorld()->GetAuthGameMode())
		{
			GameMode->RestartPlayer(this);
		}
	}
}
