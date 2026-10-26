// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CH4_2PlayerController.generated.h"

class UInputMappingContext;
class ACH4_2Pawn;
class UCH4_2UI;
class ACH4_2PlayerState;
class UCH4_2PlayerStatsWidget;
class ACH4_2GameState;
class AGameStateBase;
class UCH4_2ChatWidget;
class UParticleSystem;
class USoundBase;

/**
 *  Vehicle Player Controller class
 *  Handles input mapping and user interface
 */
UCLASS(abstract)
class ACH4_2PlayerController : public APlayerController
{
	GENERATED_BODY()

protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** If true, the optional steering wheel input mapping context will be registered */
	UPROPERTY(EditAnywhere, Category = "Input|Steering Wheel Controls")
	bool bUseSteeringWheelControls = false;

	/** Optional Input Mapping Context to be used for steering wheel input.
	 *  This is added alongside the default Input Mapping Context and does not block other forms of input.
	 */
	UPROPERTY(EditAnywhere, Category = "Input|Steering Wheel Controls", meta = (EditCondition = "bUseSteeringWheelControls"))
	UInputMappingContext* SteeringWheelInputMappingContext;

	/** Type of vehicle to automatically respawn when it's destroyed */
	UPROPERTY(EditAnywhere, Category="Vehicle|Respawn")
	TSubclassOf<ACH4_2Pawn> VehiclePawnClass;

	/** Pointer to the controlled vehicle pawn */
	UPROPERTY(ReplicatedUsing=OnRep_VehiclePawn)
	TObjectPtr<ACH4_2Pawn> VehiclePawn;

	/** Type of the UI to spawn */
	UPROPERTY(EditAnywhere, Category="Vehicle|UI")
	TSubclassOf<UCH4_2UI> VehicleUIClass;

	/** Pointer to the UI widget */
	UPROPERTY(Transient)
	TObjectPtr<UCH4_2UI> VehicleUI;

	/** Assign the generated WBP assets in BP_VehicleAdvPlayerController. */
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UCH4_2PlayerStatsWidget> PlayerStatsWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UCH4_2ChatWidget> ChatWidgetClass;
		
protected:

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Input setup */
	virtual void SetupInputComponent() override;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void InitPlayerState() override;
	virtual void CleanupPlayerState() override;
	void SubmitChatMessage(const FString& Message);
	void CloseChat();

	/** Server sends cosmetics over the persistent PC channel before destroying a pickup. */
	UFUNCTION(Client, Reliable)
	void ClientPlayPickupEffects(FVector Location, UParticleSystem* Effect, USoundBase* Sound);

	/** Local PIE diagnostics only. These are not Server RPCs. */
	UFUNCTION(Exec)
	void CHStatsList();

	UFUNCTION(Exec)
	void CHStatsAddCoins(int32 PlayerId, int32 Amount);

	UFUNCTION(Exec)
	void CHStatsSpendCoins(int32 PlayerId, int32 Amount);

	UFUNCTION(Exec)
	void CHStatsAddSpeed(int32 PlayerId, float Amount);

	/** Update vehicle UI on tick */
	virtual void Tick(float Delta) override;

protected:
	virtual void OnRep_PlayerState() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Pawn setup */
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	UFUNCTION()
	void OnRep_VehiclePawn();

	/** Handles pawn destruction and respawning */
	UFUNCTION()
	void OnPawnDestroyed(AActor* DestroyedPawn);

private:
	static constexpr int32 MaxChatLength = 200;
	static constexpr double ChatIntervalSeconds = 0.5;
	static constexpr int32 MaxChatHistory = 50;

	UPROPERTY(Transient)
	TObjectPtr<UCH4_2ChatWidget> ChatWidget;

	TArray<FText> ChatMessages;
	bool bChatOpen = false;
	bool bCursorVisibleBeforeChat = false;
	double NextChatAttemptTime = 0.0;
	bool bRateLimitNoticeSent = false;

	void OpenChat();

	UFUNCTION(Server, Reliable)
	void ServerSubmitChat(const FString& Message);

	UFUNCTION(Client, Reliable)
	void ClientReceiveChat(const FString& SenderName, const FString& Message);

	UFUNCTION(Client, Reliable)
	void ClientChatRejected(const FText& Reason);

	UPROPERTY(Transient)
	TObjectPtr<UCH4_2PlayerStatsWidget> PlayerStatsWidget;

	UPROPERTY(Transient)
	TWeakObjectPtr<ACH4_2PlayerState> BoundPlayerState;

	UPROPERTY(Transient)
	TWeakObjectPtr<ACH4_2GameState> BoundGameState;

	FDelegateHandle GameStateSetHandle;
	void BindMatchState(AGameStateBase* NewGameState);
	void UnbindMatchState();

	UFUNCTION()
	void RefreshMatchState();

	void BindPlayerStats();
	void UnbindPlayerStats();

	UFUNCTION()
	void RefreshPlayerStats();

	ACH4_2PlayerState* FindPIEDebugPlayerState(int32 PlayerId) const;
};
