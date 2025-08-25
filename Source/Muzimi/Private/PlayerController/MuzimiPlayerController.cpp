// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/MuzimiPlayerController.h"
#include "HUD/MuzimiHUD.h"
#include "HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Net/UnrealNetwork.h"
#include "Components/TextBlock.h"
#include "HUD/Announcement.h"
#include "GameMode/MuzimiGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "MuzimiComponents/CombatComponent.h"
#include "Weapon/Weapon.h"
#include "Character/MuzimiCharacter.h"
#include "PlayerState/MuzimiPlayerState.h"
#include "GameState/MuzimiGameState.h"
#include "InputActionValue.h"
#include "EnhancedInputComponent.h"
#include "HUD/ReturnToMainMenu.h"
#include "MuzimiTypes/Announcement.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Components/Image.h"

void AMuzimiPlayerController::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("[PlayerController] BeginPlay called for %s"), *GetName());
	MuzimiHUD = Cast<AMuzimiHUD>(GetHUD());
	
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem)
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
	ServerCheckMatchState();
}

void AMuzimiPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetHUDTime();
	PollInit();
	CheckTimeSync(DeltaTime);
	CheckPing(DeltaTime);

}

void AMuzimiPlayerController::CheckPing(float DeltaTime)
{
	HighPingRunningTime += DeltaTime;
	if (HighPingRunningTime > CheckPingFrequency)
	{
		PlayerState = (PlayerState == nullptr) ? TObjectPtr<APlayerState>(GetPlayerState<APlayerState>()) : PlayerState;
		if (PlayerState)
		{
			if (PlayerState->GetCompressedPing() * 4 > HighPingThreshold)
			{
				HighPingWarning();
				PingAnimationRunningTime = 0.f;
				ServerReportPingStatus(true);
			}
			else
			{
				ServerReportPingStatus(false);
			}
		}
		HighPingRunningTime = 0.f;
	}
	bool bHighPingAnimationPlaying = MuzimiHUD &&
		MuzimiHUD->CharacterOverlay &&
		MuzimiHUD->CharacterOverlay->HighPingAnimation &&
		MuzimiHUD->CharacterOverlay->IsAnimationPlaying(MuzimiHUD->CharacterOverlay->HighPingAnimation);
	if (bHighPingAnimationPlaying)
	{
		PingAnimationRunningTime += DeltaTime;
		if (PingAnimationRunningTime > HIghPingDuration)
		{
			StopHighPingWarning();
		}
	}
}

void AMuzimiPlayerController::ServerReportPingStatus_Implementation(bool bHighPing)
{
	HighPingDelegate.Broadcast(bHighPing);
}

void AMuzimiPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		TimeSyncRunningTime = 0.f;
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void AMuzimiPlayerController::BroadcastElim(APlayerState* Attacker, APlayerState* Victim)
{
	ClientElimAnnouncement(Attacker, Victim);
}

void AMuzimiPlayerController::ClientElimAnnouncement_Implementation(APlayerState* Attacker, APlayerState* Victim)
{
	APlayerState* Self = GetPlayerState<APlayerState>();
	if (Attacker && Victim && Self)
	{
		MuzimiHUD = MuzimiHUD == nullptr ? Cast<AMuzimiHUD>(GetHUD()):MuzimiHUD;
		if (MuzimiHUD)
		{
			FString AttackerName, VictimName;
			AttackerName = Attacker == Self ? Attacker->GetPlayerName()+ FString("(You)") : Attacker->GetPlayerName();
			VictimName = Victim == Self ? Victim->GetPlayerName()+FString("(You)") :Victim->GetPlayerName();
			MuzimiHUD->AddElimAnnouncement(AttackerName, VictimName);
		}
	}
}


void AMuzimiPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (InputComponent == nullptr) return;

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EnhancedInputComponent) return;
	
	EnhancedInputComponent->BindAction(QuitAction, ETriggerEvent::Started, this, &AMuzimiPlayerController::ShowReturntoMainMenu);
	
}

void AMuzimiPlayerController::ShowReturntoMainMenu(const FInputActionValue& Value)
{

	if (ReturnToMainMenuWidget == nullptr)
	{
		return;
	}
	if (ReturnToMainMenu == nullptr)
	{
		ReturnToMainMenu = CreateWidget<UReturnToMainMenu>(this, ReturnToMainMenuWidget);
	}
	if (ReturnToMainMenu)
	{
		bReturnToMainMenuOpen = !bReturnToMainMenuOpen;
		if (bReturnToMainMenuOpen)
		{
			ReturnToMainMenu->MenuSetup();
		}
		else
		{
			ReturnToMainMenu->MenuTearDown();
		}
	}
}

void AMuzimiPlayerController::SetHUDTime()
{
	if (MuzimiHUD == nullptr) return;
	float TimeLeft = 0.f;
	if(MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if(MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::Cooldown) TimeLeft = CooldownTime+WarmupTime+MatchTime - GetServerTime() + LevelStartingTime;
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	if (HasAuthority())
	{

		MuzimiGameMode = MuzimiGameMode==nullptr? Cast<AMuzimiGameMode>(UGameplayStatics::GetGameMode(this)): MuzimiGameMode;
		if (MuzimiGameMode)
		{
			SecondsLeft = FMath::CeilToInt(MuzimiGameMode->GetCountdownTime()+LevelStartingTime);
		}
	}
	if (CountdownInt != SecondsLeft)
	{
	
		if (MatchState == MatchState::WaitingToStart || MatchState==MatchState::Cooldown)
		{
			SetHUDAnnouncementCountDown(TimeLeft);
		}
		if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountDown(TimeLeft);
		}
	}

		CountdownInt = SecondsLeft;
}

void AMuzimiPlayerController::PollInit()
{
	if (MuzimiHUD == nullptr) return;
	if (CharacterOverlay == nullptr )
	{

		if (MuzimiHUD && MuzimiHUD->CharacterOverlayClass)
		{
			CharacterOverlay = MuzimiHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				AMuzimiCharacter* MuzimiCharacter = Cast<AMuzimiCharacter>(GetPawn());
				SetHUDHealth(HUDHealth, HUDMaxHealth);
			    SetHUDShield(MuzimiCharacter->GetShield(), MuzimiCharacter->GetMaxShield());
				SetHUDScore(HUDScore);
				SetHUDDefeats(HUDDefeats);
				if (MuzimiCharacter && MuzimiCharacter->GetCombat() && MuzimiCharacter->GetCombat()->GetEquippedWeapon())
				{
					SetHUDWeaponAmmo(MuzimiCharacter->GetCombat()->GetEquippedWeapon()->GetAmmo(), MuzimiCharacter->GetCombat()->GetEquippedWeapon()->GetMagCapacity());
				}
				if (MuzimiCharacter && MuzimiCharacter->GetCombat())
				{
					SetHUDCarriedAmmo(MuzimiCharacter->GetCombat()->GetCarriedAmmo());
					SetHUDGrenadesCountDown(MuzimiCharacter->GetCombat()->GetGrenades());
				}
			}

		}
	}
}

void AMuzimiPlayerController::OnRep_ShowTeamScores()
{
	if (bShowTeamScores)
	{
		InitTeamScores();
	}
	else
	{
		HideTeamScores();
	}
}

void AMuzimiPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void AMuzimiPlayerController::ServerCheckMatchState_Implementation()
{
	AMuzimiGameMode* GameMode = Cast<AMuzimiGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		CooldownTime = GameMode->CooldownTime;
		ClientJoinMidgame(MatchState,LevelStartingTime, MatchTime, WarmupTime,CooldownTime);

		if (MuzimiHUD && MatchState == MatchState::WaitingToStart)
		{
		MuzimiHUD->AddAnnouncement();
		}
	}
}

void AMuzimiPlayerController::ClientJoinMidgame_Implementation(FName stateOfmatch, float StartingTime ,float Match,float Warmup, float Cooldown)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	LevelStartingTime = StartingTime;
	MatchState = stateOfmatch;
	CooldownTime = Cooldown;
	OnMatchStateSet(MatchState);
	if (MuzimiHUD && MatchState == MatchState::WaitingToStart && !HasAuthority())
	{
		MuzimiHUD->AddAnnouncement();
	}
}

void AMuzimiPlayerController::OnMatchStateSet(FName State, bool bTeamsMatch )
{
	MatchState = State;

	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted(bTeamsMatch);
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}

}

void AMuzimiPlayerController::HandleMatchHasStarted(bool bTeamMatch )
{
	MuzimiHUD = MuzimiHUD ? MuzimiHUD : Cast<AMuzimiHUD>(GetHUD());

	if(HasAuthority()) bShowTeamScores = bTeamMatch;
	if (MuzimiHUD)
	{
		MuzimiHUD->AddCharacterOverlay();
		if (MuzimiHUD->Announcement)
		{
			MuzimiHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
		if (!HasAuthority()) return;
		if (bTeamMatch)
		{
			InitTeamScores();
		}
		else
		{
			HideTeamScores();
		}
	}
}

void AMuzimiPlayerController::HandleCooldown()
{
	MuzimiHUD = MuzimiHUD ? MuzimiHUD : Cast<AMuzimiHUD>(GetHUD());
	if (MuzimiHUD)
	{
		MuzimiHUD->CharacterOverlay->RemoveFromParent();
		bool bHUDVaild = MuzimiHUD &&
			MuzimiHUD->Announcement &&
			MuzimiHUD->Announcement->AnnouncementText &&
			MuzimiHUD->Announcement->InfoText;
		if (bHUDVaild)
		{
			MuzimiHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText =Announcement::NewMatchStartsIn;
			MuzimiHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));
			AMuzimiGameState* MuzimiGameState = Cast<AMuzimiGameState>(UGameplayStatics::GetGameState(this));
			AMuzimiPlayerState* MuzimiPlayerState = GetPlayerState<AMuzimiPlayerState>();
			if (MuzimiGameState && MuzimiPlayerState)
			{
				TArray<AMuzimiPlayerState*> TopPlayers = MuzimiGameState->TopScoringPlayers;
				FString InfoTextString=bShowTeamScores?GetTeamInfoText(MuzimiGameState): GetInfoText(TopPlayers);
				MuzimiHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
			}
		}
		AMuzimiCharacter* MuzimiCharacter = Cast<AMuzimiCharacter>(GetPawn());
		if (MuzimiCharacter)
		{
			MuzimiCharacter->bDisableGameplay=true;
			MuzimiCharacter->bUseControllerRotationYaw = false;
			MuzimiCharacter->GetCombat()->FireButtonPressed(false);
		}
		
	}
}
FString AMuzimiPlayerController::GetTeamInfoText(AMuzimiGameState* MuzimiGameState)
{
	if (MuzimiGameState == nullptr) return FString("MuzimiGameState == nullptr");
	FString InfoTextString;
	const int32 RedTeamScore = MuzimiGameState->RedTeamScore;
	const int32 BlueTeamScore = MuzimiGameState->BlueTeamScore;
	if (RedTeamScore == 0 && BlueTeamScore == 0)
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}
	else if (RedTeamScore == BlueTeamScore)
	{
		InfoTextString = FString::Printf(TEXT("%s\n"), *Announcement::TeamsTiedForTheWin);
		InfoTextString.Append(Announcement::RedTeam);
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(Announcement::BlueTeam);
	}
	else if (RedTeamScore > BlueTeamScore)
	{
		InfoTextString = Announcement::RedTeamWins;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s:%d\n"), *Announcement::RedTeam, RedTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s:%d\n"), *Announcement::BlueTeam, BlueTeamScore));
	}
	else if (RedTeamScore < BlueTeamScore)
	{
		InfoTextString = Announcement::BlueTeamWins;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s:%d\n"), *Announcement::BlueTeam, BlueTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s:%d\n"), *Announcement::RedTeam, RedTeamScore));	
	}
	return InfoTextString;
}
FString AMuzimiPlayerController::GetInfoText(const TArray<class AMuzimiPlayerState*>& TopPlayers)
{
	AMuzimiPlayerState* MuzimiPlayerState = GetPlayerState<AMuzimiPlayerState>();
	if (MuzimiPlayerState == nullptr) return FString("MuzimiPlayerState == nullptr");
	FString InfoTextString;

	if (!TopPlayers.Num())
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}
	else if (TopPlayers.Num() == 1 && TopPlayers[0] == MuzimiPlayerState)
	{
		InfoTextString = Announcement::YouAreTheWinner;
	}
	else if (TopPlayers.Num() == 1)
	{
		InfoTextString = FString::Printf(TEXT("Winner:\n%s"), *TopPlayers[0]->GetPlayerName());
	}
	else if (TopPlayers.Num() > 1)
	{
		InfoTextString = Announcement::PlayersTiedForTheWin;
		InfoTextString.Append(FString("\n"));
		for (auto TopPlayer : TopPlayers)
		{
			InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TopPlayer->GetPlayerName()));

		}
	}
	return InfoTextString;
}
void AMuzimiPlayerController::ServerRequestServerTime_Implementation(float ClientRequestTime)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(ClientRequestTime, ServerTimeOfReceipt);
}

void AMuzimiPlayerController::ClientReportServerTime_Implementation(float ClientRequestTime, float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - ClientRequestTime;
	SingleTripTime = 0.5 * RoundTripTime;
	float CurrentServerTime = TimeServerReceivedClientRequest + (0.5*RoundTripTime);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

void AMuzimiPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMuzimiPlayerController, MatchState)
	DOREPLIFETIME(AMuzimiPlayerController, bShowTeamScores);
}

void AMuzimiPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	MuzimiHUD = MuzimiHUD ? MuzimiHUD : Cast<AMuzimiHUD>(GetHUD());
	if (MuzimiHUD == nullptr) return;
	bool bHUDVaild = MuzimiHUD &&
		MuzimiHUD->CharacterOverlay &&
		MuzimiHUD->CharacterOverlay->HealthBar &&
		MuzimiHUD->CharacterOverlay->HealthText;
	if (bHUDVaild)
	{
		const float HealthPercent = Health / MaxHealth;
		MuzimiHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		MuzimiHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void AMuzimiPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	MuzimiHUD = MuzimiHUD ? MuzimiHUD : Cast<AMuzimiHUD>(GetHUD());
	if (MuzimiHUD == nullptr) return;
	bool bHUDVaild = MuzimiHUD &&
		MuzimiHUD->CharacterOverlay &&
		MuzimiHUD->CharacterOverlay->ShieldBar &&
		MuzimiHUD->CharacterOverlay->ShieldText;
	if (bHUDVaild)
	{
		bInitializeShield = false;
		const float ShieldPercent = Shield / MaxShield;
		MuzimiHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
		FString ShieldText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
		MuzimiHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}
	else
	{
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
	}
}

void AMuzimiPlayerController::HighPingWarning()
{
	MuzimiHUD = MuzimiHUD ? MuzimiHUD : Cast<AMuzimiHUD>(GetHUD());
	bool bHUDVaild = MuzimiHUD &&
		MuzimiHUD->CharacterOverlay &&
		MuzimiHUD->CharacterOverlay->HighPingImage &&
		MuzimiHUD->CharacterOverlay->HighPingAnimation;
	if (bHUDVaild)
	{
		MuzimiHUD->CharacterOverlay->HighPingImage->SetOpacity(1.f);
		MuzimiHUD->CharacterOverlay->PlayAnimation(MuzimiHUD->CharacterOverlay->HighPingAnimation,0.f,5);
	}
}

void AMuzimiPlayerController::StopHighPingWarning()
{
	MuzimiHUD = MuzimiHUD ? MuzimiHUD : Cast<AMuzimiHUD>(GetHUD());
	bool bHUDVaild = MuzimiHUD &&
		MuzimiHUD->CharacterOverlay &&
		MuzimiHUD->CharacterOverlay->HighPingImage &&
		MuzimiHUD->CharacterOverlay->HighPingAnimation;
	if (bHUDVaild)
	{
		MuzimiHUD->CharacterOverlay->HighPingImage->SetOpacity(0.f);
		if (MuzimiHUD->CharacterOverlay->IsAnimationPlaying(MuzimiHUD->CharacterOverlay->HighPingAnimation))
		{
			MuzimiHUD->CharacterOverlay->StopAnimation(MuzimiHUD->CharacterOverlay->HighPingAnimation);
		}
	}
}


void AMuzimiPlayerController::SetHUDScore(float Score)
{
	MuzimiHUD = MuzimiHUD ? MuzimiHUD : Cast<AMuzimiHUD>(GetHUD());
	if (MuzimiHUD == nullptr) return;
	bool bHUDVaild = MuzimiHUD &&
		MuzimiHUD->CharacterOverlay &&
		MuzimiHUD->CharacterOverlay->ScoreAmount;
	if (bHUDVaild)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		MuzimiHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDScore = Score;
	}
}

void AMuzimiPlayerController::SetHUDDefeats(int32 Defeats)
{
	MuzimiHUD = MuzimiHUD ? MuzimiHUD : Cast<AMuzimiHUD>(GetHUD());
	if (MuzimiHUD == nullptr) return;
	bool bHUDVaild = MuzimiHUD &&
		MuzimiHUD->CharacterOverlay &&
		MuzimiHUD->CharacterOverlay->DefeatsAmount;
	if (bHUDVaild)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"),Defeats);
		MuzimiHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDDefeats = Defeats;
	}
}

void AMuzimiPlayerController::SetHUDWeaponAmmo(int32 Ammo,int32 MagCapacity)
{
	MuzimiHUD = MuzimiHUD ? MuzimiHUD : Cast<AMuzimiHUD>(GetHUD());
	if (MuzimiHUD == nullptr) return;
	bool bHUDVaild = MuzimiHUD &&
		MuzimiHUD->CharacterOverlay &&
		MuzimiHUD->CharacterOverlay->WeaponAmmoAmount;
	bool bisHUDVaild = MuzimiHUD &&
		MuzimiHUD->CharacterOverlay &&
		MuzimiHUD->CharacterOverlay->MagCapacity;
	if (bHUDVaild)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		MuzimiHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	if (bisHUDVaild)
	{
		FString CapacityText = FString::Printf(TEXT("%d"),MagCapacity);
		MuzimiHUD->CharacterOverlay->MagCapacity->SetText(FText::FromString(CapacityText));
	}
}

void AMuzimiPlayerController::SetHUDCarriedAmmo(int32 CarriedAmmo)
{
	MuzimiHUD = MuzimiHUD ? MuzimiHUD : Cast<AMuzimiHUD>(GetHUD());
	if (MuzimiHUD == nullptr) return;
	bool bHUDVaild = MuzimiHUD &&
		MuzimiHUD->CharacterOverlay &&
		MuzimiHUD->CharacterOverlay->CarriedAmmoAmount;
	if (bHUDVaild)
	{
		FString CarriedAmmoText = FString::Printf(TEXT("%d"),CarriedAmmo);
		MuzimiHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(CarriedAmmoText));
	}
}

void AMuzimiPlayerController::SetHUDMatchCountDown(float CountDownTime)
{
	MuzimiHUD = MuzimiHUD ? MuzimiHUD : Cast<AMuzimiHUD>(GetHUD());
	if (MuzimiHUD == nullptr) return;
	bool bHUDVaild = MuzimiHUD &&
		MuzimiHUD->CharacterOverlay &&
		MuzimiHUD->CharacterOverlay->MatchCountDown;
	if (bHUDVaild)
	{
		if (CountDownTime < 0)
		{
			MuzimiHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}
		int32 Minutes = FMath::FloorToInt(CountDownTime / 60.f);
		int32 Seconds = FMath::FloorToInt(CountDownTime) % 60;
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes,Seconds);
		MuzimiHUD->CharacterOverlay->MatchCountDown->SetText(FText::FromString(CountdownText));
	}
}

void AMuzimiPlayerController::SetHUDAnnouncementCountDown(float CountDownTime)
{
	MuzimiHUD = MuzimiHUD ? MuzimiHUD : Cast<AMuzimiHUD>(GetHUD());
	if (MuzimiHUD == nullptr) return;
	bool bHUDVaild = MuzimiHUD &&
		MuzimiHUD->Announcement &&
		MuzimiHUD->Announcement->WarmupTime;
	if (bHUDVaild)
	{
		if (CountDownTime < 0)
		{
			MuzimiHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}
		int32 Minutes = FMath::FloorToInt(CountDownTime / 60.f);
		int32 Seconds = FMath::FloorToInt(CountDownTime) % 60;
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		MuzimiHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

void AMuzimiPlayerController::SetHUDGrenadesCountDown(int32 Grenades)
{
	MuzimiHUD = MuzimiHUD ? MuzimiHUD : Cast<AMuzimiHUD>(GetHUD());
	if (MuzimiHUD == nullptr) return;
	bool bHUDVaild = MuzimiHUD &&
		MuzimiHUD->CharacterOverlay &&
		MuzimiHUD->CharacterOverlay->GrenadesText;
	if (bHUDVaild)
	{
		FString GrenadesText = FString::Printf(TEXT("%d"), Grenades);
		MuzimiHUD->CharacterOverlay->GrenadesText->SetText(FText::FromString(GrenadesText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDGrenades = Grenades;
	}
}


void AMuzimiPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	AMuzimiCharacter* MuzimiCharacter = Cast<AMuzimiCharacter>(InPawn);
	if (MuzimiCharacter)
	{
		SetHUDHealth(MuzimiCharacter->GetHealth(), MuzimiCharacter->GetMaxHealth());
		SetHUDShield(MuzimiCharacter->GetShield(), MuzimiCharacter->GetMaxShield());
	}
}

float AMuzimiPlayerController::GetServerTime()
{
	if (HasAuthority())
	{
		return GetWorld()->GetTimeSeconds();
	}
	else
	{
		return GetWorld()->GetTimeSeconds() + ClientServerDelta;
	}
}

void AMuzimiPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void AMuzimiPlayerController::HideTeamScores()
{
	MuzimiHUD = MuzimiHUD ? MuzimiHUD : Cast<AMuzimiHUD>(GetHUD());
	bool bHUDVaild = MuzimiHUD &&
		MuzimiHUD->CharacterOverlay &&
		MuzimiHUD->CharacterOverlay->RedTeamScore &&
		MuzimiHUD->CharacterOverlay->BlueTeamScore &&
		MuzimiHUD->CharacterOverlay->ScoreSpacerText;
	if (bHUDVaild)
	{
		MuzimiHUD->CharacterOverlay->RedTeamScore->SetText(FText());
		MuzimiHUD->CharacterOverlay->BlueTeamScore->SetText(FText());
		MuzimiHUD->CharacterOverlay->ScoreSpacerText->SetText(FText());
	}
}

void AMuzimiPlayerController::InitTeamScores()
{
	MuzimiHUD = MuzimiHUD ? MuzimiHUD : Cast<AMuzimiHUD>(GetHUD());
	bool bHUDVaild = MuzimiHUD &&
		MuzimiHUD->CharacterOverlay &&
		MuzimiHUD->CharacterOverlay->RedTeamScore &&
		MuzimiHUD->CharacterOverlay->BlueTeamScore &&
		MuzimiHUD->CharacterOverlay->ScoreSpacerText;
	if (bHUDVaild)
	{
		FString Zero("0");
		FString Spacer(" / ");
		MuzimiHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString(Zero));
		MuzimiHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(Zero));
		MuzimiHUD->CharacterOverlay->ScoreSpacerText->SetText(FText::FromString(Spacer));
	}
}

void AMuzimiPlayerController::SetHUDRedTeamScore(int32 RedScore)
{
	MuzimiHUD = MuzimiHUD ? MuzimiHUD : Cast<AMuzimiHUD>(GetHUD());
	if (MuzimiHUD == nullptr) return;
	bool bHUDVaild = MuzimiHUD &&
		MuzimiHUD->CharacterOverlay &&
		MuzimiHUD->CharacterOverlay->RedTeamScore &&
		MuzimiHUD->CharacterOverlay->BlueTeamScore &&
		MuzimiHUD->CharacterOverlay->ScoreSpacerText;
	if (bHUDVaild)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), RedScore);
		MuzimiHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString(ScoreText));
	}
}

void AMuzimiPlayerController::SetHUDBlueTeamScore(int32 BlueScore)
{
	MuzimiHUD = MuzimiHUD ? MuzimiHUD : Cast<AMuzimiHUD>(GetHUD());
	if (MuzimiHUD == nullptr) return;
	bool bHUDVaild = MuzimiHUD &&
		MuzimiHUD->CharacterOverlay &&
		MuzimiHUD->CharacterOverlay->RedTeamScore &&
		MuzimiHUD->CharacterOverlay->BlueTeamScore &&
		MuzimiHUD->CharacterOverlay->ScoreSpacerText;
	if (bHUDVaild)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), BlueScore);
		MuzimiHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(ScoreText));
	}
}




