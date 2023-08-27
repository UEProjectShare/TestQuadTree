#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "UObject/ObjectMacros.h"
#include "Battery.generated.h"

UCLASS()
class TESTQUADTREE_API ABattery : public AStaticMeshActor
{
	GENERATED_BODY()
public:	
	ABattery();
	
protected:
	virtual void BeginPlay() override;
	
public:	
	virtual void Tick(float DeltaTime) override;
	void ActiveState(bool InbActive, AActor* InTargetActor);
	
	UPROPERTY(EditAnywhere)
	UMaterial* Normal;
	
	UPROPERTY(EditAnywhere)
	UMaterial* Active;
	
	UPROPERTY()
	AActor* TargetActor;
	
	bool bActive = false;
};

