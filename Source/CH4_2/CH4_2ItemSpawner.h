#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "CH4_2ItemSpawner.generated.h"

class USceneComponent;

UCLASS()
class CH4_2_API ACH4_2ItemSpawner : public AActor
{
	GENERATED_BODY()

public:
	ACH4_2ItemSpawner();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Spawner")
	TObjectPtr<USceneComponent> SpawnPoint;

	/** Select BP_CoinPickup or BP_SpeedPickup. Validated on the server. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawner")
	TSubclassOf<AActor> ItemClass;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category="Spawner")
	TObjectPtr<AActor> SpawnedItem;

private:
	UPROPERTY(Transient)
	TSubclassOf<AActor> ActiveItemClass;

	FTimerHandle RespawnTimer;
	bool bStopping = false;
	static constexpr float RespawnDelay = 2.0f;

	void SpawnItem();
	void ScheduleRespawn();
	void UnbindItem(AActor* Item);

	UFUNCTION()
	void OnItemDestroyed(AActor* DestroyedActor);

	UFUNCTION()
	void OnItemEndPlay(AActor* Actor, EEndPlayReason::Type EndPlayReason);
};
