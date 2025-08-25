// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/ShieldPickup.h"
#include "Character/MuzimiCharacter.h"
#include "MuzimiComponents/CombatComponent.h"

AShieldPickup::AShieldPickup()
{
	bReplicates = true;
}

void AShieldPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	AMuzimiCharacter* MuzimiCharacter = Cast<AMuzimiCharacter>(OtherActor);
	if (MuzimiCharacter)
	{
		UBuffComponent* Buff = MuzimiCharacter->GetBuff();
		if (Buff)
		{
			Buff->ShieldReplenish(ShieldReplenishAmount, ShieldReplenishTime);
		}
	}
	Destroy();
}
