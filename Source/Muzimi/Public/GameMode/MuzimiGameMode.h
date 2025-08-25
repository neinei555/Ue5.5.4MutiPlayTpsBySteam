// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MuzimiGameMode.generated.h"


namespace MatchState
{
	extern MUZIMI_API const FName Cooldown;// Match duration has been reached. Display winner and begin cooldown timer. 
}
/**
 * 
 */
UCLASS()
class MUZIMI_API AMuzimiGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	AMuzimiGameMode();
	virtual void RestartGame() override;
	virtual void HandleSeamlessTravelPlayer(AController*& C) override;
	virtual void Tick(float DeltaTime) override;
	virtual void PlayerEliminated(class AMuzimiCharacter* ElimmedCharacter, class AMuzimiPlayerController* VictimController, class AMuzimiPlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* ElimmedCharacter,AController* ElimmedController);
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage);
	void PlayerLeftGame( class AMuzimiPlayerState* PlayerLeaving);
	bool bTeamMatch = false;
	UPROPERTY(EditAnywhere)
	float WarmupTime = 5.f;

	UPROPERTY(EditAnywhere)
	float MatchTime = 10.f;

	UPROPERTY(EditAnywhere)
	float CooldownTime = 5.f;

	float LevelStartingTime = 0.f;
protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;
private:
	float CountdownTime = 0.f;
public:
	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
};
