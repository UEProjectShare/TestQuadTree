#include "Battery.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"

ABattery::ABattery()
{
	PrimaryActorTick.bCanEverTick = true;
	GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
	GetStaticMeshComponent()->SetConstraintMode(EDOFMode::XYPlane);
	GetStaticMeshComponent()->SetSimulatePhysics(true);	
}

void ABattery::BeginPlay()
{
	Super::BeginPlay();	
}

void ABattery::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bActive && TargetActor)
	{
		const float DrawTime = 1 / UKismetSystemLibrary::GetFrameCount();
		DrawDebugLine(GetWorld(), GetActorLocation(), TargetActor->GetActorLocation(), FColor(0,148,220,255), false, DrawTime, 1, 4.0f);
	}
}

void ABattery::ActiveState(bool InbActive, AActor* InTargetActor)
{
	if (bActive == InbActive)
	{
		return;
	}
	
	bActive = InbActive;
	TargetActor = InTargetActor;
	GetStaticMeshComponent()->SetMaterial(0, bActive ? Active : Normal);
}
