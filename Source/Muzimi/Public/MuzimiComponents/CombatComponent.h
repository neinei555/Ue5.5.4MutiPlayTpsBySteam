
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HUD/MuzimiHUD.h"
#include "Weapon/WeaponTypes.h"
#include "MuzimiTypes/CombatStates.h"
#include "CombatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MUZIMI_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	friend class AMuzimiCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UFUNCTION(BlueprintCallable)
	void FinishReloading();
	
	UFUNCTION(BlueprintCallable)
	void FinishSwap();

	UFUNCTION(BlueprintCallable)
	void FinishSwapAttachWeapon();
	void FireButtonPressed(bool bPressed);

	UFUNCTION(BlueprintCallable)
	void GrenadeLauncher();

	UFUNCTION(Server,Reliable)
	void ServerGrenadeLauncher(const FVector_NetQuantize& Target);

	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();

	void JumpToShotgunEnd();

	void ThrowGrenade();

	UFUNCTION(Server,Reliable)
	void ServerThrowGrenade();

	UPROPERTY(EditAnywhere)
	int32 MaxCarriedAmmo = 500;

	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();

	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount);

	int32 GetCarriedAmmo();

	bool ShouldSwapWeapon();

	void LocalFire(const FVector_NetQuantize& TraceHitTarget);
	void ShotgunLocalFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	bool bLocallyReloading = false;
protected:
	virtual void BeginPlay() override;

	void EquipWeapon(class AWeapon* WeaponToEquip);
	void SwapWeapon();
	void ReloadEmptyWeapon();
	void PlayEquipWeaponSound(AWeapon* WeaponToEquip);
	void UpdateCarriedAmmo();
	void AttachActorToRightHand(AActor* ActorToAttach);
	void AttachFlagToLeftHand(AWeapon* Flag);
	void AttachActorToLeftHand(AActor* ActorToAttach);
	void AttachActorToBackpack(AActor* ActorToAttach);
	void DropEquippedWeapon();
	void SetAiming(bool bIsAiming);
	
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool IsAiming);
	
	UFUNCTION()
	void OnRep_EquippedWeapon();

	void Fire();

	void FireProjectileWeapon();
	void FireHitScanWeapon();
	void FireShotgun();

	bool bFireButtonPressed;
	
	void ShowAttachedGrenade(bool bShowGrenade);

	UFUNCTION(Server,Reliable,WithValidation)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget,float FireDelay);

	UFUNCTION(NetMulticast,Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);


	UFUNCTION(Server, Reliable,WithValidation)
	void ServerShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);
    
	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);

	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);

	bool CanFire();

	// Carried ammo for the currently_equipped weapon
	UPROPERTY(ReplicatedUsing= OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	TMap<EWeaponType, int32> CarriedAmmoMap;

	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 30;

	UPROPERTY(EditAnywhere)
	int32 StartingRocketAmmo = 30;

	UPROPERTY(EditAnywhere)
	int32 StartingPistolAmmo = 30;

	UPROPERTY(EditAnywhere)
	int32 StartingSMGAmmo = 30;

	UPROPERTY(EditAnywhere)
	int32 StartingShotgunAmmo = 30;

	UPROPERTY(EditAnywhere)
	int32 StartingSniperAmmo = 30;

	UPROPERTY(EditAnywhere)
	int32 StartingGrenadeLauncherAmmo = 30;

	void InitializeCarriedAmmo();

	UPROPERTY(ReplicatedUsing= OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	int32 AmountToReload();

	void HandleReload();

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile>GrenadeClass;
private:
	class AMuzimiCharacter* Character;
	class AMuzimiHUD* HUD;
	class AMuzimiPlayerController* PlayerController;
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	class AWeapon* EquippedWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	class AWeapon* SecondaryWeapon;

	UFUNCTION()
	void OnRep_SecondaryWeapon();

	UPROPERTY(ReplicatedUsing = OnRep_Aming)
	bool bAiming=false;

	bool bAimButtonPressed = false;
	UFUNCTION()
	void OnRep_Aming();
	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	/**
	* HUD and crosshairs
	*/

	FHUDPackage HUDPackage;
	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;

	FVector HitTarget;

	/**
	* Aiming and FOV
	*/
	// Filed of view not aiming; setthecamra to the camera's base in BeginPlay
	float DefaultFOV;

	UPROPERTY(EditAnywhere,Category=Combat)
	float ZoomedFOV = 30.f;

	float CurrentFOV;

	void Reload();

	UFUNCTION(Server,Reliable)
	void ServerReload();

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 20.f;

	void InterpFOV(float DeltaTime);
	/**
	* Automatic fire
	*/
	FTimerHandle FireTimer;
	bool bCanFire = true;
	void StartFireTimer();
	void FireTimerFinished();

	void UpdateAmmoValues();
	void UpdateShotgunAmmoValues();

	UPROPERTY(ReplicatedUsing=OnRep_Grenades)
	int32 Grenades = 4;

	UFUNCTION()
	void OnRep_Grenades();

	UPROPERTY(EditAnywhere)
	int32 MaxGrenades = 4;

	void UpdateHUDGrenades();

	UPROPERTY(ReplicatedUsing=OnRep_HoldingTheFlag)
	bool bHoldingTheFlag = false;

	UFUNCTION()
	void OnRep_HoldingTheFlag();
public:
	FORCEINLINE int32 GetGrenades() const { return Grenades; }
	FORCEINLINE AWeapon* GetEquippedWeapon()const { return EquippedWeapon;}
};
