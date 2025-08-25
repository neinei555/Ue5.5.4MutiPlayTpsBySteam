// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/CapturaTheFlagGameMode.h"
#include "Weapon/Flag.h"
#include "CaptureTheFlag/FlagZone.h"
#include "GameState/MuzimiGameState.h"

void ACapturaTheFlagGameMode::PlayerEliminated(class AMuzimiCharacter* ElimmedCharacter, class AMuzimiPlayerController* VictimController, class AMuzimiPlayerController* AttackerController)
{
	AMuzimiGameMode::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);
}

void ACapturaTheFlagGameMode::FlagCaptured(AFlag* Flag, AFlagZone* Zone)
{
	bool bValidCapture = Flag->Team == Zone->Team;
	if (!bValidCapture) return;
	AMuzimiGameState* MGameState = Cast<AMuzimiGameState>(GameState);
	if (MGameState)
	{
		if (Zone->Team == ETeam::ET_BlueTeam)
		{
			MGameState->BlueTeamScores();
		}
		if(Zone->Team == ETeam::ET_RedTeam)
		{
			MGameState->RedTeamScores();
		}
           
	}
}
