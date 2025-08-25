// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class MUZIMI_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();

	/**
	* used with server-side rewind
	*/
	UPROPERTY(EditAnywhere)
	bool bUseServerSideRewind = false;
	FVector_NetQuantize TraceStart;
	FVector_NetQuantize InitialVelocity;


	UPROPERTY(EditAnywhere)
	float InitialSpeed = 15000.f;

	//Only set this for Grenades and Rockets
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
	float Damage = 20.f;

	// Doesn't matter for Grenades and Rockets
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
	float HeadShotDamage = 40.f;
protected:
	virtual void BeginPlay() override;
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	virtual void Destroyed() override;

	void ExplodeDamage();
	UPROPERTY(EditAnywhere, Category = "Trail System")
	class UNiagaraSystem* TrailSystem;

	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;

	void SpawnTrailSystem();

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticle;

	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;


	void StartDestroyTimer();
	void DestroyTimerFinished();

	UPROPERTY(VisibleAnywhere, Category = "Weapon Projectile")
	class UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(EditAnywhere)
	float DamageInnerRadius = 200.f;
	UPROPERTY(EditAnywhere)
	float DamageOuterRadius = 500.f;


private:

	UPROPERTY(EditAnywhere)
	class UParticleSystem* Tracer;
	UPROPERTY(EditAnywhere)
	class UParticleSystemComponent* TracerComponent;

	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.f;


public:	
	virtual void Tick(float DeltaTime) override;

};
