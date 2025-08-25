
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MUZIMI_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBuffComponent();
	friend class AMuzimiCharacter;
	void Heal(float HealAmount, float HealingTime);
	void ShieldReplenish(float ShieldReplenishAmount, float ShieldReplenishTime);
	void SetInitialSpeeds(float BaseSpeed, float CrouchSpeed);
	void BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);
	void SetInitialJumpVelocity(float Velocity);
	void BuffJump(float BuffJumpVelocity, float BuffTime);
protected:
	virtual void BeginPlay() override;
	void HealRampUp(float DeltaTime);
	void ShieldReplenishRampUp(float DeltaTime);
private:
	UPROPERTY()
	class AMuzimiCharacter* Character;

	/**
	* Heal buff
	*/
	bool bHealing = false;
	float HealingRate = 0;
	float AmountToHeal = 0.f;

	/**
	* Shield buff
	*/
	bool  bShieldReplenish = false;
	float ShieldReplenishRate = 0;
	float AmountToShieldReplenish = 0.f;

	/**
	* Speed buff
	*/
	FTimerHandle SpeedBuffTimer;
	void ResetSpeeds();
	float InitialBaseSpeed;
	float InitialCrouchSpeed;
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float BaseSpeed, float CrouchSpeed);
	/**
	* Jump buff
	*/
	FTimerHandle JumpBuffTimer;
	void ResetJump();
	float InitialJumpVelocity;
	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpBuff(float JumpVelocity);
public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
