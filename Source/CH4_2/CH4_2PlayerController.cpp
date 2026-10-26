// Copyright Epic Games, Inc. All Rights Reserved.


#include "CH4_2PlayerController.h"
#include "CH4_2Pawn.h"
#include "CH4_2UI.h"
#include "CH4_2PlayerState.h"
#include "CH4_2PlayerStatsWidget.h"
#include "CH4_2GameState.h"
#include "CH4_2ChatWidget.h"
#include "CH4_2CoinPickup.h"
#include "Components/InputComponent.h"
#include "HAL/PlatformTime.h"
#include "InputCoreTypes.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "EnhancedInputSubsystems.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Blueprint/UserWidget.h"
#include "CH4_2.h"
#include "Widgets/Input/SVirtualJoystick.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"

void ACH4_2PlayerController::ClientPlayPickupEffects_Implementation(FVector Location, UParticleSystem* Effect, USoundBase* Sound)
{
	if (!IsLocalController() || GetNetMode() == NM_DedicatedServer)
	{
		return;
	}
	if (IsValid(Effect))
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, Effect, Location);
	}
	if (IsValid(Sound))
	{
		UGameplayStatics::PlaySoundAtLocation(this, Sound, Location);
	}
}

namespace
{
	FString ChatSingleLine(const FString& Input)
	{
		FString Result = Input;
		for (TCHAR& Character : Result)
		{
			if (Character < TEXT(' ') || Character == 127 || Character == 0x2028 || Character == 0x2029)
			{
				Character = TEXT(' ');
			}
		}
		return Result.TrimStartAndEnd();
	}
}

void ACH4_2PlayerController::OpenChat()
{
	if (!IsLocalPlayerController() || bChatOpen || !ChatWidget || !ChatWidget->BeginChat())
	{
		return;
	}
	bChatOpen = true;
	bCursorVisibleBeforeChat = bShowMouseCursor;
	FlushPressedKeys();
	// UI-only focus does not clear inputs already stored by Chaos movement.
	if (ACH4_2Pawn* CurrentVehicle = Cast<ACH4_2Pawn>(GetPawn()))
	{
		CurrentVehicle->DoThrottle(0.0f);
		CurrentVehicle->DoSteering(0.0f);
		CurrentVehicle->DoBrakeStop();
		CurrentVehicle->DoHandbrakeStop();
	}
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(ChatWidget->GetInputFocusTarget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	bShowMouseCursor = true;
}

void ACH4_2PlayerController::CloseChat()
{
	if (!IsLocalPlayerController() || !bChatOpen)
	{
		return;
	}
	bChatOpen = false;
	if (ChatWidget)
	{
		ChatWidget->EndChat();
	}
	FlushPressedKeys();
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = bCursorVisibleBeforeChat;
}

void ACH4_2PlayerController::SubmitChatMessage(const FString& Message)
{
	if (!IsLocalPlayerController())
	{
		return;
	}
	if (Message.Len() > MaxChatLength)
	{
		if (ChatWidget)
		{
			ChatWidget->SetFeedback(NSLOCTEXT("CH4_2Chat", "TooLong", "Message limit: 200 characters."));
		}
		return;
	}
	const FString CleanMessage = ChatSingleLine(Message);
	if (CleanMessage.IsEmpty())
	{
		return;
	}
	if (ChatWidget)
	{
		ChatWidget->SetFeedback(FText::GetEmpty());
	}
	ServerSubmitChat(CleanMessage);
}

void ACH4_2PlayerController::ServerSubmitChat_Implementation(const FString& Message)
{
	const APlayerState* SenderState = GetPlayerState<APlayerState>();
	if (!HasAuthority() || !GetWorld() || !IsValid(SenderState) || SenderState->GetOwner() != this)
	{
		return;
	}
	const double Now = FPlatformTime::Seconds();
	if (Now < NextChatAttemptTime)
	{
		if (!bRateLimitNoticeSent)
		{
			ClientChatRejected(NSLOCTEXT("CH4_2Chat", "TooFast", "Please wait 0.5 seconds between messages."));
			bRateLimitNoticeSent = true;
		}
		return;
	}
	NextChatAttemptTime = Now + ChatIntervalSeconds;
	bRateLimitNoticeSent = false;
	if (Message.Len() > MaxChatLength)
	{
		ClientChatRejected(NSLOCTEXT("CH4_2Chat", "TooLong", "Message limit: 200 characters."));
		return;
	}
	const FString CleanMessage = ChatSingleLine(Message);
	if (CleanMessage.IsEmpty())
	{
		return;
	}
	FString SenderName = ChatSingleLine(SenderState->GetPlayerName()).Left(64);
	if (SenderName.IsEmpty())
	{
		SenderName = FString::Printf(TEXT("Player %d"), SenderState->GetPlayerId());
	}
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ACH4_2PlayerController* Recipient = Cast<ACH4_2PlayerController>(It->Get()))
		{
			Recipient->ClientReceiveChat(SenderName, CleanMessage);
		}
	}
}

void ACH4_2PlayerController::ClientReceiveChat_Implementation(const FString& SenderName, const FString& Message)
{
	if (!IsLocalPlayerController())
	{
		return;
	}
	ChatMessages.Add(FText::Format(NSLOCTEXT("CH4_2Chat", "Message", "{0}: {1}"),
		FText::FromString(SenderName), FText::FromString(Message)));
	if (ChatMessages.Num() > MaxChatHistory)
	{
		ChatMessages.RemoveAt(0, ChatMessages.Num() - MaxChatHistory);
	}
	if (ChatWidget)
	{
		ChatWidget->SetHistory(ChatMessages);
	}
}

void ACH4_2PlayerController::ClientChatRejected_Implementation(const FText& Reason)
{
	if (IsLocalPlayerController() && ChatWidget)
	{
		ChatWidget->SetFeedback(Reason);
	}
}

void ACH4_2PlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	// only spawn UI on local player controllers
	if (IsLocalPlayerController())
	{
		if (SVirtualJoystick::ShouldDisplayTouchInterface())
		{
			// spawn the mobile controls widget
			MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

			if (MobileControlsWidget)
			{
				// add the controls to the player screen
				MobileControlsWidget->AddToPlayerScreen(0);

			} else {

				UE_LOG(LogCH4_2, Error, TEXT("Could not spawn mobile controls widget."));

			}
		}
		

		if (PlayerStatsWidgetClass)
		{
			PlayerStatsWidget = CreateWidget<UCH4_2PlayerStatsWidget>(this, PlayerStatsWidgetClass);
		}
		if (PlayerStatsWidget)
		{
			PlayerStatsWidget->AddToPlayerScreen(10);
		}
		else
		{
			UE_LOG(LogCH4_2, Error, TEXT("Assign WBP_PlayerStats to PlayerStatsWidgetClass on the player controller BP."));
		}
		if (ChatWidgetClass)
		{
			ChatWidget = CreateWidget<UCH4_2ChatWidget>(this, ChatWidgetClass);
		}
		if (ChatWidget)
		{
			ChatWidget->AddToPlayerScreen(20);
			ChatWidget->SetHistory(ChatMessages);
		}
		else
		{
			UE_LOG(LogCH4_2, Error, TEXT("Assign WBP_Chat to ChatWidgetClass on the player controller BP."));
		}
		BindPlayerStats();
		GameStateSetHandle = GetWorld()->GameStateSetEvent.AddUObject(this, &ACH4_2PlayerController::BindMatchState);
		BindMatchState(GetWorld()->GetGameState());

		// spawn the UI widget and add it to the viewport
		VehicleUI = CreateWidget<UCH4_2UI>(this, VehicleUIClass);

		if (VehicleUI)
		{
			VehicleUI->AddToViewport();
			UE_LOG(LogCH4_2, Log, TEXT("Vehicle HUD created for local controller %s"), *GetName());

		} else {

			UE_LOG(LogCH4_2, Error, TEXT("Could not spawn vehicle UI widget."));

		}
	}
}

void ACH4_2PlayerController::InitPlayerState()
{
	Super::InitPlayerState();
	BindPlayerStats();
}

void ACH4_2PlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	BindPlayerStats();
}

void ACH4_2PlayerController::CleanupPlayerState()
{
	UnbindPlayerStats();
	if (PlayerStatsWidget)
	{
		PlayerStatsWidget->SetWaitingForPlayerState();
	}
	Super::CleanupPlayerState();
}

void ACH4_2PlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// No focus transfer during world teardown; just release the local widget.
	bChatOpen = false;
	if (ChatWidget)
	{
		ChatWidget->EndChat();
		ChatWidget->RemoveFromParent();
		ChatWidget = nullptr;
	}
	ChatMessages.Empty();
	if (GetWorld())
	{
		GetWorld()->GameStateSetEvent.Remove(GameStateSetHandle);
	}
	GameStateSetHandle.Reset();
	UnbindMatchState();
	UnbindPlayerStats();
	if (PlayerStatsWidget)
	{
		PlayerStatsWidget->RemoveFromParent();
		PlayerStatsWidget = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}

void ACH4_2PlayerController::BindMatchState(AGameStateBase* NewGameState)
{
	if (!IsLocalPlayerController())
	{
		return;
	}
	ACH4_2GameState* CurrentState = Cast<ACH4_2GameState>(NewGameState);
	if (BoundGameState.Get() != CurrentState)
	{
		UnbindMatchState();
		BoundGameState = CurrentState;
		if (CurrentState)
		{
			CurrentState->OnMatchChanged.AddUniqueDynamic(this, &ACH4_2PlayerController::RefreshMatchState);
		}
	}
	RefreshMatchState();
}

void ACH4_2PlayerController::UnbindMatchState()
{
	if (ACH4_2GameState* PreviousState = BoundGameState.Get())
	{
		PreviousState->OnMatchChanged.RemoveDynamic(this, &ACH4_2PlayerController::RefreshMatchState);
	}
	BoundGameState.Reset();
}

void ACH4_2PlayerController::RefreshMatchState()
{
	if (!IsLocalPlayerController() || !PlayerStatsWidget)
	{
		return;
	}
	if (const ACH4_2GameState* CurrentState = BoundGameState.Get())
	{
		PlayerStatsWidget->SetMatchSnapshot(CurrentState->GetMatchSnapshot());
	}
	else
	{
		PlayerStatsWidget->SetWaitingForGameState();
	}
}

void ACH4_2PlayerController::BindPlayerStats()
{
	if (!IsLocalPlayerController())
	{
		return;
	}
	ACH4_2PlayerState* CurrentState = GetPlayerState<ACH4_2PlayerState>();
	if (BoundPlayerState.Get() != CurrentState)
	{
		UnbindPlayerStats();
		BoundPlayerState = CurrentState;
		if (CurrentState)
		{
			CurrentState->OnStatsChanged.AddUniqueDynamic(this, &ACH4_2PlayerController::RefreshPlayerStats);
		}
	}
	// Handles both replication before widget creation and replication after it.
	RefreshPlayerStats();
}

void ACH4_2PlayerController::UnbindPlayerStats()
{
	if (ACH4_2PlayerState* PreviousState = BoundPlayerState.Get())
	{
		PreviousState->OnStatsChanged.RemoveDynamic(this, &ACH4_2PlayerController::RefreshPlayerStats);
	}
	BoundPlayerState.Reset();
}

void ACH4_2PlayerController::RefreshPlayerStats()
{
	if (!IsLocalPlayerController() || !PlayerStatsWidget)
	{
		return;
	}
	if (const ACH4_2PlayerState* CurrentState = BoundPlayerState.Get())
	{
		PlayerStatsWidget->SetStats(CurrentState->GetCoins(), CurrentState->GetBonusMaxSpeed());
	}
	else
	{
		PlayerStatsWidget->SetWaitingForPlayerState();
	}
}

ACH4_2PlayerState* ACH4_2PlayerController::FindPIEDebugPlayerState(int32 PlayerId) const
{
#if !UE_BUILD_SHIPPING
	if (!HasAuthority() || !GetWorld() || GetWorld()->WorldType != EWorldType::PIE)
	{
		UE_LOG(LogCH4_2, Warning, TEXT("Stats change rejected: run this command on the PIE host."));
		return nullptr;
	}
	if (const AGameStateBase* CurrentGameState = GetWorld()->GetGameState())
	{
		for (APlayerState* Candidate : CurrentGameState->PlayerArray)
		{
			if (IsValid(Candidate) && Candidate->GetPlayerId() == PlayerId)
			{
				if (ACH4_2PlayerState* Stats = Cast<ACH4_2PlayerState>(Candidate))
				{
					return Stats;
				}
				UE_LOG(LogCH4_2, Warning, TEXT("Player %d has the wrong PlayerState class. Check GameMode defaults."), PlayerId);
				return nullptr;
			}
		}
	}
	UE_LOG(LogCH4_2, Warning, TEXT("No PlayerState for PlayerId=%d"), PlayerId);
#endif
	return nullptr;
}

void ACH4_2PlayerController::CHStatsList()
{
#if !UE_BUILD_SHIPPING
	if (!GetWorld() || GetWorld()->WorldType != EWorldType::PIE)
	{
		return;
	}
	if (const AGameStateBase* CurrentGameState = GetWorld()->GetGameState())
	{
		for (APlayerState* Candidate : CurrentGameState->PlayerArray)
		{
			if (const ACH4_2PlayerState* Stats = Cast<ACH4_2PlayerState>(Candidate))
			{
				UE_LOG(LogCH4_2, Log, TEXT("Stats Authority=%d PlayerId=%d Self=%d Coins=%d BonusMaxSpeed=%g"),
					HasAuthority(), Stats->GetPlayerId(), Stats == PlayerState.Get(), Stats->GetCoins(), Stats->GetBonusMaxSpeed());
			}
			else
			{
				UE_LOG(LogCH4_2, Warning, TEXT("Unexpected PlayerState: %s. Check GameMode PlayerStateClass."), *GetNameSafe(Candidate));
			}
		}
	}
#endif
}

void ACH4_2PlayerController::CHStatsAddCoins(int32 PlayerId, int32 Amount)
{
	if (ACH4_2PlayerState* Stats = FindPIEDebugPlayerState(PlayerId))
	{
		const bool bApplied = Stats->AddCoins(Amount);
		UE_LOG(LogCH4_2, Log, TEXT("AddCoins PlayerId=%d Amount=%d Applied=%d Coins=%d"), PlayerId, Amount, bApplied, Stats->GetCoins());
	}
}

void ACH4_2PlayerController::CHStatsSpendCoins(int32 PlayerId, int32 Amount)
{
	if (ACH4_2PlayerState* Stats = FindPIEDebugPlayerState(PlayerId))
	{
		const bool bApplied = Stats->TrySpendCoins(Amount);
		UE_LOG(LogCH4_2, Log, TEXT("SpendCoins PlayerId=%d Amount=%d Applied=%d Coins=%d"), PlayerId, Amount, bApplied, Stats->GetCoins());
	}
}

void ACH4_2PlayerController::CHStatsAddSpeed(int32 PlayerId, float Amount)
{
	if (ACH4_2PlayerState* Stats = FindPIEDebugPlayerState(PlayerId))
	{
		const bool bApplied = Stats->IncreaseBonusMaxSpeed(Amount);
		UE_LOG(LogCH4_2, Log, TEXT("AddSpeed PlayerId=%d Amount=%g Applied=%d BonusMaxSpeed=%g"), PlayerId, Amount, bApplied, Stats->GetBonusMaxSpeed());
	}
}

void ACH4_2PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (IsLocalPlayerController() && InputComponent)
	{
		InputComponent->BindKey(EKeys::Enter, IE_Pressed, this, &ACH4_2PlayerController::OpenChat);
	}
	
	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!SVirtualJoystick::ShouldDisplayTouchInterface())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

void ACH4_2PlayerController::Tick(float Delta)
{
	Super::Tick(Delta);

	if (IsValid(VehiclePawn) && IsValid(VehicleUI))
	{
		VehicleUI->UpdateSpeed(VehiclePawn->GetChaosVehicleMovement()->GetForwardSpeed());
		VehicleUI->UpdateGear(VehiclePawn->GetChaosVehicleMovement()->GetCurrentGear());
	}
}

void ACH4_2PlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// get a pointer to the controlled pawn
	VehiclePawn = Cast<ACH4_2Pawn>(InPawn);

	// subscribe to the pawn's OnDestroyed delegate
	if (IsValid(VehiclePawn))
	{
		VehiclePawn->OnDestroyed.AddUniqueDynamic(this, &ACH4_2PlayerController::OnPawnDestroyed);
		UE_LOG(LogCH4_2, Log, TEXT("Vehicle possessed: Controller=%s Pawn=%s Location=%s"),
			*GetName(), *VehiclePawn->GetName(), *VehiclePawn->GetActorLocation().ToString());
		if (HasAuthority())
		{
			TArray<AActor*> OverlappingCoins;
			VehiclePawn->GetOverlappingActors(OverlappingCoins, ACH4_2CoinPickup::StaticClass());
			for (AActor* Actor : OverlappingCoins)
			{
				if (ACH4_2CoinPickup* Coin = Cast<ACH4_2CoinPickup>(Actor); IsValid(Coin))
				{
					Coin->TryCollect(VehiclePawn);
				}
			}
		}
	}
}

void ACH4_2PlayerController::OnRep_VehiclePawn()
{
	UE_LOG(LogCH4_2, Log, TEXT("Vehicle reference replicated: Controller=%s Pawn=%s"),
		*GetName(), *GetNameSafe(VehiclePawn));
}

void ACH4_2PlayerController::OnUnPossess()
{
	// Pawn detaches its controller before broadcasting OnDestroyed.
	// Keep that binding during destruction so the template respawn still runs.
	if (IsValid(VehiclePawn) && !VehiclePawn->IsActorBeingDestroyed())
	{
		VehiclePawn->OnDestroyed.RemoveDynamic(this, &ACH4_2PlayerController::OnPawnDestroyed);
	}
	VehiclePawn = nullptr;
	Super::OnUnPossess();
}

void ACH4_2PlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(ACH4_2PlayerController, VehiclePawn, COND_OwnerOnly);
}

void ACH4_2PlayerController::OnPawnDestroyed(AActor* DestroyedPawn)
{
	if (HasAuthority() && IsValid(GetWorld()) && !GetWorld()->bIsTearingDown)
	{
		if (AGameModeBase* GameMode = GetWorld()->GetAuthGameMode())
		{
			GameMode->RestartPlayer(this);
		}
	}
}
