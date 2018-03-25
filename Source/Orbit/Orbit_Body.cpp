// Fill out your copyright notice in the Description page of Project Settings.

#include "Orbit_Body.h"
#include "Components/SplineComponent.h"
#include "Orbit_Spline.h"
#include "Engine/World.h"
#include "Engine.h"
#include "Components/MeshComponent.h"


// Sets default values
AOrbit_Body::AOrbit_Body()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AOrbit_Body::BeginPlay()
{
	Super::BeginPlay();

	// For now if is mobile, skip this and disable ticking
	if (IsMobile == false)
	{
		PrimaryActorTick.bCanEverTick = false;
		return;
	}

	// Ensure the SemiMinorAxis has been calculated
	SemiMinorAxis = GetSemiMinorAxis(SemiMajorAxis, Eccentricity);

	// Create the spline that represents this orbit
	CreateOrbitSpline();
}

FVector AOrbit_Body::GetUpdatedPosition(FVector thisPos, FVector otherPos, float omega, float t, float r, float angle)
{
	float xPos = r * FMath::Cos(omega * t);
	float yPos = r * FMath::Sin(omega * t);

	FVector newPos(xPos, yPos, 0.0f);
	newPos = newPos.RotateAngleAxis(angle, FVector::ForwardVector);

	newPos += otherPos;

	return newPos;
}


void AOrbit_Body::GenerateEllipticalOrbitSpline()
{
	if (OrbitSpline == nullptr)
	{
		OrbitSpline = FindComponentByClass<USplineComponent>();
		if (OrbitSpline == nullptr) return;
	}

	OrbitSpline->ClearSplinePoints();

	// Assuming ellipse on x axis centred at origin
	// Plot the 4 points corresponding to the axes

	TArray<FVector> points = GetEllipsePoints(SemiMajorAxis, SemiMinorAxis, 12);

	for (FVector pos : points)
	{
		pos = pos.RotateAngleAxis(Inclination, FVector::ForwardVector);
		OrbitSpline->AddSplinePoint(ParentBody->GetActorLocation() + pos, ESplineCoordinateSpace::World);
	}

	OrbitSpline->SetRelativeRotation(FRotator(0.0f, PeriapsisLongitude, 0.0f));

	OrbitSpline->SetClosedLoop(true);
}

void AOrbit_Body::CreateOrbitSpline()
{
	// If there is no parent body, there is no orbit
	if (ParentBody == nullptr) return;

	// Spawn the spline
	FActorSpawnParameters params;
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	params.bNoFail = true;
	FString name = GetName().Append(FString(" Orbit Spline"));
	params.Name = FName(*name);
	
	AOrbit_Spline* spline = GetWorld()->SpawnActor<AOrbit_Spline>(AOrbit_Spline::StaticClass(), ParentBody->GetActorLocation(), FRotator::ZeroRotator, params);

	if (spline)
	{
		// Create the spline and save a ref
		spline->GenerateSplineForBody(this, MaxSplinePoints, ParentBodyMoving());
		OrbitSpline = spline->FindComponentByClass<USplineComponent>();
	}
}

void AOrbit_Body::RefreshOrbitSpline()
{
	if (OrbitSpline)
	{
		Cast<AOrbit_Spline>(OrbitSpline->GetOwner())->GenerateSplineForBody(this, MaxSplinePoints, ParentBodyMoving());
	}
}

bool AOrbit_Body::ParentBodyMoving()
{
	bool parentMoving = false;
	AOrbit_Body* parent = Cast<AOrbit_Body>(ParentBody);
	if (parent)
	{
		parentMoving = parent->IsMobile;
	}
	return parentMoving;
}

void AOrbit_Body::RotatePlanet(float DeltaTime)
{
	if (PlanetMesh == nullptr)
	{
		PlanetMesh = FindComponentByClass<UMeshComponent>();
	}

	if (PlanetMesh)
	{
		FRotator rot = PlanetRotationRate * DeltaTime;
		PlanetMesh->SetRelativeRotation(PlanetMesh->RelativeRotation + rot);
	}

}

float AOrbit_Body::SolveEllipseForX(float a, float b, float y)
{
	float ans = FMath::Square(y) / FMath::Square(b);
	ans = 1 - ans;
	ans *= FMath::Square(a);
	return FMath::Sqrt(ans);
}

float AOrbit_Body::SolveEllipseForY(float a, float b, float x)
{
	float ans = FMath::Square(x) / FMath::Square(a);
	ans = 1 - ans;
	ans *= FMath::Square(b);
	return FMath::Sqrt(ans);
}

TArray<FVector> AOrbit_Body::GetEllipsePoints(float a, float b, int num)
{
	if (num < 4)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to generate an ellipse with less that 4 points, defaulting to 4"));
		num = 4;
	}

	TArray<FVector> points;

	// Get the step
	float angleStep = (2.f * PI) / num;
	for (int i = 0; i < num; i++)
	{
		float angle = i * angleStep;

		points.Add(FVector(a * FMath::Cos(angle), b * FMath::Sin(angle), 0.0f));		
	}

	return points;
	
}

float AOrbit_Body::GetSemiMinorAxis(float a, float e)
{
	return a * FMath::Sqrt(1.f - FMath::Square(e));
}


void AOrbit_Body::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = PropertyChangedEvent.GetPropertyName();


	if (PropertyName == GET_MEMBER_NAME_CHECKED(AOrbit_Body, TOffset))
	{
		// Offset starting position of planet
		T = TOffset;
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(AOrbit_Body, PeriapsisHeight))
	{
		PeriapsisHeight = FMath::Clamp(PeriapsisHeight, 0.0f, SemiMajorAxis);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(AOrbit_Body, Eccentricity) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(AOrbit_Body, SemiMajorAxis))
	{
		// Calculate Semi-Minor Axis from eccentricity and Semi-Major Axis
		Eccentricity = FMath::Clamp(Eccentricity, 0.f, 1.f);
		SemiMinorAxis = GetSemiMinorAxis(SemiMajorAxis, Eccentricity);

		// Make sure Periapse is not now above the SMA
		PeriapsisHeight = FMath::Clamp(PeriapsisHeight, 0.0f, SemiMajorAxis);
	}

	// Update the spline if needed
	RefreshOrbitSpline();

}

// Called every frame
void AOrbit_Body::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	T += DeltaTime;

	// update the position
	float dist = AngularVelocity * T;
	if (OrbitSpline)
	{ 
		float splineLength = OrbitSpline->GetSplineLength();
		if (splineLength < dist) T = 0.f;
		else if (dist <= 0.0f) dist = splineLength;
		FVector newPos = OrbitSpline->GetLocationAtDistanceAlongSpline(dist, ESplineCoordinateSpace::World);
		SetActorLocation(newPos);
	}

	// update the rotation
	RotatePlanet(DeltaTime);

}

