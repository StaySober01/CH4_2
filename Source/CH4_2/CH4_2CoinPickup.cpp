#include "CH4_2CoinPickup.h"
#include "CH4_2.h"
#include "CH4_2Pawn.h"
#include "CH4_2PlayerController.h"
#include "CH4_2PlayerState.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

ACH4_2CoinPickup::ACH4_2CoinPickup()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(false);
	SetNetUpdateFrequency(1.0f);
	SetMinNetUpdateFrequency(1.0f);

	PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
	SetRootComponent(PickupSphere);
	PickupSphere->InitSphereRadius(100.0f);
	PickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupSphere->SetCollisionObjectType(ECC_WorldDynamic);
	PickupSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupSphere->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Overlap);
	PickupSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	PickupSphere->SetGenerateOverlapEvents(true);

	CoinMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoinMesh"));
	CoinMesh->SetupAttachment(PickupSphere);
	CoinMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CoinMesh->SetGenerateOverlapEvents(false);
	CoinMesh->SetRelativeScale3D(FVector(0.6f, 0.6f, 0.12f));
	CoinMesh->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (MeshAsset.Succeeded())
	{
		CoinMesh->SetStaticMesh(MeshAsset.Object);
	}
}

void ACH4_2CoinPickup::BeginPlay()
{
	Super::BeginPlay();
	if (!HasAuthority())
	{
		PickupSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		return;
	}

	if (!PickupEffect || !PickupSound)
	{
		UE_LOG(LogCH4_2, Warning, TEXT("Coin %s: assign PickupEffect and PickupSound in the coin Blueprint."), *GetName());
	}
	PickupSphere->OnComponentBeginOverlap.AddUniqueDynamic(this, &ACH4_2CoinPickup::OnPickupOverlap);
	// A placed or dynamically spawned coin may already overlap a possessed car.
	TArray<AActor*> OverlappingActors;
	PickupSphere->GetOverlappingActors(OverlappingActors, ACH4_2Pawn::StaticClass());
	for (AActor* OtherActor : OverlappingActors)
	{
		TryCollect(OtherActor);
		if (bConsumed)
		{
			break;
		}
	}
}

void ACH4_2CoinPickup::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	PickupSphere->OnComponentBeginOverlap.RemoveDynamic(this, &ACH4_2CoinPickup::OnPickupOverlap);
	Super::EndPlay(EndPlayReason);
}

void ACH4_2CoinPickup::OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	TryCollect(OtherActor);
}

void ACH4_2CoinPickup::TryCollect(AActor* OtherActor)
{
	if (!HasAuthority() || bConsumed || IsActorBeingDestroyed())
	{
		return;
	}
	ACH4_2Pawn* Vehicle = Cast<ACH4_2Pawn>(OtherActor);
	if (!IsValid(Vehicle) || Vehicle->IsActorBeingDestroyed() || !PickupSphere->IsOverlappingActor(Vehicle))
	{
		return;
	}
	ACH4_2PlayerController* Controller = Cast<ACH4_2PlayerController>(Vehicle->GetController());
	ACH4_2PlayerState* Stats = Vehicle->GetPlayerState<ACH4_2PlayerState>();
	if (!IsValid(Controller) || Controller->IsActorBeingDestroyed() || Controller->GetPawn() != Vehicle
		|| !IsValid(Stats) || Stats->IsActorBeingDestroyed() || Stats->GetOwner() != Controller
		|| Controller->GetPlayerState<ACH4_2PlayerState>() != Stats)
	{
		return;
	}

	// Set the guard before AddCoins broadcasts UI delegates (which can re-enter).
	bConsumed = true;
	if (!Stats->AddCoins(10))
	{
		bConsumed = false;
		return;
	}
	PickupSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	const FVector EffectLocation = GetActorLocation();
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ACH4_2PlayerController* Recipient = Cast<ACH4_2PlayerController>(It->Get()))
		{
			if (IsValid(Recipient) && !Recipient->IsActorBeingDestroyed())
			{
				Recipient->ClientPlayPickupEffects(EffectLocation, PickupEffect, PickupSound);
			}
		}
	}
	UE_LOG(LogCH4_2, Log, TEXT("Coin collected: actor=%s player=%d reward=10"), *GetName(), Stats->GetPlayerId());
	Destroy();
}
