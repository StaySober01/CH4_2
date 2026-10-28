// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CH4_2GameState.h"
#include "TimerManager.h"
#include "CH4_2GameMode.generated.h"

class APlayerStart;

UCLASS(abstract)
class ACH4_2GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACH4_2GameMode();
	virtual void InitGameState() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	UPROPERTY(EditDefaultsOnly, Category="Vehicle|Spawn")
	FVector VehicleSpawnCheckHalfExtent = FVector(250.0, 120.0, 80.0);

	UPROPERTY(EditDefaultsOnly, Category="Vehicle|Spawn")
	FVector VehicleSpawnCheckOffset = FVector(0.0, 0.0, 80.0);

private:
	static constexpr int32 RequiredPlayers = 2;
	static constexpr int32 WaitingSeconds = 5;
	static constexpr int32 PlayingSeconds = 60;
	TSet<TWeakObjectPtr<APlayerController>> ConnectedControllers;
	FCH4_2MatchSnapshot MatchSnapshot;
	FTimerHandle MatchTimer;
	void UpdateConnectedPlayers();
	void AdvanceMatchClock();
	void PublishMatchSnapshot();
	void FinalizeMatch(bool bPlayerLeft);

	bool IsPlayerStartAvailable(const APlayerStart* PlayerStart) const;
};



