#include "CH4_2ItemSpawner.h"
#include "CH4_2.h"
#include "CH4_2CoinPickup.h"
#include "CH4_2SpeedPickup.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

ACH4_2ItemSpawner::ACH4_2ItemSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;
	bNetLoadOnClient = false;
	SpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnPoint"));
	SetRootComponent(SpawnPoint);
}

void ACH4_2ItemSpawner::BeginPlay()
{
	Super::BeginPlay();
	bStopping = false;
	if (!HasAuthority())
	{
		return;
	}
	UClass* SelectedClass = ItemClass.Get();
	if (!SelectedClass || SelectedClass->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated)
		|| (!SelectedClass->IsChildOf(ACH4_2CoinPickup::StaticClass())
			&& !SelectedClass->IsChildOf(ACH4_2SpeedPickup::StaticClass()))
		|| !SelectedClass->GetDefaultObject<AActor>()->GetIsReplicated())
	{
		UE_LOG(LogCH4_2, Error, TEXT("Spawner %s: ItemClass must be a replicated coin or speed pickup Blueprint."), *GetName());
		return;
	}
	// Keep the selected type fixed for this spawner's play lifetime.
	ActiveItemClass = SelectedClass;
	SpawnItem();
}

void ACH4_2ItemSpawner::SpawnItem()
{
	UWorld* World = GetWorld();
	if (!HasAuthority() || bStopping || !World || World->bIsTearingDown
		|| IsActorBeingDestroyed() || !ActiveItemClass || IsValid(SpawnedItem))
	{
		return;
	}
	World->GetTimerManager().ClearTimer(RespawnTimer);
	const FTransform SpawnTransform = SpawnPoint->GetComponentTransform();
	AActor* Item = World->SpawnActorDeferred<AActor>(ActiveItemClass, SpawnTransform, this, nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!IsValid(Item))
	{
		UE_LOG(LogCH4_2, Warning, TEXT("Spawner %s: spawn failed; retrying in 2 seconds."), *GetName());
		ScheduleRespawn();
		return;
	}
	SpawnedItem = Item;
	Item->OnDestroyed.AddUniqueDynamic(this, &ACH4_2ItemSpawner::OnItemDestroyed);
	Item->OnEndPlay.AddUniqueDynamic(this, &ACH4_2ItemSpawner::OnItemEndPlay);
	UE_LOG(LogCH4_2, Log, TEXT("Spawner %s: spawning %s"), *GetName(), *Item->GetName());
	// BeginPlay can collect and destroy the item here. Do not overwrite the reference afterwards.
	Item->FinishSpawning(SpawnTransform);
}

void ACH4_2ItemSpawner::ScheduleRespawn()
{
	UWorld* World = GetWorld();
	if (!HasAuthority() || bStopping || !World || World->bIsTearingDown || IsActorBeingDestroyed())
	{
		return;
	}
	if (!World->GetTimerManager().IsTimerActive(RespawnTimer))
	{
		World->GetTimerManager().SetTimer(RespawnTimer, this, &ACH4_2ItemSpawner::SpawnItem, RespawnDelay, false);
	}
}

void ACH4_2ItemSpawner::UnbindItem(AActor* Item)
{
	if (Item)
	{
		Item->OnDestroyed.RemoveDynamic(this, &ACH4_2ItemSpawner::OnItemDestroyed);
		Item->OnEndPlay.RemoveDynamic(this, &ACH4_2ItemSpawner::OnItemEndPlay);
	}
}

void ACH4_2ItemSpawner::OnItemDestroyed(AActor* DestroyedActor)
{
	if (DestroyedActor != SpawnedItem)
	{
		return;
	}
	UnbindItem(DestroyedActor);
	SpawnedItem = nullptr;
	ScheduleRespawn();
}

void ACH4_2ItemSpawner::OnItemEndPlay(AActor* Actor, EEndPlayReason::Type EndPlayReason)
{
	if (Actor != SpawnedItem || EndPlayReason == EEndPlayReason::Destroyed)
	{
		return;
	}
	// Streaming removal and world shutdown are not pickup consumption.
	UnbindItem(Actor);
	SpawnedItem = nullptr;
	bStopping = true;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RespawnTimer);
	}
}

void ACH4_2ItemSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	bStopping = true;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RespawnTimer);
	}
	AActor* Item = SpawnedItem;
	UnbindItem(Item);
	SpawnedItem = nullptr;
	if (HasAuthority() && IsValid(Item) && !Item->IsActorBeingDestroyed())
	{
		Item->Destroy();
	}
	Super::EndPlay(EndPlayReason);
}
