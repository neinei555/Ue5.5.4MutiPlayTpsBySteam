// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerState/MuzimiPlayerState.h"
#include "PlayerController/MuzimiPlayerController.h"
#include "Character/MuzimiCharacter.h"
#include "Net/UnrealNetwork.h"

void AMuzimiPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMuzimiPlayerState, Defeats);
	DOREPLIFETIME(AMuzimiPlayerState, Team);
}

void AMuzimiPlayerState::OnRep_Score()
{
	Super::OnRep_Score();
	MuzimiCharacter = MuzimiCharacter == nullptr ? Cast<AMuzimiCharacter>(GetPawn()) : MuzimiCharacter;
	if (MuzimiCharacter)
	{
		MuzimiPlayerController = MuzimiPlayerController == nullptr ? Cast<AMuzimiPlayerController>(GetPlayerController()) : MuzimiPlayerController;
		if (MuzimiPlayerController)
		{
			MuzimiPlayerController->SetHUDScore(GetScore());
		}
	}
}

void AMuzimiPlayerState::OnRep_Defeats()
{
	MuzimiCharacter = MuzimiCharacter == nullptr ? Cast<AMuzimiCharacter>(GetPawn()) : MuzimiCharacter;
	if (MuzimiCharacter)
	{
		MuzimiPlayerController = MuzimiPlayerController == nullptr ? Cast<AMuzimiPlayerController>(GetPlayerController()) : MuzimiPlayerController;
		if (MuzimiPlayerController)
		{
			MuzimiPlayerController->SetHUDDefeats(Defeats);
		}
	}
}

void AMuzimiPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);
	MuzimiCharacter = MuzimiCharacter == nullptr ? Cast<AMuzimiCharacter>(GetPawn()) : MuzimiCharacter;
	if (MuzimiCharacter)
	{
		MuzimiPlayerController = MuzimiPlayerController == nullptr ? Cast<AMuzimiPlayerController>(GetPlayerController()) : MuzimiPlayerController;
		if (MuzimiPlayerController)
		{
			MuzimiPlayerController->SetHUDScore(GetScore());
		}
	}
}

void AMuzimiPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats += DefeatsAmount;
	MuzimiCharacter = MuzimiCharacter == nullptr ? Cast<AMuzimiCharacter>(GetPawn()) : MuzimiCharacter;
	if (MuzimiCharacter)
	{
		MuzimiPlayerController = MuzimiPlayerController == nullptr ? Cast<AMuzimiPlayerController>(GetPlayerController()) : MuzimiPlayerController;
		if (MuzimiPlayerController)
		{
			MuzimiPlayerController->SetHUDDefeats(Defeats);
		}
	}
}
