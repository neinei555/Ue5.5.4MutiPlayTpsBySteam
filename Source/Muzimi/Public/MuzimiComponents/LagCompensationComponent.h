// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Weapon/Projectile.h"
#include "LagCompensationComponent.generated.h"

USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfor;

	UPROPERTY()
	AMuzimiCharacter* Character;
};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;

	UPROPERTY()
	bool bHeadShot;
};

USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<AMuzimiCharacter*, uint32> HeadShots;
	UPROPERTY()
	TMap<AMuzimiCharacter*, uint32> BodyShots;
};
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MUZIMI_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULagCompensationComponent();
	friend class AMuzimiCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void SaveFramePackage();
	void ShowFramePackage(const FFramePackage& Package, FColor Color);
	/**
	* Hitscan
	*/
	FServerSideRewindResult ServerSideRewind(
		class AMuzimiCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime
	);

	FServerSideRewindResult ConfirmHit(
		const FFramePackage& Package,
		AMuzimiCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation
	);
	/**
	* Projectile
	*/

	UFUNCTION(Server, Reliable)
	void ServerProjectileScoreRequest(
		AMuzimiCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime,
		class AWeapon* DamageCauser
	);
	FServerSideRewindResult ServerProjectileSideRewind(
		class AMuzimiCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
	);

	FServerSideRewindResult ProjectileConfirmHit(
		const FFramePackage& Package,
		AMuzimiCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
	);

	UFUNCTION(Server,Reliable)
	void ServerScoreRequest(
		AMuzimiCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime,
	    AWeapon* DamageCauser
	);
	UFUNCTION(Server, Reliable)
	void ServerShotgunScoreRequest(
		const TArray<AMuzimiCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime
	);
	void CacheBoxPositions(AMuzimiCharacter* HitCharacter, FFramePackage& OutFramePackage);
	void MoveBoxes(AMuzimiCharacter* HitCharacter,const FFramePackage& Package);
	void ResetHitBoxes(AMuzimiCharacter* HitCharacter,const FFramePackage& Package);
	void EnableCharacterMeshCollision(AMuzimiCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);
	FFramePackage GetFrameToCheck(AMuzimiCharacter* HitCharacter, float HitTime);

	/**
	* Shotgun
	*/
	FShotgunServerSideRewindResult ShotgunServerSideRewind(
		const TArray<AMuzimiCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime
	);
	FShotgunServerSideRewindResult ShotgunConfirmHit(
		const TArray<FFramePackage>& FramePackages,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations
	);
protected:
	virtual void BeginPlay() override;
	void SaveFramePackage(FFramePackage& Package);
	FFramePackage InterBetweenFrame(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);

private:

	UPROPERTY()
	AMuzimiCharacter* Character;

	UPROPERTY()
	class AMuzimiPlayerController* Controller;

	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 0.5f;
public:	
		
};
