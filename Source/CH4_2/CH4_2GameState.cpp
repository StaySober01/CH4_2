#include "CH4_2GameState.h"
#include "Net/UnrealNetwork.h"

void ACH4_2GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACH4_2GameState, MatchSnapshot);
}

void ACH4_2GameState::SetMatchSnapshot(const FCH4_2MatchSnapshot& NewSnapshot)
{
	if (!HasAuthority())
	{
		return;
	}
	MatchSnapshot = NewSnapshot;
	ForceNetUpdate();
	OnMatchChanged.Broadcast(); // Listen host does not receive its own OnRep.
}

void ACH4_2GameState::OnRep_MatchSnapshot()
{
	OnMatchChanged.Broadcast();
}
