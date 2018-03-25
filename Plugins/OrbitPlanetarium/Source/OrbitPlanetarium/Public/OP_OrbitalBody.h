// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OP_OrbitalBody.generated.h"

UCLASS()
class ORBITPLANETARIUM_API AOP_OrbitalBody : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOP_OrbitalBody();

	UPROPERTY(EditAnywhere, Category = Planet)
		FRotator PlanetRotationRate;

	UPROPERTY(VisibleAnywhere, Category = Planet)
		class UMeshComponent* PlanetMesh;

	UPROPERTY(EditAnywhere, Category = Orbit)
		AActor* ParentBody;

	UPROPERTY(EditAnywhere, Category = Orbit)
		float AngularVelocity = 0.0f;

	UPROPERTY(EditAnywhere, Category = Orbit)
		float TOffset = 0.0f;

	UPROPERTY(EditAnywhere, Category = Orbit)
		float Inclination = 0.0f;

	UPROPERTY(EditAnywhere, Category = Orbit)
		float PeriapsisLongitude = 0.0f;

	UPROPERTY(EditAnywhere, Category = Orbit)
		float PeriapsisLattitude = 0.0f;

	UPROPERTY(EditAnywhere, Category = Orbit)
		float PeriapsisHeight = 150.0f;

	UPROPERTY(EditAnywhere, Category = Elliptical)
		float SemiMajorAxis = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = Elliptical)
		float SemiMinorAxis = 0.0f;

	UPROPERTY(EditAnywhere, Category = Elliptical)
		float Eccentricity = 0.0f;

	UPROPERTY(EditAnywhere, Category = Spline)
		class USplineComponent* OrbitSpline;

	UPROPERTY(EditAnywhere, Category = Spline)
		int MaxSplinePoints = 20;

	UPROPERTY(EditAnywhere, Category = Orbit)
		bool IsMobile = true;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	float T = 0.0f;

	// Creates an AOrbit_Spline
	void CreateOrbitSpline();
	void RefreshOrbitSpline();
	void UpdatePosition();
	bool ParentBodyMoving();

	// Rotate the planet by adding rotationRate * DeltaTime to it
	void RotatePlanet(float DeltaTime);

	static float GetSemiMinorAxis(float a, float e);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
