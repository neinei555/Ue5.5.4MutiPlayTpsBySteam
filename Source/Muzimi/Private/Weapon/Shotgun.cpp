// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Character/MuzimiCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h" 
#include "MuzimiComponents/LagCompensationComponent.h"
#include "PlayerController/MuzimiPlayerController.h"
#include "Kismet/KismetMathLibrary.h"


void AShotgun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	AWeapon::Fire(FVector());
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		TMap<AMuzimiCharacter*, uint32>HitMap;
		TMap<AMuzimiCharacter*, uint32> HeadShotHitMap;
		
		for (FVector_NetQuantize HitTarget : HitTargets)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);
			AMuzimiCharacter* MuzimiCharacter = Cast<AMuzimiCharacter>(FireHit.GetActor());
			if (MuzimiCharacter)
			{
				const bool bHeadShot = FireHit.BoneName.ToString() == FString("head");
				if (bHeadShot)
				{
					if (HeadShotHitMap.Contains(MuzimiCharacter)) HeadShotHitMap[MuzimiCharacter]++;
					else HeadShotHitMap.Emplace(MuzimiCharacter, 1);
				}
				else
				{
					if (HitMap.Contains(MuzimiCharacter)) HitMap[MuzimiCharacter]++;
					else HitMap.Emplace(MuzimiCharacter, 1);
				}


			}
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ImpactParticles,
					FireHit.ImpactPoint,
					FireHit.ImpactNormal.Rotation()
				);
			}
			if (HitSound)
			{
				UGameplayStatics::PlaySoundAtLocation(
					this,
					HitSound,
					FireHit.ImpactPoint,
					.5f,
					FMath::FRandRange(-.5f, .5f)
				);
			}

		}
		TArray<AMuzimiCharacter*> HitCharacters;
		TMap<AMuzimiCharacter*, float> DamageMap;
		for (auto HitPair : HitMap)
		{
			if (HitPair.Key )
			{
				DamageMap.Emplace(HitPair.Key, HitPair.Value * Damage);
				HitCharacters.AddUnique(HitPair.Key);
			}
		}
		for (auto HeadShotHitPair : HeadShotHitMap)
		{
			if (HeadShotHitPair.Key)
			{
				if (DamageMap.Contains(HeadShotHitPair.Key))  DamageMap[HeadShotHitPair.Key] += HeadShotHitPair.Value * HeadShotDamage;
				else  DamageMap.Emplace(HeadShotHitPair.Key, HeadShotHitPair.Value * HeadShotDamage);
				HitCharacters.AddUnique(HeadShotHitPair.Key);
			}
		}
		for (auto DamagePair : DamageMap)
		{
			if (DamagePair.Key&&InstigatorController)
			{
				bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
				if (HasAuthority() && bCauseAuthDamage)
				{
					UGameplayStatics::ApplyDamage(
						DamagePair.Key,
						DamagePair.Value,
						InstigatorController,
						this,
						UDamageType::StaticClass()
					);
				}
			}
		}
		if (!HasAuthority() && bUseServerSideRewind)
		{
			MuzimiOwnerCharacter = MuzimiOwnerCharacter == nullptr ? Cast<AMuzimiCharacter>(OwnerPawn) : MuzimiOwnerCharacter;
			MuzimiOwnerController = MuzimiOwnerController == nullptr ? Cast<AMuzimiPlayerController>(InstigatorController) : MuzimiOwnerController;
			if (MuzimiOwnerController && MuzimiOwnerCharacter && MuzimiOwnerCharacter->GetLagCompensation() && MuzimiOwnerCharacter->IsLocallyControlled())
			{
				MuzimiOwnerCharacter->GetLagCompensation()->ServerShotgunScoreRequest(
					HitCharacters,
					Start,
					HitTargets,
					MuzimiOwnerController->GetServerTime() - MuzimiOwnerController->SingleTripTime
				);
			}
		}
	}
}

void AShotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return ;

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	for (uint32 i = 0; i < NumberOfPellets; i++)
	{
	const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	const FVector EndLoc = SphereCenter + RandVec;
	FVector ToEndLoc = EndLoc - TraceStart;
	ToEndLoc = TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size();
	HitTargets.Add(ToEndLoc);
	}
	return;


}

