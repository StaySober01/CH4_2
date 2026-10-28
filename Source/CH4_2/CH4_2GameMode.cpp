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
	bool bParticipantLeft = false;
	if (APlayerController* ExitingPlayer = Cast<APlayerController>(Exiting))
	{
		bParticipantLeft = ConnectedControllers.Remove(TWeakObjectPtr<APlayerController>(ExitingPlayer)) > 0;
	}
	Super::Logout(Exiting);
	if (HasAuthority())
	{
		if (bParticipantLeft && MatchSnapshot.Phase == ECH4_2MatchPhase::Playing
			&& GetWorld() && !GetWorld()->bIsTearingDown)
		{
			FinalizeMatch(true);
		}
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
			FinalizeMatch(false);
		}
	}
	PublishMatchSnapshot();
}

void ACH4_2GameMode::FinalizeMatch(bool bPlayerLeft)
{
	if (!HasAuthority() || MatchSnapshot.Phase != ECH4_2MatchPhase::Playing || MatchSnapshot.bResultFinalized)
	{
		return;
	}
	GetWorldTimerManager().ClearTimer(MatchTimer);
	MatchSnapshot.Phase = ECH4_2MatchPhase::Finished;
	MatchSnapshot.RemainingSeconds = 0;
	MatchSnapshot.bCountdownActive = false;
	MatchSnapshot.EndReason = bPlayerLeft ? ECH4_2MatchEndReason::PlayerLeft : ECH4_2MatchEndReason::TimeExpired;
	MatchSnapshot.ResultParticipantIds.Reset();
	MatchSnapshot.Winners.Reset();
	int32 HighestCoins = -1;
	for (const TWeakObjectPtr<APlayerController>& Entry : ConnectedControllers)
	{
		const APlayerController* PC = Entry.Get();
		const ACH4_2PlayerState* Stats = IsValid(PC) ? PC->GetPlayerState<ACH4_2PlayerState>() : nullptr;
		if (!IsValid(PC) || PC->IsActorBeingDestroyed() || !IsValid(Stats) || Stats->IsActorBeingDestroyed()
			|| Stats->IsOnlyASpectator() || Stats->GetOwner() != PC)
		{
			continue;
		}
		MatchSnapshot.ResultParticipantIds.AddUnique(Stats->GetPlayerId());
		const int32 Coins = Stats->GetCoins();
		if (!bPlayerLeft && Coins < HighestCoins)
		{
			continue;
		}
		if (!bPlayerLeft && Coins > HighestCoins)
		{
			HighestCoins = Coins;
			MatchSnapshot.Winners.Reset();
		}
		FCH4_2Winner Winner;
		Winner.PlayerId = Stats->GetPlayerId();
		Winner.PlayerName = Stats->GetPlayerName().Left(64);
		for (TCHAR& Character : Winner.PlayerName)
		{
			if (Character < TEXT(' ') || Character == 127 || Character == 0x2028 || Character == 0x2029)
			{
				Character = TEXT(' ');
			}
		}
		Winner.PlayerName.TrimStartAndEndInline();
		if (Winner.PlayerName.IsEmpty())
		{
			Winner.PlayerName = TEXT("Player");
		}
		MatchSnapshot.Winners.Add(MoveTemp(Winner));
	}
	MatchSnapshot.ResultParticipantIds.Sort();
	MatchSnapshot.Winners.Sort([](const FCH4_2Winner& A, const FCH4_2Winner& B) { return A.PlayerId < B.PlayerId; });
	MatchSnapshot.bResultFinalized = true;
	UE_LOG(LogCH4_2, Log, TEXT("Match result finalized: Reason=%s Participants=%d Winners=%d"),
		*UEnum::GetValueAsString(MatchSnapshot.EndReason), MatchSnapshot.ResultParticipantIds.Num(), MatchSnapshot.Winners.Num());
	for (const FCH4_2Winner& Winner : MatchSnapshot.Winners)
	{
		UE_LOG(LogCH4_2, Log, TEXT("Match winner: PlayerId=%d Name=%s"), Winner.PlayerId, *Winner.PlayerName);
	}
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
