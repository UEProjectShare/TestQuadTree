#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "GameFramework/Actor.h"
#include "QuadTree.generated.h"

class QuadTreeNode;
class ABattery;

UCLASS()
class TESTQUADTREE_API AQuadTree : public AActor  //该类可以当成树
{
	GENERATED_BODY()
	
public:	
	AQuadTree();
	
protected:
	virtual void BeginPlay() override;
	
public:	
	virtual void Tick(float DeltaTime) override;
	void SpawnActors();
	void ActorsAddVelocity();

	UPROPERTY(EditAnywhere)
	int32 CubeCount = 20;
	
	UPROPERTY(EditAnywhere)
	int32 Width = 500;
	
	UPROPERTY(EditAnywhere)
	int32 Height = 500;
	
	UPROPERTY(EditAnywhere)
	float PlayRate = 0.05;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<ABattery> BatteryClass;
	
	UPROPERTY(EditAnywhere)
	AActor* TraceActor;
	
	UPROPERTY(EditAnywhere)
	float AffectRadianRange = 50;

	UPROPERTY(EditAnywhere)
	bool bCanMove = false;
	
	UPROPERTY()
	TArray<ABattery*> Objs;

	TSharedPtr<QuadTreeNode> Root;
	FTimerHandle Timer;
	FTimerHandle Timer2;
};