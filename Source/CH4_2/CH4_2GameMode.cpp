// Copyright Epic Games, Inc. All Rights Reserved.

#include "CH4_2GameMode.h"
#include "CH4_2PlayerController.h"
#include "CH4_2.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"

ACH4_2GameMode::ACH4_2GameMode()
{
	PlayerControllerClass = ACH4_2PlayerController::StaticClass();
}

AActor* ACH4_2GameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	TArray<APlayerStart*> AvailableStarts;
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		if (IsPlayerStartAvailable(*It))
		{
			AvailableStarts.Add(*It);
		}
	}

	if (AvailableStarts.IsEmpty())
	{
		UE_LOG(LogCH4_2, Warning, TEXT("No unoccupied vehicle PlayerStart is available."));
		return nullptr;
	}
	return AvailableStarts[FMath::RandHelper(AvailableStarts.Num())];
}

bool ACH4_2GameMode::IsPlayerStartAvailable(const APlayerStart* PlayerStart) const
{
	if (!IsValid(PlayerStart) || !IsValid(GetWorld()))
	{
		return false;
	}
	const FTransform Transform = PlayerStart->GetActorTransform();
	FCollisionObjectQueryParams Objects;
	Objects.AddObjectTypesToQuery(ECC_Vehicle);
	const FCollisionQueryParams Query(SCENE_QUERY_STAT(VehiclePlayerStartCheck), false);
	return !GetWorld()->OverlapAnyTestByObjectType(
		Transform.TransformPosition(VehicleSpawnCheckOffset), Transform.GetRotation(),
		Objects, FCollisionShape::MakeBox(VehicleSpawnCheckHalfExtent), Query);
}
