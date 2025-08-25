// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "MuzimiPlayerController.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool,bPingTooHigh);
/**
 * 
 */
UCLASS()
class MUZIMI_API AMuzimiPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void SetHUDHealth(float Health,float MaxHealth);
	void SetHUDShield(float Shield, float MaxShield);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo,int32 MagCapacity);
	void SetHUDCarriedAmmo(int32 CarriedAmmo);
	void SetHUDMatchCountDown(float CountDownTime);
	void SetHUDAnnouncementCountDown(float CountDownTime);
	void SetHUDGrenadesCountDown(int32 Grenades);
	virtual void Tick(float DeltaTime) override;
	void CheckPing(float DeltaTime);
	void CheckTimeSync(float DeltaTime);
	virtual void OnPossess(APawn* InPawn);
	virtual float GetServerTime();//Synced with server world clock
	virtual void ReceivedPlayer() override;
	void HideTeamScores();
	void InitTeamScores();
	void SetHUDRedTeamScore(int32 RedScore);
	void SetHUDBlueTeamScore(int32 BlueScore);

	void HighPingWarning();
	void StopHighPingWarning();
	FString GetInfoText(const TArray<class AMuzimiPlayerState*>& Players);
	FString GetTeamInfoText(class AMuzimiGameState* MuzimiGameState);
	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame(FName stateOfmatch, float StartingTime, float Match, float Warmup,float Cooldown);

	void OnMatchStateSet(FName State,bool bTeamsMatch=false);

	UPROPERTY()
	class AMuzimiGameMode* MuzimiGameMode;

	void HandleCooldown();
	float SingleTripTime = 0;

	FHighPingDelegate HighPingDelegate;

	void BroadcastElim(APlayerState* Attacker, APlayerState* Victim);
protected:
	virtual void SetupInputComponent() override;

	void ShowReturntoMainMenu(const FInputActionValue& Value);

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* QuitAction;

	void SetHUDTime();
	void PollInit();

	/**
	* Sync time between client and server
	*/
	//Requests the current server time ,passing in the cient's time when the request was sent
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float ClientRequestTime);

	//Report the current server time to the client in response to the request
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float ClientRequestTime, float TimeServerReceivedClientRequest);

	float ClientServerDelta = 0.f;//difference between client and server

	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f; 
	UPROPERTY()
	float TimeSyncRunningTime = 0.f;

	UFUNCTION(Client,Reliable)
	void ClientElimAnnouncement(APlayerState* Attacker, APlayerState* Victim);
	
	UPROPERTY(ReplicatedUsing = OnRep_ShowTeamScores)
	bool bShowTeamScores = false;

	UFUNCTION()
	void OnRep_ShowTeamScores();
private:
	/** 
	* Return to main menu
	*/
	UPROPERTY(EditAnywhere,Category=HUD)
	TSubclassOf<class UUserWidget> ReturnToMainMenuWidget;

	UPROPERTY()
	class UReturnToMainMenu* ReturnToMainMenu;

	bool bReturnToMainMenuOpen = false;
	UPROPERTY()
	float LevelStartingTime = 0.f;
	UPROPERTY()
	float MatchTime = 10.f;
	UPROPERTY()
	float WarmupTime = 5.f;
	UPROPERTY()
	float CooldownTime = 5.f;
	UPROPERTY()
	class AMuzimiHUD* MuzimiHUD;
	UPROPERTY()
	uint32 CountdownInt = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	void HandleMatchHasStarted(bool bTeamMatch=false);

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	bool bInitializeCharacterOverlay = false;

	float HUDHealth ;
	float HUDMaxHealth ;
	float HUDShield;
	bool bInitializeShield = false;
	float HUDMaxShield;
	float HUDScore ;
	int32 HUDDefeats ;
	int32 HUDGrenades;

	float HighPingRunningTime = 0.f;

	float PingAnimationRunningTime = 0.f;

	UPROPERTY(EditAnywhere)
	float HIghPingDuration = 5.f;

	UPROPERTY(EditAnywhere)
	float CheckPingFrequency = 20.f;

	UFUNCTION(Server,Reliable)
	void ServerReportPingStatus(bool bHighPing);

	UPROPERTY(EditAnywhere)
	float HighPingThreshold = 50.f;
public:
	 
};
