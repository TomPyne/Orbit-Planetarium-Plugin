// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Orbit_Spline.generated.h"

UCLASS()
class ORBIT_API AOrbit_Spline : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOrbit_Spline();

	UPROPERTY(EditAnywhere, Category = Spline)
	class USplineComponent* Spline;

	UPROPERTY(EditAnywhere, Category = Spline)
	int SplinePoints = 4;

	void GenerateSplineForBody(AActor* body, int numPoints = 4, bool parentBodyMoving = false);

	

protected:

	// The body this orbit surrounds, used for updating position
	AActor * ParentBody = nullptr;

	bool UpdatePosition = false;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Update the position of the orbit
	void UpdateOrbitPostion();

	static TArray<FVector> GetEllipsePoints(float a, float b, int num = 4);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
