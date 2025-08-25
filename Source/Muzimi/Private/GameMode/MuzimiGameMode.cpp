// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/MuzimiGameMode.h"
#include "Character/MuzimiCharacter.h"
#include "PlayerController/MuzimiPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include  "Character/MuzimiCharacter.h"
#include "HUD/MuzimiHUD.h"

#include "EnhancedInputSubsystems.h"
#include "GameState/MuzimiGameState.h"
#include "PlayerState/MuzimiPlayerState.h"


namespace MatchState
{
	const FName Cooldown = TEXT("Cooldown");
}
AMuzimiGameMode::AMuzimiGameMode()
{
	bDelayedStart = true;  
}




void AMuzimiGameMode::RestartGame()
{
	LevelStartingTime = GetWorld()->GetTimeSeconds();
	Super::RestartGame();
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		AMuzimiPlayerController* MuzimiPlayerController = Cast<AMuzimiPlayerController>(*Iterator);
		MuzimiPlayerController->ClientRestart(MuzimiPlayerController->GetPawn());
		MuzimiPlayerController->DispatchBeginPlay();
	}
}

void AMuzimiGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
	UE_LOG(LogGameMode, Log, TEXT(">> Force Controller Replacement in Child GameMode: %s"), *C->GetName());

	APlayerController* PC = Cast<APlayerController>(C);
	if (!PC)
	{
		return; // 非玩家控制器直接返回
	}

	// 强制获取目标控制器类（即使与当前类相同）
	UClass* PCClassToSpawn = GetPlayerControllerClassToSpawnForSeamlessTravel(PC);

	// 1. 生成新控制器
	APlayerController* NewPC = SpawnPlayerControllerCommon(
		PC->IsLocalPlayerController() ? ROLE_SimulatedProxy : ROLE_AutonomousProxy,
		PC->GetFocalLocation(),
		PC->GetControlRotation(),
		PCClassToSpawn
	);

	if (!NewPC)
	{
		UE_LOG(LogGameMode, Error, TEXT("Failed to spawn new PlayerController!"));
		PC->Destroy();
		return;
	}

	// 2. 迁移数据
	PC->SeamlessTravelTo(NewPC);    // 旧控制器 → 新控制器
	NewPC->SeamlessTravelFrom(PC);  // 新控制器 ← 旧控制器
	SwapPlayerControllers(PC, NewPC);

	// 3. 更新外部引用
	C = NewPC;

	// 4. 销毁旧控制器
	PC->Destroy();

	// 5. 初始化新控制器（调用父类逻辑）
	Super::HandleSeamlessTravelPlayer(C);
	LevelStartingTime = GetWorld()->GetTimeSeconds();
	UE_LOG(LogGameMode, Log, TEXT("<< Force Controller Replacement Complete: %s"), *C->GetName());
}



void AMuzimiGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds()+LevelStartingTime;
		if (CountdownTime <=0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime+MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = WarmupTime + MatchTime + CooldownTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			RestartGame();
		}
	}
}

void AMuzimiGameMode::PlayerEliminated(AMuzimiCharacter* ElimmedCharacter, AMuzimiPlayerController* VictimController, AMuzimiPlayerController* AttackerController)
{
	if (VictimController == nullptr || VictimController->PlayerState == nullptr) return;
	if (AttackerController == nullptr || AttackerController->PlayerState == nullptr) return;
	AMuzimiPlayerState* VictimPlayerState = VictimController ? Cast<AMuzimiPlayerState>(VictimController->PlayerState) : nullptr;
	AMuzimiPlayerState* AttackerPlayerState = AttackerController ? Cast<AMuzimiPlayerState>(AttackerController->PlayerState) : nullptr;
	AMuzimiGameState* MuzimiGameState = GetGameState<AMuzimiGameState>();
	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		TArray<AMuzimiPlayerState*> PlayersCurrentlyInTheLead;
		for (auto LeadPlayer : MuzimiGameState->TopScoringPlayers)
		{
			PlayersCurrentlyInTheLead.Add(LeadPlayer);
		}
		AttackerPlayerState->AddToScore(1.f);
		MuzimiGameState->UpdateTopScoringPlayers(AttackerPlayerState);
		if (MuzimiGameState->TopScoringPlayers.Contains(AttackerPlayerState))
		{
			AMuzimiCharacter* Leader = Cast<AMuzimiCharacter>(AttackerPlayerState->GetPawn());
			if (Leader)
			{
				Leader->MulticastGainedTheLead();
			}
		}
		for (int32 i = 0; i < PlayersCurrentlyInTheLead.Num(); i++)
		{
			if (!MuzimiGameState->TopScoringPlayers.Contains(PlayersCurrentlyInTheLead[i]))
			{
				AMuzimiCharacter* Loser = Cast<AMuzimiCharacter>(PlayersCurrentlyInTheLead[i]->GetPawn());
				if (Loser)
				{
					Loser->MulticastLostTheLead();
				}
			}
		}
	}
	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim(false);
	}
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AMuzimiPlayerController* MuzimiPlayer = Cast<AMuzimiPlayerController>(*It);
		if (MuzimiPlayer && VictimPlayerState && AttackerPlayerState)
		{
			MuzimiPlayer->BroadcastElim(AttackerPlayerState, VictimPlayerState);
		}
	}
}

void AMuzimiGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}

float AMuzimiGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	return BaseDamage;
}

void AMuzimiGameMode::PlayerLeftGame( AMuzimiPlayerState* PlayerLeaving)
{
	if (PlayerLeaving == nullptr) return;
	AMuzimiGameState* MuzimiGameState = GetGameState<AMuzimiGameState>();
	if (MuzimiGameState && MuzimiGameState->TopScoringPlayers.Contains(PlayerLeaving))
	{
		MuzimiGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}
	AMuzimiCharacter* CharacterLeaving = Cast<AMuzimiCharacter>(PlayerLeaving->GetPawn());
	if (CharacterLeaving)
	{
		CharacterLeaving->Elim(true);
	}
}

void AMuzimiGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void AMuzimiGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		AMuzimiPlayerController* MuzimiPlayerController = Cast<AMuzimiPlayerController>(*Iterator);
		if (MuzimiPlayerController)
		{
			AMuzimiCharacter* ControlledCharacter = Cast<AMuzimiCharacter>(MuzimiPlayerController->GetPawn());
			if (ControlledCharacter && ControlledCharacter->DefaultMappingContext)
			{
				UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(MuzimiPlayerController->GetLocalPlayer());
				if (Subsystem)
				{
					Subsystem->AddMappingContext(ControlledCharacter->DefaultMappingContext, 0);
				}
			}
			MuzimiPlayerController->OnMatchStateSet(MatchState,bTeamMatch);
		}
	}
}
