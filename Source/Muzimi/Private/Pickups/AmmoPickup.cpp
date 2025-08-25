// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/AmmoPickup.h"
#include "Character/MuzimiCharacter.h"
#include "MuzimiComponents/CombatComponent.h"
void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
    
	AMuzimiCharacter* MuzimiCharacter = Cast<AMuzimiCharacter>(OtherActor);
	if (MuzimiCharacter)
	{
		UCombatComponent* Combat = MuzimiCharacter->GetCombat();
		if (Combat)
		{
			Combat->PickupAmmo(WeaponType, AmmoAmount);
		}
	}
	Destroy();
}
