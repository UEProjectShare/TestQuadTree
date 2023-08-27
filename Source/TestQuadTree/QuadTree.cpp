#include "QuadTree.h"
#include "Battery.h"
#include "QuadTreeNode.h"
#include "TimerManager.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

AQuadTree::AQuadTree()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

UObject* QuadTreeNode::WorldObject = nullptr;

void AQuadTree::BeginPlay()
{
	Super::BeginPlay();
	QuadTreeNode::WorldObject = GetWorld();
	Root = MakeShareable(new QuadTreeNode(FVector::ZeroVector, FVector(Height, Width, 0), 0));
	GetWorld()->GetTimerManager().SetTimer(Timer, this, &AQuadTree::SpawnActors, PlayRate, true);
	if(bCanMove)
	{
		GetWorld()->GetTimerManager().SetTimer(Timer2, this, &AQuadTree::ActorsAddVelocity, 2, true);
	}
}

void AQuadTree::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (Root.IsValid())
	{
		Root->UpdateState(); //更新状态
		Root->TraceObjectInRange(TraceActor, AffectRadianRange); //判断是否在扫描器的范围内	
	}
}

// 定时生成物体
void AQuadTree::SpawnActors()
{
	if (CubeCount <= 0)
	{
		GetWorld()->GetTimerManager().ClearTimer(Timer);
		return;
	}
	
	CubeCount--;

	const FVector Pos = FVector(UKismetMathLibrary::RandomIntegerInRange(-Height + 10, Height - 10),
	                            UKismetMathLibrary::RandomIntegerInRange(-Width + 10, Width - 10), 11);

	const FTransform Trans = FTransform(FRotator(0, UKismetMathLibrary::RandomFloatInRange(0, 360), 0), Pos, FVector(0.2));
	ABattery* Actor = GetWorld()->SpawnActor<ABattery>(BatteryClass, Trans);
	if (IsValid(Actor))
	{
		Objs.Add(Actor);
		Root->InsertObj(Actor);	
	}
}

// 定时给物体一个速度
void AQuadTree::ActorsAddVelocity()
{
	for (const ABattery* Actor : Objs)
	{
		Actor->GetStaticMeshComponent()->SetPhysicsLinearVelocity(UKismetMathLibrary::RandomUnitVector() * 50);
	}
}
