// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/MuzimiGameState.h"
#include "Net/UnrealNetwork.h"
#include "PlayerController/MuzimiPlayerController.h"
#include "PlayerState/MuzimiPlayerState.h"

void AMuzimiGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMuzimiGameState, TopScoringPlayers)
	DOREPLIFETIME(AMuzimiGameState, RedTeamScore);
	DOREPLIFETIME(AMuzimiGameState, BlueTeamScore);
}

void AMuzimiGameState::UpdateTopScoringPlayers(class AMuzimiPlayerState* ScoringPlayer)
{
	if (TopScoringPlayers.Num() == 0)
	{
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if (ScoringPlayer->GetScore() == TopScore)
	{
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}
	else if (ScoringPlayer->GetScore() > TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
}

void AMuzimiGameState::RedTeamScores()
{
	++RedTeamScore;

	AMuzimiPlayerController* MPlayer = Cast<AMuzimiPlayerController>(GetWorld()->GetFirstPlayerController());
	if (MPlayer)
	{
		MPlayer->SetHUDRedTeamScore(RedTeamScore);
	}
}

void AMuzimiGameState::BlueTeamScores()
{
	++BlueTeamScore;

	AMuzimiPlayerController* MPlayer = Cast<AMuzimiPlayerController>(GetWorld()->GetFirstPlayerController());
	if (MPlayer)
	{
		MPlayer->SetHUDBlueTeamScore(BlueTeamScore);
	}
}

void AMuzimiGameState::OnRep_RedTeamScore()
{
	AMuzimiPlayerController* MPlayer = Cast<AMuzimiPlayerController>(GetWorld()->GetFirstPlayerController());
	if (MPlayer)
	{
		MPlayer->SetHUDRedTeamScore(RedTeamScore);
	}
}

void AMuzimiGameState::OnRep_BlueTeamScore()
{
	AMuzimiPlayerController* MPlayer = Cast<AMuzimiPlayerController>(GetWorld()->GetFirstPlayerController());
	if (MPlayer)
	{
		MPlayer->SetHUDBlueTeamScore(BlueTeamScore);
	}
}
