// Copyright Epic Games, Inc. All Rights Reserved.

#include "CH4_2GameMode.h"
#include "CH4_2PlayerController.h"
#include "CH4_2PlayerState.h"
#include "CH4_2.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "TimerManager.h"

ACH4_2GameMode::ACH4_2GameMode()
{
	PlayerControllerClass = ACH4_2PlayerController::StaticClass();
	PlayerStateClass = ACH4_2PlayerState::StaticClass();
	GameStateClass = ACH4_2GameState::StaticClass();
}

void ACH4_2GameMode::InitGameState()
{
	Super::InitGameState();
	PublishMatchSnapshot();
}

void ACH4_2GameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	if (HasAuthority() && IsValid(NewPlayer))
	{
		const APlayerState* NewState = NewPlayer->GetPlayerState<APlayerState>();
		if (NewState && !NewState->IsOnlyASpectator())
		{
			ConnectedControllers.Add(TWeakObjectPtr<APlayerController>(NewPlayer));
			UpdateConnectedPlayers();
		}
	}
}

void ACH4_2GameMode::Logout(AController* Exiting)
{
	if (APlayerController* ExitingPlayer = Cast<APlayerController>(Exiting))
	{
		ConnectedControllers.Remove(TWeakObjectPtr<APlayerController>(ExitingPlayer));
	}
	Super::Logout(Exiting);
	if (HasAuthority())
	{
		UpdateConnectedPlayers();
	}
}

void ACH4_2GameMode::UpdateConnectedPlayers()
{
	for (auto It = ConnectedControllers.CreateIterator(); It; ++It)
	{
		if (!It->IsValid())
		{
			It.RemoveCurrent();
		}
	}
	MatchSnapshot.ConnectedPlayers = ConnectedControllers.Num();
	if (MatchSnapshot.Phase == ECH4_2MatchPhase::Waiting)
	{
		if (MatchSnapshot.ConnectedPlayers < RequiredPlayers)
		{
			GetWorldTimerManager().ClearTimer(MatchTimer);
			MatchSnapshot.bCountdownActive = false;
			MatchSnapshot.RemainingSeconds = 0;
		}
		else if (!MatchSnapshot.bCountdownActive)
		{
			MatchSnapshot.bCountdownActive = true;
			MatchSnapshot.RemainingSeconds = WaitingSeconds;
			GetWorldTimerManager().SetTimer(MatchTimer, this, &ACH4_2GameMode::AdvanceMatchClock, 1.0f, true);
		}
	}
	PublishMatchSnapshot();
}

void ACH4_2GameMode::AdvanceMatchClock()
{
	if (!HasAuthority() || !MatchSnapshot.bCountdownActive)
	{
		return;
	}
	MatchSnapshot.RemainingSeconds = FMath::Max(0, MatchSnapshot.RemainingSeconds - 1);
	if (MatchSnapshot.RemainingSeconds == 0)
	{
		if (MatchSnapshot.Phase == ECH4_2MatchPhase::Waiting)
		{
			MatchSnapshot.Phase = ECH4_2MatchPhase::Playing;
			MatchSnapshot.RemainingSeconds = PlayingSeconds;
		}
		else
		{
			MatchSnapshot.Phase = ECH4_2MatchPhase::Finished;
			MatchSnapshot.bCountdownActive = false;
			GetWorldTimerManager().ClearTimer(MatchTimer);
		}
	}
	PublishMatchSnapshot();
}

void ACH4_2GameMode::PublishMatchSnapshot()
{
	if (ACH4_2GameState* MatchState = Cast<ACH4_2GameState>(GameState))
	{
		MatchState->SetMatchSnapshot(MatchSnapshot);
		UE_LOG(LogCH4_2, Log, TEXT("Match Phase=%s Remaining=%d Players=%d Countdown=%d"),
			*UEnum::GetValueAsString(MatchSnapshot.Phase), MatchSnapshot.RemainingSeconds,
			MatchSnapshot.ConnectedPlayers, MatchSnapshot.bCountdownActive);
	}
	else
	{
		UE_LOG(LogCH4_2, Error, TEXT("Match requires CH4_2GameState. Check GameMode GameStateClass."));
	}
}

void ACH4_2GameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(MatchTimer);
	ConnectedControllers.Empty();
	Super::EndPlay(EndPlayReason);
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
