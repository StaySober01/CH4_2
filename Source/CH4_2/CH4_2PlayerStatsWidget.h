#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CH4_2GameState.h"
#include "CH4_2PlayerStatsWidget.generated.h"

class UTextBlock;

/** Native, local-only panel; the template's speed/gear widget stays unchanged. */
UCLASS()
class CH4_2_API UCH4_2PlayerStatsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetStats(int32 NewCoins, float NewBonusMaxSpeed);
	void SetWaitingForPlayerState();
	void SetMatchSnapshot(const FCH4_2MatchSnapshot& Snapshot);
	void SetWaitingForGameState();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> CoinsText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> BonusSpeedText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> MatchText;

	FCH4_2MatchSnapshot DisplayMatch;
	bool bHasGameState = false;
	void RefreshMatchText();

	int32 DisplayCoins = 0;
	float DisplayBonusMaxSpeed = 0.0f;
	bool bHasPlayerState = false;
	void RefreshText();
};
