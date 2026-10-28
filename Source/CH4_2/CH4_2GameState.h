#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CH4_2GameState.generated.h"

UENUM(BlueprintType)
enum class ECH4_2MatchPhase : uint8
{
	Waiting,
	Playing,
	Finished
};

USTRUCT(BlueprintType)
struct FCH4_2Winner
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 PlayerId = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly)
	FString PlayerName;
};

UENUM(BlueprintType)
enum class ECH4_2MatchEndReason : uint8
{
	None,
	TimeExpired,
	PlayerLeft
};

USTRUCT(BlueprintType)
struct FCH4_2MatchSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	ECH4_2MatchPhase Phase = ECH4_2MatchPhase::Waiting;

	UPROPERTY(BlueprintReadOnly)
	int32 RemainingSeconds = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 ConnectedPlayers = 0;

	UPROPERTY(BlueprintReadOnly)
	bool bCountdownActive = false;

	UPROPERTY(BlueprintReadOnly)
	bool bResultFinalized = false;

	UPROPERTY(BlueprintReadOnly)
	ECH4_2MatchEndReason EndReason = ECH4_2MatchEndReason::None;

	UPROPERTY(BlueprintReadOnly)
	TArray<int32> ResultParticipantIds;

	/** IDs and names survive player departure; individual coin balances stay private. */
	UPROPERTY(BlueprintReadOnly)
	TArray<FCH4_2Winner> Winners;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCH4_2MatchChanged);

UCLASS()
class CH4_2_API ACH4_2GameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	const FCH4_2MatchSnapshot& GetMatchSnapshot() const { return MatchSnapshot; }

	UPROPERTY(BlueprintAssignable, Category="Match")
	FCH4_2MatchChanged OnMatchChanged;

private:
	friend class ACH4_2GameMode;
	void SetMatchSnapshot(const FCH4_2MatchSnapshot& NewSnapshot);

	UPROPERTY(ReplicatedUsing=OnRep_MatchSnapshot)
	FCH4_2MatchSnapshot MatchSnapshot;

	UFUNCTION()
	void OnRep_MatchSnapshot();
};
