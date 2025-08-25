// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "MuzimiGameState.generated.h"

/**
 * 
 */
UCLASS()
class MUZIMI_API AMuzimiGameState : public AGameState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void UpdateTopScoringPlayers(class AMuzimiPlayerState* ScoringPlayer);
	UPROPERTY(Replicated)
	TArray<class AMuzimiPlayerState*> TopScoringPlayers;

	/**
	* Teams
	*/

	void RedTeamScores();
	void BlueTeamScores();

	TArray<AMuzimiPlayerState*> RedTeam;
	TArray<AMuzimiPlayerState*> BlueTeam;

	UPROPERTY(ReplicatedUsing=OnRep_RedTeamScore)
	float RedTeamScore = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_BlueTeamScore)
	float BlueTeamScore = 0.f;

	UFUNCTION()
	void OnRep_RedTeamScore();
	UFUNCTION()
	void OnRep_BlueTeamScore();

private:
	float TopScore = 0.f;
};
