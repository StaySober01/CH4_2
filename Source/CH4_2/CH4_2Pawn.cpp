// Copyright Epic Games, Inc. All Rights Reserved.

#include "CH4_2Pawn.h"
#include "CH4_2WheelFront.h"
#include "CH4_2WheelRear.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "CH4_2.h"
#include "CH4_2PlayerState.h"
#include "CH4_2PlayerController.h"
#include "Components/ArrowComponent.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/DamageType.h"
#include "HAL/PlatformTime.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Templates/UnrealTemplate.h"

#define LOCTEXT_NAMESPACE "VehiclePawn"

ACH4_2Pawn::ACH4_2Pawn()
{
	// construct the front camera boom
	FrontSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Front Spring Arm"));
	FrontSpringArm->SetupAttachment(GetMesh());
	FrontSpringArm->TargetArmLength = 0.0f;
	FrontSpringArm->bDoCollisionTest = false;
	FrontSpringArm->bEnableCameraRotationLag = true;
	FrontSpringArm->CameraRotationLagSpeed = 15.0f;
	FrontSpringArm->SetRelativeLocation(FVector(30.0f, 0.0f, 120.0f));

	FrontCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Front Camera"));
	FrontCamera->SetupAttachment(FrontSpringArm);
	FrontCamera->bAutoActivate = false;

	// construct the back camera boom
	BackSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Back Spring Arm"));
	BackSpringArm->SetupAttachment(GetMesh());
	BackSpringArm->TargetArmLength = 650.0f;
	BackSpringArm->SocketOffset.Z = 150.0f;
	BackSpringArm->bDoCollisionTest = false;
	BackSpringArm->bInheritPitch = false;
	BackSpringArm->bInheritRoll = false;
	BackSpringArm->bEnableCameraRotationLag = true;
	BackSpringArm->CameraRotationLagSpeed = 2.0f;
	BackSpringArm->CameraLagMaxDistance = 50.0f;

	BackCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Back Camera"));
	BackCamera->SetupAttachment(BackSpringArm);

	// Configure the car mesh
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(FName("Vehicle"));

	// get the Chaos Wheeled movement component
	ChaosVehicleMovement = CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());
	AttackOrigin = CreateDefaultSubobject<UArrowComponent>(TEXT("AttackOrigin"));
	AttackOrigin->SetupAttachment(GetMesh());
	AttackOrigin->SetRelativeLocation(FVector(200.0f, 0.0f, 50.0f));
	AttackOrigin->SetHiddenInGame(true);

}

void ACH4_2Pawn::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		// Apply after Blueprint defaults: server pickup triggers require both sides to overlap.
		GetMesh()->SetGenerateOverlapEvents(true);
		if (!AttackHitEffect)
		{
			UE_LOG(LogCH4_2, Warning, TEXT("Vehicle %s: assign AttackHitEffect in the vehicle Blueprint."), *GetName());
		}
	}
}

void ACH4_2Pawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (AttackAction)
		{
			EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ACH4_2Pawn::Attack);
		}
		else
		{
			UE_LOG(LogCH4_2, Warning, TEXT("Vehicle %s: assign IA_Fire to AttackAction."), *GetName());
		}
		// steering 
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Triggered, this, &ACH4_2Pawn::Steering);
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Completed, this, &ACH4_2Pawn::Steering);

		// throttle 
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &ACH4_2Pawn::Throttle);
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Completed, this, &ACH4_2Pawn::Throttle);

		// break 
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &ACH4_2Pawn::Brake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Started, this, &ACH4_2Pawn::StartBrake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Completed, this, &ACH4_2Pawn::StopBrake);

		// handbrake 
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Started, this, &ACH4_2Pawn::StartHandbrake);
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &ACH4_2Pawn::StopHandbrake);

		// look around 
		EnhancedInputComponent->BindAction(LookAroundAction, ETriggerEvent::Triggered, this, &ACH4_2Pawn::LookAround);

		// toggle camera 
		EnhancedInputComponent->BindAction(ToggleCameraAction, ETriggerEvent::Triggered, this, &ACH4_2Pawn::ToggleCamera);

		// reset the vehicle 
		EnhancedInputComponent->BindAction(ResetVehicleAction, ETriggerEvent::Triggered, this, &ACH4_2Pawn::ResetVehicle);
	}
	else
	{
		UE_LOG(LogCH4_2, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ACH4_2Pawn::Tick(float Delta)
{
	Super::Tick(Delta);
	ApplySpeedLimit();

	// add some angular damping if the vehicle is in midair
	bool bMovingOnGround = ChaosVehicleMovement->IsMovingOnGround();
	GetMesh()->SetAngularDamping(bMovingOnGround ? 0.0f : 3.0f);

	// realign the camera yaw to face front
	float CameraYaw = BackSpringArm->GetRelativeRotation().Yaw;
	CameraYaw = FMath::FInterpTo(CameraYaw, 0.0f, Delta, 1.0f);

	BackSpringArm->SetRelativeRotation(FRotator(0.0f, CameraYaw, 0.0f));
}

void ACH4_2Pawn::ApplySpeedLimit()
{
	// Simulated proxies follow replicated physics; they cannot read the owner's bonus.
	if ((!HasAuthority() && !IsLocallyControlled()) || !GetMesh()->IsSimulatingPhysics())
	{
		return;
	}
	const ACH4_2PlayerState* Stats = GetPlayerState<ACH4_2PlayerState>();
	const float Bonus = IsValid(Stats) ? Stats->GetBonusMaxSpeed() : 0.0f;
	if (!FMath::IsFinite(BaseMaxSpeedKmh) || !FMath::IsFinite(Bonus))
	{
		return;
	}
	// Use double for the conversion so even large finite float stats cannot overflow.
	const double LimitCmPerSecond = FMath::Max(0.0, static_cast<double>(BaseMaxSpeedKmh) + Bonus) / 0.036;
	FVector Velocity = GetMesh()->GetPhysicsLinearVelocity();
	const double HorizontalSpeed = Velocity.Size2D();
	if (HorizontalSpeed > LimitCmPerSecond && HorizontalSpeed > UE_SMALL_NUMBER)
	{
		const double Scale = LimitCmPerSecond / HorizontalSpeed;
		Velocity.X *= Scale;
		Velocity.Y *= Scale;
		// Preserve vertical velocity for jumps and gravity.
		GetMesh()->SetPhysicsLinearVelocity(Velocity);
	}
}

bool ACH4_2Pawn::GetAttackPlayer(ACH4_2PlayerController*& OutController, ACH4_2PlayerState*& OutStats) const
{
	OutController = Cast<ACH4_2PlayerController>(GetController());
	OutStats = GetPlayerState<ACH4_2PlayerState>();
	return !IsActorBeingDestroyed() && IsValid(OutController) && !OutController->IsActorBeingDestroyed()
		&& OutController->GetPawn() == this && GetOwner() == OutController
		&& IsValid(OutStats) && !OutStats->IsActorBeingDestroyed()
		&& OutStats->GetOwner() == OutController
		&& OutController->GetPlayerState<ACH4_2PlayerState>() == OutStats;
}

void ACH4_2Pawn::Attack()
{
	ACH4_2PlayerController* PC = Cast<ACH4_2PlayerController>(GetController());
	if (!IsLocallyControlled() || !IsValid(PC) || PC->IsChatOpen() || IsActorBeingDestroyed())
	{
		return;
	}
	const double Now = FPlatformTime::Seconds();
	if (Now < NextLocalAttackTime)
	{
		return;
	}
	NextLocalAttackTime = Now + 2.0;
	ServerAttack();
}

void ACH4_2Pawn::ServerAttack_Implementation()
{
	ACH4_2PlayerController* PC = nullptr;
	ACH4_2PlayerState* Stats = nullptr;
	if (!HasAuthority() || bResolvingAttack || !GetWorld() || GetWorld()->bIsTearingDown || !IsValid(AttackOrigin)
		|| !GetAttackPlayer(PC, Stats) || !PC->TryStartVehicleAttack())
	{
		return;
	}
	TGuardValue<bool> ResolvingGuard(bResolvingAttack, true);
	const FVector Start = AttackOrigin->GetComponentLocation();
	const FVector End = Start + GetActorForwardVector() * AttackRangeCm;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(VehicleAttack), false, this);
	FHitResult Hit;
	// Reject a muzzle already beyond a thin wall instead of tracing through it.
	const bool bOriginBlocked = GetWorld()->LineTraceSingleByChannel(Hit, GetActorLocation(), Start, ECC_Visibility, Params);
	FVector DrawStart = bOriginBlocked ? GetActorLocation() : Start;
	FVector DrawEnd = End;
	ACH4_2Pawn* Target = nullptr;
	if (bOriginBlocked)
	{
		DrawEnd = Hit.ImpactPoint;
	}
	else if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		DrawEnd = Hit.ImpactPoint;
		Target = Cast<ACH4_2Pawn>(Hit.GetActor());
	}
	ACH4_2PlayerController* TargetPC = nullptr;
	ACH4_2PlayerState* TargetStats = nullptr;
	const bool bHitVehicle = IsValid(Target) && Target != this && Target->CanBeDamaged()
		&& Target->GetAttackPlayer(TargetPC, TargetStats) && TargetPC != PC;
	const FString TargetName = GetNameSafe(Target);
	float Transferred = 0.0f;
	if (bHitVehicle)
	{
		PendingAttackTarget = Target;
		Transferred = UGameplayStatics::ApplyDamage(Target, 5.0f, PC, this, UDamageType::StaticClass());
		PendingAttackTarget.Reset();
	}
	// Damage/UI callbacks may end play or travel to another world.
	if (IsActorBeingDestroyed() || !GetWorld() || GetWorld()->bIsTearingDown)
	{
		return;
	}
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ACH4_2PlayerController* Recipient = Cast<ACH4_2PlayerController>(It->Get());
			IsValid(Recipient) && !Recipient->IsActorBeingDestroyed())
		{
			Recipient->ClientShowVehicleAttack(DrawStart, DrawEnd, bHitVehicle, AttackHitEffect);
		}
	}
	UE_LOG(LogCH4_2, Log, TEXT("Vehicle attack: attacker=%s target=%s transferred=%.0f originBlocked=%d"),
		*GetName(), *TargetName, Transferred, bOriginBlocked);
}

float ACH4_2Pawn::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	ACH4_2Pawn* Attacker = Cast<ACH4_2Pawn>(DamageCauser);
	ACH4_2PlayerController* AttackerPC = nullptr;
	ACH4_2PlayerState* AttackerStats = nullptr;
	ACH4_2PlayerController* VictimPC = nullptr;
	ACH4_2PlayerState* VictimStats = nullptr;
	if (!HasAuthority() || !CanBeDamaged() || !IsValid(Attacker) || Attacker == this
		|| !Attacker->HasAuthority() || Attacker->GetWorld() != GetWorld()
		|| !Attacker->bResolvingAttack || Attacker->PendingAttackTarget.Get() != this
		|| DamageAmount != 5.0f || !Attacker->GetAttackPlayer(AttackerPC, AttackerStats)
		|| EventInstigator != AttackerPC || !GetAttackPlayer(VictimPC, VictimStats) || VictimPC == AttackerPC
		|| !ShouldTakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser))
	{
		return 0.0f;
	}
	// Consume this server trace once, before delegates or Blueprint damage events can re-enter.
	Attacker->PendingAttackTarget.Reset();
	if (!VictimStats->TryTransferCoinsTo(AttackerStats, 5))
	{
		return 0.0f;
	}
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	return 5.0f;
}

void ACH4_2Pawn::Steering(const FInputActionValue& Value)
{
	// route the input
	DoSteering(Value.Get<float>());
}

void ACH4_2Pawn::Throttle(const FInputActionValue& Value)
{
	// route the input
	DoThrottle(Value.Get<float>());
}

void ACH4_2Pawn::Brake(const FInputActionValue& Value)
{
	// route the input
	DoBrake(Value.Get<float>());
}

void ACH4_2Pawn::StartBrake(const FInputActionValue& Value)
{
	// route the input
	DoBrakeStart();
}

void ACH4_2Pawn::StopBrake(const FInputActionValue& Value)
{
	// route the input
	DoBrakeStop();
}

void ACH4_2Pawn::StartHandbrake(const FInputActionValue& Value)
{
	// route the input
	DoHandbrakeStart();
}

void ACH4_2Pawn::StopHandbrake(const FInputActionValue& Value)
{
	// route the input
	DoHandbrakeStop();
}

void ACH4_2Pawn::LookAround(const FInputActionValue& Value)
{
	// route the input
	DoLookAround(Value.Get<float>());
}

void ACH4_2Pawn::ToggleCamera(const FInputActionValue& Value)
{
	// route the input
	DoToggleCamera();
}

void ACH4_2Pawn::ResetVehicle(const FInputActionValue& Value)
{
	// route the input
	DoResetVehicle();
}

void ACH4_2Pawn::DoSteering(float SteeringValue)
{
	// add the input
	ChaosVehicleMovement->SetSteeringInput(SteeringValue);
}

void ACH4_2Pawn::DoThrottle(float ThrottleValue)
{
	// add the input
	ChaosVehicleMovement->SetThrottleInput(ThrottleValue);

	// reset the brake input
	ChaosVehicleMovement->SetBrakeInput(0.0f);
}

void ACH4_2Pawn::DoBrake(float BrakeValue)
{
	// add the input
	ChaosVehicleMovement->SetBrakeInput(BrakeValue);

	// reset the throttle input
	ChaosVehicleMovement->SetThrottleInput(0.0f);
}

void ACH4_2Pawn::DoBrakeStart()
{
	// call the Blueprint hook for the brake lights
	BrakeLights(true);
}

void ACH4_2Pawn::DoBrakeStop()
{
	// call the Blueprint hook for the brake lights
	BrakeLights(false);

	// reset brake input to zero
	ChaosVehicleMovement->SetBrakeInput(0.0f);
}

void ACH4_2Pawn::DoHandbrakeStart()
{
	// add the input
	ChaosVehicleMovement->SetHandbrakeInput(true);

	// call the Blueprint hook for the break lights
	BrakeLights(true);
}

void ACH4_2Pawn::DoHandbrakeStop()
{
	// add the input
	ChaosVehicleMovement->SetHandbrakeInput(false);

	// call the Blueprint hook for the break lights
	BrakeLights(false);
}

void ACH4_2Pawn::DoLookAround(float YawDelta)
{
	// rotate the spring arm
	BackSpringArm->AddLocalRotation(FRotator(0.0f, YawDelta, 0.0f));
}

void ACH4_2Pawn::DoToggleCamera()
{
	// toggle the active camera flag
	bFrontCameraActive = !bFrontCameraActive;

	FrontCamera->SetActive(bFrontCameraActive);
	BackCamera->SetActive(!bFrontCameraActive);
}

void ACH4_2Pawn::DoResetVehicle()
{
	// reset to a location slightly above our current one
	FVector ResetLocation = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);

	// reset to our yaw. Ignore pitch and roll
	FRotator ResetRotation = GetActorRotation();
	ResetRotation.Pitch = 0.0f;
	ResetRotation.Roll = 0.0f;

	// teleport the actor to the reset spot and reset physics
	SetActorTransform(FTransform(ResetRotation, ResetLocation, FVector::OneVector), false, nullptr, ETeleportType::TeleportPhysics);

	GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	GetMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector);
}

#undef LOCTEXT_NAMESPACE
