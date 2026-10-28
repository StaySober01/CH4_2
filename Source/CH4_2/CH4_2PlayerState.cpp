#include "CH4_2PlayerState.h"
#include "Net/UnrealNetwork.h"

void ACH4_2PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(ACH4_2PlayerState, Coins, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ACH4_2PlayerState, BonusMaxSpeed, COND_OwnerOnly);
}

bool ACH4_2PlayerState::AddCoins(int32 Amount)
{
	if (!HasAuthority() || Amount <= 0 || Amount > MAX_int32 - Coins)
	{
		return false;
	}
	Coins += Amount;
	ForceNetUpdate();
	NotifyStatsChanged();
	return true;
}

bool ACH4_2PlayerState::TrySpendCoins(int32 Amount)
{
	if (!HasAuthority() || Amount <= 0 || Amount > Coins)
	{
		return false;
	}
	Coins -= Amount;
	ForceNetUpdate();
	NotifyStatsChanged();
	return true;
}

bool ACH4_2PlayerState::TryTransferCoinsTo(ACH4_2PlayerState* Recipient, int32 Amount)
{
	if (!HasAuthority() || IsActorBeingDestroyed() || !IsValid(Recipient) || Recipient == this
		|| !Recipient->HasAuthority() || Recipient->IsActorBeingDestroyed() || GetWorld() != Recipient->GetWorld()
		|| Amount <= 0 || Coins < Amount || Amount > MAX_int32 - Recipient->Coins)
	{
		return false;
	}
	Coins -= Amount;
	Recipient->Coins += Amount;
	ForceNetUpdate();
	Recipient->ForceNetUpdate();
	NotifyStatsChanged();
	if (IsValid(Recipient) && !Recipient->IsActorBeingDestroyed())
	{
		Recipient->NotifyStatsChanged();
	}
	return true;
}

bool ACH4_2PlayerState::IncreaseBonusMaxSpeed(float Amount)
{
	if (!HasAuthority() || !FMath::IsFinite(Amount) || Amount <= 0.0f)
	{
		return false;
	}
	const float NewValue = BonusMaxSpeed + Amount;
	if (!FMath::IsFinite(NewValue) || NewValue <= BonusMaxSpeed)
	{
		return false;
	}
	BonusMaxSpeed = NewValue;
	ForceNetUpdate();
	NotifyStatsChanged();
	return true;
}

void ACH4_2PlayerState::OnRep_Coins()
{
	NotifyStatsChanged();
}

void ACH4_2PlayerState::OnRep_PlayerId()
{
	Super::OnRep_PlayerId();
	NotifyStatsChanged(); // Re-evaluate local results if identity arrives after GameState.
}

void ACH4_2PlayerState::OnRep_BonusMaxSpeed()
{
	NotifyStatsChanged();
}

void ACH4_2PlayerState::NotifyStatsChanged()
{
	// Server setters call this too: a listen host does not receive its own OnRep.
	OnStatsChanged.Broadcast();
}
