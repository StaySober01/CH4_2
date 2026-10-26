#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CH4_2CoinPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UPrimitiveComponent;
class UParticleSystem;
class USoundBase;

/** One server-owned coin. Visual assets are configured in a Blueprint child. */
UCLASS()
class CH4_2_API ACH4_2CoinPickup : public AActor
{
	GENERATED_BODY()

public:
	ACH4_2CoinPickup();
	/** Also used after server possession when the car was already inside this trigger. */
	void TryCollect(AActor* OtherActor);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Coin")
	TObjectPtr<USphereComponent> PickupSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Coin")
	TObjectPtr<UStaticMeshComponent> CoinMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Coin|Effects")
	TObjectPtr<UParticleSystem> PickupEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Coin|Effects")
	TObjectPtr<USoundBase> PickupSound;

private:
	bool bConsumed = false;

	UFUNCTION()
	void OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

};
