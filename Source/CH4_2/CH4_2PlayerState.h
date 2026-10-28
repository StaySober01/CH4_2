#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CH4_2PlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCH4_2PlayerStatsChanged);

/** Server-owned player values, replicated only to this player's connection. */
UCLASS()
class CH4_2_API ACH4_2PlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_PlayerId() override;

	UFUNCTION(BlueprintPure, Category="Player Stats")
	int32 GetCoins() const { return Coins; }

	UFUNCTION(BlueprintPure, Category="Player Stats")
	float GetBonusMaxSpeed() const { return BonusMaxSpeed; }

	/** Returns false without changing state for non-authority or invalid amounts. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Player Stats")
	bool AddCoins(int32 Amount);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Player Stats")
	bool TrySpendCoins(int32 Amount);

	/** Server-only transaction: update both balances before broadcasting either UI event. */
	bool TryTransferCoinsTo(ACH4_2PlayerState* Recipient, int32 Amount);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Player Stats")
	bool IncreaseBonusMaxSpeed(float Amount);

	UPROPERTY(BlueprintAssignable, Category="Player Stats")
	FCH4_2PlayerStatsChanged OnStatsChanged;

private:
	UPROPERTY(ReplicatedUsing=OnRep_Coins)
	int32 Coins = 0;

	UPROPERTY(ReplicatedUsing=OnRep_BonusMaxSpeed)
	float BonusMaxSpeed = 0.0f;

	UFUNCTION()
	void OnRep_Coins();

	UFUNCTION()
	void OnRep_BonusMaxSpeed();

	void NotifyStatsChanged();
};
