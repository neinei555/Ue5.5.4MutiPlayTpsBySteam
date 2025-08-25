// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/TeamGameMode.h"
#include "GameState/MuzimiGameState.h"
#include "PlayerState/MuzimiPlayerState.h"
#include "Character/MuzimiCharacter.h"
#include "PlayerController/MuzimiPlayerController.h"
#include "Kismet/GameplayStatics.h"
ATeamGameMode::ATeamGameMode()
{
	bTeamMatch = true;
}
void ATeamGameMode::PlayerEliminated(class AMuzimiCharacter* ElimmedCharacter, class AMuzimiPlayerController* VictimController, class AMuzimiPlayerController* AttackerController)
{
	Super::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);
	if (VictimController == nullptr || AttackerController == nullptr) return;
	AMuzimiGameState* MGameState = Cast<AMuzimiGameState>(UGameplayStatics::GetGameState(this));
	AMuzimiPlayerState* VictimPlayerState = VictimController ? Cast<AMuzimiPlayerState>(VictimController->PlayerState) : nullptr;
	AMuzimiPlayerState* AttackerPlayerState = AttackerController ? Cast<AMuzimiPlayerState>(AttackerController->PlayerState) : nullptr;
	if (VictimPlayerState == nullptr || AttackerPlayerState == nullptr) return;
	if (VictimPlayerState == AttackerPlayerState) return;
	if (VictimPlayerState->GetTeam() == AttackerPlayerState->GetTeam()) return;
	if (AttackerPlayerState)
	{
		if(AttackerPlayerState->GetTeam()==ETeam::ET_RedTeam)
		{
			if(MGameState)
			{
				MGameState->RedTeamScores();
			}
		}
		else if(AttackerPlayerState->GetTeam() == ETeam::ET_BlueTeam)
		{
			if (MGameState)
			{
				MGameState->BlueTeamScores();
			}
		}
	}


}
void ATeamGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	AMuzimiGameState* MGameState = Cast<AMuzimiGameState>(UGameplayStatics::GetGameState(this));
	if (MGameState)
	{
		AMuzimiPlayerState* MPState = NewPlayer->GetPlayerState<AMuzimiPlayerState>();
			if (MPState && MPState->GetTeam() == ETeam::ET_NoTeam)
			{
				if (MGameState->RedTeam.Num() < MGameState->BlueTeam.Num())
				{
					MPState->SetTeam(ETeam::ET_RedTeam);
					MGameState->RedTeam.AddUnique(MPState);
				}
				else
				{
					MPState->SetTeam(ETeam::ET_BlueTeam);
					MGameState->BlueTeam.AddUnique(MPState);
				}
			}
	}
}
float ATeamGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	AMuzimiPlayerState* VictimPlayerState = Victim ? Cast<AMuzimiPlayerState>(Victim->PlayerState) : nullptr;
	AMuzimiPlayerState* AttackerPlayerState = Attacker ? Cast<AMuzimiPlayerState>(Attacker->PlayerState) : nullptr;
	if (AttackerPlayerState == nullptr || VictimPlayerState == nullptr) return BaseDamage;
	if (AttackerPlayerState == VictimPlayerState)
	{
		return BaseDamage;
	}
	if(AttackerPlayerState->GetTeam() == VictimPlayerState->GetTeam())
	{
		return 0.f;
	}
	return BaseDamage;
}

void ATeamGameMode::Logout(AController* Exiting)
{

	AMuzimiGameState* MGameState = Cast<AMuzimiGameState>(UGameplayStatics::GetGameState(this));
	if (MGameState)
	{
		AMuzimiPlayerState* MPState = Exiting->GetPlayerState<AMuzimiPlayerState>();
		if (MPState)
		{
			if(MPState->GetTeam() == ETeam::ET_RedTeam)
			{
				MGameState->RedTeam.Remove(MPState);
			}
			else if(MPState->GetTeam() == ETeam::ET_BlueTeam)
			{
				MGameState->BlueTeam.Remove(MPState);
			}
		}
	}
}
void ATeamGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	AMuzimiGameState* MGameState = Cast<AMuzimiGameState>(UGameplayStatics::GetGameState(this));
	if (MGameState)
	{
		for (auto PState : MGameState->PlayerArray)
		{
			AMuzimiPlayerState* MPState = Cast<AMuzimiPlayerState>(PState);
			if(MPState && MPState->GetTeam()== ETeam::ET_NoTeam)
			{
				if (MGameState->RedTeam.Num() <= MGameState->BlueTeam.Num())
				{
					MPState->SetTeam(ETeam::ET_RedTeam);
					MGameState->RedTeam.AddUnique(MPState);
				}
				else
				{
					MPState->SetTeam(ETeam::ET_BlueTeam);
					MGameState->BlueTeam.AddUnique(MPState);
				}
			}
		}
	}
}
