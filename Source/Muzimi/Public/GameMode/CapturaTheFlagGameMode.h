// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameMode/TeamGameMode.h"
#include "CapturaTheFlagGameMode.generated.h"

/**
 * 
 */
UCLASS()
class MUZIMI_API ACapturaTheFlagGameMode : public ATeamGameMode
{
	GENERATED_BODY()
public:
	virtual void PlayerEliminated(class AMuzimiCharacter* ElimmedCharacter, class AMuzimiPlayerController* VictimController, class AMuzimiPlayerController* AttackerController);
	void FlagCaptured(class AFlag* Flag, class AFlagZone* Zone);
protected:

private:
};
