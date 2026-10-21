// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CH4_2GameMode.generated.h"

class APlayerStart;

UCLASS(abstract)
class ACH4_2GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACH4_2GameMode();

protected:
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	UPROPERTY(EditDefaultsOnly, Category="Vehicle|Spawn")
	FVector VehicleSpawnCheckHalfExtent = FVector(250.0, 120.0, 80.0);

	UPROPERTY(EditDefaultsOnly, Category="Vehicle|Spawn")
	FVector VehicleSpawnCheckOffset = FVector(0.0, 0.0, 80.0);

private:
	bool IsPlayerStartAvailable(const APlayerStart* PlayerStart) const;
};



