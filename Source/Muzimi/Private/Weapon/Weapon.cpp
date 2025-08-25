// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Character/MuzimiCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Weapon/Casing.h"
#include "PlayerController/MuzimiPlayerController.h"
#include "MuzimiComponents/CombatComponent.h"
#include "Kismet/KismetMathLibrary.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Ignore);

	EnableCustomDepth(true);
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE);
	WeaponMesh->MarkRenderStateDirty();

	AreaCollision = CreateDefaultSubobject<USphereComponent>(TEXT("AreaCollision"));
	AreaCollision->SetupAttachment(RootComponent);
	AreaCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AreaCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget); 
	}
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	if (Owner == nullptr)
	{
		MuzimiOwnerCharacter = nullptr;
		MuzimiOwnerController = nullptr;
		return;
	}
	else
	{
		MuzimiOwnerCharacter = MuzimiOwnerCharacter == nullptr ? Cast<AMuzimiCharacter>(GetOwner()) : MuzimiOwnerCharacter;
		if (MuzimiOwnerCharacter && MuzimiOwnerCharacter->GetEquippedWeapon() && MuzimiOwnerCharacter->GetEquippedWeapon() == this)
		{
			SetHUDAmmo();
		}
	}
}

FVector AWeapon::TraceEndWithScatter(const FVector& HitTarget)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return FVector();

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	const FVector EndLoc = SphereCenter + RandVec;
	const FVector ToEndLoc = EndLoc - TraceStart;

	return FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());
}

void AWeapon::OnPingTooHigh(bool bPingTooHigh)
{
	bUseServerSideRewind = !bPingTooHigh;

}


void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME_CONDITION(AWeapon, bUseServerSideRewind,COND_OwnerOnly);
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}
	if (CasingClass)
	{
		APawn* InstigatorPawn = Cast<APawn>(GetOwner());
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));
		if (AmmoEjectSocket)
		{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);
				UWorld* World = GetWorld();
				if (World)
				{
					World->SpawnActor<ACasing>(
						CasingClass,
						SocketTransform.GetLocation(),
						SocketTransform.GetRotation().Rotator()
					);
				}
		}
	}
		SpendRound();
}

void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr); 
	MuzimiOwnerCharacter = nullptr;
	MuzimiOwnerController = nullptr;
}

void AWeapon::EnableCustomDepth(bool bEnable)
{
	if (WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	AreaCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaCollision->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Overlap);
	AreaCollision->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
	AreaCollision->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
}


void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AMuzimiCharacter* MuzimiCharacter = Cast<AMuzimiCharacter>(OtherActor);
	if (MuzimiCharacter)
	{
		if (WeaponType == EWeaponType::EWT_Flag && MuzimiCharacter->GetTeam() != Team)return;
		if (MuzimiCharacter->IsHoldingTheFlag())return;
		MuzimiCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AMuzimiCharacter* MuzimiCharacter = Cast<AMuzimiCharacter>(OtherActor);
	if (MuzimiCharacter)
	{
		if (WeaponType == EWeaponType::EWT_Flag && MuzimiCharacter->GetTeam() != Team)return;
		if (MuzimiCharacter->IsHoldingTheFlag())return;
		MuzimiCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void AWeapon::SetHUDAmmo()
{
	MuzimiOwnerCharacter = MuzimiOwnerCharacter == nullptr ? Cast<AMuzimiCharacter>(GetOwner()) : MuzimiOwnerCharacter;
	if (MuzimiOwnerCharacter)
	{
		MuzimiOwnerController = MuzimiOwnerController == nullptr ? Cast<AMuzimiPlayerController>(MuzimiOwnerCharacter->GetController()) : MuzimiOwnerController;
		if (MuzimiOwnerController)
		{
			MuzimiOwnerController->SetHUDWeaponAmmo(Ammo, MagCapacity);
		}
	}
}

void AWeapon::AddAmmo(int32 Amount)
{
	if (MuzimiOwnerController == nullptr) return;
	Ammo = FMath::Clamp(Ammo + Amount, 0, MagCapacity);
	MuzimiOwnerController->SetHUDWeaponAmmo(Ammo, MagCapacity);
	ClientAddAmmo(Amount);
}
void AWeapon::SpendRound()
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
	SetHUDAmmo();
	if (HasAuthority())
	{
		ClientUpdateAmmo(Ammo);
	}
	else
	{
		++Sequence;
	}
}
void AWeapon::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
	if (HasAuthority())return;
	Ammo = ServerAmmo;
	--Sequence;
	Ammo -= Sequence;
	SetHUDAmmo();
}

void AWeapon::ClientAddAmmo_Implementation(int32 AmmoToAdd)
{
	if (HasAuthority())return;
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	MuzimiOwnerCharacter = MuzimiOwnerCharacter == nullptr ? Cast<AMuzimiCharacter>(GetOwner()) : MuzimiOwnerCharacter;
	if (MuzimiOwnerCharacter && MuzimiOwnerCharacter->GetCombat()&& IsAmmoFull())
	{
		MuzimiOwnerCharacter->GetCombat()->JumpToShotgunEnd();
	}
	SetHUDAmmo();
}


void AWeapon::SetWeaponState(EWeaponState State)
{
	if (!IsValid(this)) return;

	if (!WeaponMesh) return;

	WeaponState = State;
	OnWeaponStateSet();
	
}
void AWeapon::OnWeaponStateSet()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		OnEquipped();
		break;
	case EWeaponState::EWS_EquippedSecondary:
		OnEquippedSecondary();
		break;
	case EWeaponState::EWS_Dropped:
		OnDropped();
		break;
	}
}

void AWeapon::OnEquipped()
{
	ShowPickupWidget(false);
	AreaCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (WeaponType == EWeaponType::EWT_SubmachineGun)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
	EnableCustomDepth(false);

	MuzimiOwnerCharacter = MuzimiOwnerCharacter == nullptr ? Cast<AMuzimiCharacter>(GetOwner()) : MuzimiOwnerCharacter;
	if (MuzimiOwnerCharacter && bUseServerSideRewind)
	{
		MuzimiOwnerController = MuzimiOwnerController == nullptr ? Cast<AMuzimiPlayerController>(MuzimiOwnerCharacter->GetController()) : MuzimiOwnerController;
		if (MuzimiOwnerController && HasAuthority() && !MuzimiOwnerController->HighPingDelegate.IsBound())
		{
			MuzimiOwnerController->HighPingDelegate.AddDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

void AWeapon::OnDropped()
{
	if (HasAuthority())
	{
		AreaCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

	MuzimiOwnerCharacter = MuzimiOwnerCharacter == nullptr ? Cast<AMuzimiCharacter>(GetOwner()) : MuzimiOwnerCharacter;
	if (MuzimiOwnerCharacter && bUseServerSideRewind)
	{
		MuzimiOwnerController = MuzimiOwnerController == nullptr ? Cast<AMuzimiPlayerController>(MuzimiOwnerCharacter->GetController()) : MuzimiOwnerController;
		if (MuzimiOwnerController && HasAuthority() && MuzimiOwnerController->HighPingDelegate.IsBound())
		{
			MuzimiOwnerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}
void AWeapon::OnEquippedSecondary()
{
	ShowPickupWidget(false);
	AreaCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (WeaponType == EWeaponType::EWT_SubmachineGun)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
	if (WeaponMesh)
	{
		WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
		WeaponMesh->MarkRenderStateDirty();
	}
	MuzimiOwnerCharacter = MuzimiOwnerCharacter == nullptr ? Cast<AMuzimiCharacter>(GetOwner()) : MuzimiOwnerCharacter;
	if (MuzimiOwnerCharacter && bUseServerSideRewind)
	{
		MuzimiOwnerController = MuzimiOwnerController == nullptr ? Cast<AMuzimiPlayerController>(MuzimiOwnerCharacter->GetController()) : MuzimiOwnerController;
		if (MuzimiOwnerController && HasAuthority() && MuzimiOwnerController->HighPingDelegate.IsBound())
		{
			MuzimiOwnerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}
void AWeapon::OnRep_WeaponState()
{
	OnWeaponStateSet();
}
