#include "CH4_2PlayerStatsWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"

#define LOCTEXT_NAMESPACE "CH4_2PlayerStats"

TSharedRef<SWidget> UCH4_2PlayerStatsWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("StatsRoot"));
		WidgetTree->RootWidget = Root;
		UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("StatsPanel"));
		Panel->SetBrushColor(FLinearColor(0.02f, 0.02f, 0.02f, 0.8f));
		Panel->SetPadding(FMargin(16.0f));
		UCanvasPanelSlot* PanelSlot = Root->AddChildToCanvas(Panel);
		PanelSlot->SetPosition(FVector2D(24.0f, 24.0f));
		PanelSlot->SetAutoSize(true);

		UVerticalBox* Lines = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("StatsLines"));
		Panel->SetContent(Lines);
		CoinsText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CoinsText"));
		BonusSpeedText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("BonusSpeedText"));
		Lines->AddChildToVerticalBox(CoinsText);
		Lines->AddChildToVerticalBox(BonusSpeedText);
		MatchText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("MatchText"));
		Lines->AddChildToVerticalBox(MatchText);
	}
	SetVisibility(ESlateVisibility::HitTestInvisible);
	RefreshText();
	RefreshMatchText();
	return Super::RebuildWidget();
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
	RefreshText();
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
	BonusSpeedText->SetText(FText::Format(LOCTEXT("Bonus", "Bonus max speed: {0}"), FText::AsNumber(DisplayBonusMaxSpeed)));
}

#undef LOCTEXT_NAMESPACE
