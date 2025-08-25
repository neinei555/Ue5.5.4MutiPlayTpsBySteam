// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MuzimiAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapon/Weapon.h"
#include "Character/MuzimiCharacter.h"
#include "MuzimiTypes/CombatStates.h" 


void UMuzimiAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	MuzimiCharacter = Cast<AMuzimiCharacter>(TryGetPawnOwner());
}

void UMuzimiAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (MuzimiCharacter == nullptr)
	{
		MuzimiCharacter = Cast<AMuzimiCharacter>(TryGetPawnOwner()); 
	}
	if (MuzimiCharacter == nullptr)
	{
		return;
	}

	FVector Velocity = MuzimiCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();
	bIsInAir = MuzimiCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = MuzimiCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	bWeaponEquipped = MuzimiCharacter->IsWeaponEquipped();
	EquippedWeapon = MuzimiCharacter->GetEquippedWeapon();
	bIsCrouched = MuzimiCharacter->bIsCrouched; 
	bIsAimed = MuzimiCharacter->IsAimed();
	TurningInPlace = MuzimiCharacter->GetTurningInPlace();
	bRotateRootBone = MuzimiCharacter->ShouldRotateRootBone();
	bElimmed = MuzimiCharacter->IsElimmed();
	 //Offset Yaw for Strafing
	FRotator AimRotation = MuzimiCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(MuzimiCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaSeconds, 6.f);
	YawOffset = DeltaRotation.Yaw;
	bHoldingTheFlag = MuzimiCharacter->IsHoldingTheFlag();

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = MuzimiCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw;
	const float interp = FMath::FInterpTo(Lean, Target, DeltaSeconds, 6.f);
	Lean = FMath::Clamp(interp, -90.f, 90.f);
	AO_Yaw = MuzimiCharacter->GetAO_Yaw();
	AO_Pitch = MuzimiCharacter->GetAO_Pitch();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && MuzimiCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		MuzimiCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition,OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));
	}   
	if (MuzimiCharacter->IsLocallyControlled() && EquippedWeapon)
	{
		bLocallyControlled = true;
		RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("Hand_R"), ERelativeTransformSpace::RTS_World);
		FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - MuzimiCharacter->GetHitTarget()));
		RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaSeconds, 40.f);

	}
	bUseFABRIK = MuzimiCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;
	bool bFABRIKOverride =
		MuzimiCharacter->IsLocallyControlled() &&
		MuzimiCharacter->GetCombatState() != ECombatState::ECS_ThrowGrenade &&
		MuzimiCharacter->bFinishedSwapping;
	if (bFABRIKOverride)
	{
		bUseFABRIK = !MuzimiCharacter->IsLocallyReloading();
	}
	bUseAimOffsets = MuzimiCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !MuzimiCharacter->GetDisableGameplay() ;
	bTransformRightHand = MuzimiCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !MuzimiCharacter->GetDisableGameplay();
}

