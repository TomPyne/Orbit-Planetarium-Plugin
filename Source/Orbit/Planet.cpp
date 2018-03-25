// Fill out your copyright notice in the Description page of Project Settings.

#include "Planet.h"
#include "OrbitMessageStructs.h"
#include "Components/StaticMeshComponent.h"
#include "math.h"
#include "OrbitSettingsInstance.h"
#include "Components/SplineComponent.h"

struct FDoubleVector
{
	double X;
	double Y;
	double Z;

	FDoubleVector(double x, double y, double z);
};


// Sets default values
APlanet::APlanet()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SphereMesh = FindComponentByClass<UStaticMeshComponent>();

}

// Called when the game starts or when spawned
void APlanet::BeginPlay()
{
	Super::BeginPlay();
	
	SplineMesh = FindComponentByClass<USplineComponent>();

	// Set up the messaging
	SenderEndpoint_RegisterPlanet = FMessageEndpoint::Builder("RegisterPlanet");

	// Start the register delay
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_RegisterDelay, this, &APlanet::SendRegisterPlanetMessage, RegisterDelayTime, false);
}

void APlanet::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SenderEndpoint_RegisterPlanet.Reset();

	Super::EndPlay(EndPlayReason);
}

void APlanet::UpdatePosition(float DeltaTime)
{
	// Accelerate
	float t = (DeltaTime * TimeScale);

	// calculate distance moved
	FVector dist = (Velocity * t);
	dist += ((0.5f * Acceleration) * FMath::Square(t));


	// Move
	FVector position = GetActorLocation();
	SetActorLocation(position + dist);
}

FVector APlanet::CalculateNetForce()
{
	FVector totalForce = FVector::ZeroVector;
	for (APlanet* otherPlanet : InfluencingBodies)
	{
		double r = FVector::Distance(GetActorLocation(), otherPlanet->GetActorLocation());
		//r -= otherPlanet->Radius;
		double r_Squared = r * r;

		double force = (G * Mass * otherPlanet->Mass) / r_Squared;
		// Turn that force into a vector
		FVector direction = otherPlanet->GetActorLocation() - GetActorLocation();
		direction.Normalize();

		// Add it to the sum of force
		totalForce += direction * force;
	}

	return totalForce;
}

FVector APlanet::CalculateAccelerationFromForces(FVector Force)
{
	// F=Ma thus a = F/M
	return Force / Mass;
}

void APlanet::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = PropertyChangedEvent.GetPropertyName();


	if ((PropertyName == GET_MEMBER_NAME_CHECKED(APlanet, Mass)) || (PropertyName == GET_MEMBER_NAME_CHECKED(APlanet, Density)))
	{
		if (SphereMesh)
		{
			Mass = Mass >= 0.0f ? Mass : 0.0f;
			Density = Density >= 0.0f ? Density : 0.0f;

			// Calculate the radius
			float volume = Mass / Density;
			float radius = (3.0f * (volume / (4 * PI)));
			radius = pow(radius, (1.0f / 3.0f));
			FVector scale = FVector::OneVector * (radius / 2.0f);
			SetActorScale3D(scale);

			Radius = radius;
		}
		else
		{
			SphereMesh = FindComponentByClass<UStaticMeshComponent>();
		}
	}
}

// Called every frame
void APlanet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Fnet = CalculateNetForce();
	Acceleration = CalculateAccelerationFromForces(Fnet);

	UpdatePosition(DeltaTime);
}

void APlanet::UpdateInfluencingBodies(TArray<class APlanet*> planets)
{
	InfluencingBodies = planets;
	
	InfluencingBodies.Remove(this);
}

void APlanet::SendRegisterPlanetMessage()
{
	if (SenderEndpoint_RegisterPlanet.IsValid())
	{
		SenderEndpoint_RegisterPlanet->Publish<FRegisterPlanetMessage>(new FRegisterPlanetMessage(this));
	}
}

