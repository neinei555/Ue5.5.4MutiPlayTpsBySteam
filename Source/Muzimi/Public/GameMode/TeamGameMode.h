// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameMode/MuzimiGameMode.h"
#include "TeamGameMode.generated.h"

/**
 * 
 */
UCLASS()
class MUZIMI_API ATeamGameMode : public AMuzimiGameMode
{
	GENERATED_BODY()
public:
	ATeamGameMode();
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void PlayerEliminated(class AMuzimiCharacter* ElimmedCharacter, class AMuzimiPlayerController* VictimController, class AMuzimiPlayerController* AttackerController);

	virtual void Logout(AController* Exiting) override;
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage);
protected:
	virtual void HandleMatchHasStarted() override;
private:

};
