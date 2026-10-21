#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CH4_2GameState.h"
#include "CH4_2PlayerStatsWidget.generated.h"

class UTextBlock;

/** Data presentation for WBP_PlayerStats; layout belongs to the WBP Designer. */
UCLASS(Abstract)
class CH4_2_API UCH4_2PlayerStatsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetStats(int32 NewCoins, float NewBonusMaxSpeed);
	void SetWaitingForPlayerState();
	void SetMatchSnapshot(const FCH4_2MatchSnapshot& Snapshot);
	void SetWaitingForGameState();

protected:
	virtual void NativeConstruct() override;

private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> CoinsText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> BonusSpeedText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> MatchText;

	FCH4_2MatchSnapshot DisplayMatch;
	bool bHasGameState = false;
	void RefreshMatchText();

	int32 DisplayCoins = 0;
	float DisplayBonusMaxSpeed = 0.0f;
	bool bHasPlayerState = false;
	void RefreshText();
};
