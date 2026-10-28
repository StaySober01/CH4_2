#include "CH4_2PlayerStatsWidget.h"




#include "Components/TextBlock.h"


#define LOCTEXT_NAMESPACE "CH4_2PlayerStats"

void UCH4_2PlayerStatsWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SetVisibility(ESlateVisibility::HitTestInvisible);
    RefreshText();
    RefreshMatchText();
}

void UCH4_2PlayerStatsWidget::SetMatchSnapshot(const FCH4_2MatchSnapshot& Snapshot)
{
	bHasGameState = true;
	DisplayMatch = Snapshot;
	RefreshMatchText();
}

void UCH4_2PlayerStatsWidget::SetWaitingForGameState()
{
	bHasGameState = false;
	RefreshMatchText();
}

void UCH4_2PlayerStatsWidget::SetLocalPlayerId(int32 PlayerId)
{
	LocalPlayerId = PlayerId;
	RefreshMatchText();
}

void UCH4_2PlayerStatsWidget::RefreshMatchText()
{
	if (!MatchText)
	{
		return;
	}
	if (!bHasGameState)
	{
		MatchText->SetText(LOCTEXT("LoadingMatch", "Waiting for match data..."));
		return;
	}
	if (DisplayMatch.Phase == ECH4_2MatchPhase::Waiting && !DisplayMatch.bCountdownActive)
	{
		MatchText->SetText(FText::Format(LOCTEXT("WaitingPlayers", "Waiting for players: {0}/2"), FText::AsNumber(DisplayMatch.ConnectedPlayers)));
		return;
	}
	if (DisplayMatch.Phase == ECH4_2MatchPhase::Finished)
	{
		if (!DisplayMatch.bResultFinalized)
		{
			MatchText->SetText(LOCTEXT("ResultPending", "Finished | Waiting for result..."));
			return;
		}
		if (DisplayMatch.ResultParticipantIds.IsEmpty())
		{
			MatchText->SetText(LOCTEXT("NoParticipants", "Finished | No eligible participants"));
			return;
		}
		TArray<FString> Names;
		bool bLocalWinner = false;
		for (const FCH4_2Winner& Winner : DisplayMatch.Winners)
		{
			Names.Add(FString::Printf(TEXT("%s (#%d)"), *Winner.PlayerName, Winner.PlayerId));
			bLocalWinner |= Winner.PlayerId == LocalPlayerId;
		}
		FText Outcome;
		if (LocalPlayerId == INDEX_NONE)
		{
			Outcome = LOCTEXT("ResultIdentityPending", "Waiting for player data...");
		}
		else if (!DisplayMatch.ResultParticipantIds.Contains(LocalPlayerId))
		{
			Outcome = LOCTEXT("NotParticipant", "Not a participant in this result");
		}
		else if (bLocalWinner)
		{
			Outcome = DisplayMatch.Winners.Num() > 1
				? LOCTEXT("JointVictory", "Joint Victory!") : LOCTEXT("Victory", "Victory!");
		}
		else
		{
			Outcome = LOCTEXT("Defeat", "Defeat");
		}
		const FText Reason = DisplayMatch.EndReason == ECH4_2MatchEndReason::PlayerLeft
			? LOCTEXT("PlayerLeftReason", "Player left") : LOCTEXT("TimeExpiredReason", "Time expired");
		MatchText->SetText(FText::Format(LOCTEXT("MatchResult", "Finished | {0}\n{1}\nWinners: {2}"),
			Outcome, Reason, FText::FromString(FString::Join(Names, TEXT(", ")))));
		return;
	}
	FText Phase;
	switch (DisplayMatch.Phase)
	{
	case ECH4_2MatchPhase::Waiting:
		Phase = LOCTEXT("WaitingPhase", "Waiting");
		break;
	case ECH4_2MatchPhase::Playing:
		Phase = LOCTEXT("PlayingPhase", "Playing");
		break;
	default:
		Phase = LOCTEXT("FinishedPhase", "Finished");
		break;
	}
	MatchText->SetText(FText::Format(LOCTEXT("MatchClock", "{0}: {1}s"), Phase, FText::AsNumber(DisplayMatch.RemainingSeconds)));
}

void UCH4_2PlayerStatsWidget::SetStats(int32 NewCoins, float NewBonusMaxSpeed)
{
	bHasPlayerState = true;
	DisplayCoins = NewCoins;
	DisplayBonusMaxSpeed = NewBonusMaxSpeed;
	RefreshText();
}

void UCH4_2PlayerStatsWidget::SetWaitingForPlayerState()
{
	bHasPlayerState = false;
	LocalPlayerId = INDEX_NONE;
	RefreshText();
	RefreshMatchText();
}

void UCH4_2PlayerStatsWidget::RefreshText()
{
	if (!CoinsText || !BonusSpeedText)
	{
		return;
	}
	if (!bHasPlayerState)
	{
		CoinsText->SetText(LOCTEXT("Waiting", "Waiting for player data..."));
		BonusSpeedText->SetText(FText::GetEmpty());
		return;
	}
	CoinsText->SetText(FText::Format(LOCTEXT("Coins", "Coins: {0}"), FText::AsNumber(DisplayCoins)));
	BonusSpeedText->SetText(FText::Format(LOCTEXT("BonusKmh", "Bonus max speed: {0} km/h"), FText::AsNumber(DisplayBonusMaxSpeed)));
}

#undef LOCTEXT_NAMESPACE

