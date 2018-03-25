// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Messaging.h"
#include "Planet.generated.h"

const float G = 6.67300E-11;

UCLASS()
class ORBIT_API APlanet : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APlanet();

	UPROPERTY(EditAnywhere, Category = Body)
	float Mass;

	UPROPERTY(EditAnywhere, Category = Body)
	FVector Velocity;

	UPROPERTY(VisibleAnywhere, Category = Body)
	FVector Acceleration;

	UPROPERTY(EditAnywhere, Category = Body)
	float Density = 1.0f;

	UPROPERTY(EditAnywhere, Category = Body)
	class UStaticMeshComponent* SphereMesh;

	UPROPERTY(EditAnywhere, Category = Time)
		float TimeScale = 1.0f;

	UPROPERTY(EditAnywhere, Category = Spline)
		class USplineComponent* SplineMesh;

	UPROPERTY(VisibleAnywhere, Category = Body)
	float Radius;


protected:

	UPROPERTY(VisibleAnywhere, Category = Body)
	TArray<APlanet* > InfluencingBodies;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void UpdatePosition(float DeltaTime);
	FVector CalculateNetForce();
	FVector CalculateAccelerationFromForces(FVector Force);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void UpdateInfluencingBodies(TArray<class APlanet*> planets);

private:

	FMessageEndpointPtr SenderEndpoint_RegisterPlanet;

	// Handle for delay registering body
	FTimerHandle TimerHandle_RegisterDelay;
	float RegisterDelayTime = 0.1f;

	// Send the message registering the planet
	void SendRegisterPlanetMessage();
};
