// Fill out your copyright notice in the Description page of Project Settings.


#include "CaptureTheFlag/FlagZone.h"
#include "Components/SphereComponent.h"
#include "Weapon/Flag.h"
#include "GameMode/CapturaTheFlagGameMode.h"
#include "Character/MuzimiCharacter.h"
AFlagZone::AFlagZone()
{
	PrimaryActorTick.bCanEverTick = true;
	ZoneSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ZoneSphere"));
	SetRootComponent(ZoneSphere);
}

void AFlagZone::BeginPlay()
{
	Super::BeginPlay();
	
	ZoneSphere->OnComponentBeginOverlap.AddDynamic(this, &AFlagZone::OnSphereOverlap);
}

void AFlagZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AFlagZone::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AFlag* OverlappingFlag = Cast<AFlag>(OtherActor);
	if (OverlappingFlag && OverlappingFlag->Team == Team)
	{
		ACapturaTheFlagGameMode* GameMode = GetWorld()->GetAuthGameMode<ACapturaTheFlagGameMode>();
		if (GameMode)
		{
            GameMode->FlagCaptured(OverlappingFlag, this);
		}
		OverlappingFlag->ResetFlag();
	}
}