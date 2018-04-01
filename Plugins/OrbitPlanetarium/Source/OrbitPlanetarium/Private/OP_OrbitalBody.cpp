// Fill out your copyright notice in the Description page of Project Settings.

#include "OP_OrbitalBody.h"
#include "Components/SplineComponent.h"
#include "Components/MeshComponent.h"
#include "OP_OrbitSpline.h"
#include "Engine.h"


// Sets default values
AOP_OrbitalBody::AOP_OrbitalBody()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AOP_OrbitalBody::BeginPlay()
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

	T = TOffset;

	// Create the spline that represents this orbit
	CreateOrbitSpline();
}

void AOP_OrbitalBody::CreateOrbitSpline()
{
	// If there is no parent body, there is no orbit
	if (ParentBody == nullptr) return;

	// Spawn the spline
	FActorSpawnParameters params;
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	params.bNoFail = true;
	FString name = GetName().Append(FString(" Orbit Spline"));
	params.Name = FName(*name);

	AOP_OrbitSpline* spline = GetWorld()->SpawnActor<AOP_OrbitSpline>(AOP_OrbitSpline::StaticClass(), ParentBody->GetActorLocation(), FRotator::ZeroRotator, params);

	if (spline)
	{
		// Create the spline and save a ref
		spline->GenerateSplineForBody(this, MaxSplinePoints, ParentBodyMoving());
		OrbitSpline = spline->FindComponentByClass<USplineComponent>();
	}
}

void AOP_OrbitalBody::RefreshOrbitSpline()
{
	if (OrbitSpline)
	{
		AOP_OrbitSpline* spline = Cast<AOP_OrbitSpline>(OrbitSpline->GetOwner());
		if (spline)
		{
			spline->GenerateSplineForBody(this, MaxSplinePoints, ParentBodyMoving());
		}
	}
	else
	{
		// If no spline so far, create one
		CreateOrbitSpline();
	}
}

void AOP_OrbitalBody::UpdatePosition()
{
	float dist = AngularVelocity * T;
	UE_LOG(LogTemp, Warning, TEXT("Dist = %f"), dist);
	if (OrbitSpline)
	{
		MarkComponentsRenderStateDirty();
		float splineLength = OrbitSpline->GetSplineLength();
		if (splineLength < dist) T = 0.f;
		else if (dist <= 0.0f) dist = splineLength;
		FVector newPos = OrbitSpline->GetLocationAtDistanceAlongSpline(dist, ESplineCoordinateSpace::World);
		SetActorLocation(newPos);
		UE_LOG(LogTemp, Warning, TEXT("Setting new position"));
	}
}

bool AOP_OrbitalBody::ParentBodyMoving()
{
	bool parentMoving = false;
	AOP_OrbitalBody* parent = Cast<AOP_OrbitalBody>(ParentBody);
	if (parent)
	{
		parentMoving = parent->IsMobile;
	}
	return parentMoving;
}

void AOP_OrbitalBody::RotatePlanet(float DeltaTime)
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

float AOP_OrbitalBody::GetSemiMinorAxis(float a, float e)
{
	return a * FMath::Sqrt(1.f - FMath::Square(e));
}

void AOP_OrbitalBody::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = PropertyChangedEvent.GetPropertyName();


	if (PropertyName == GET_MEMBER_NAME_CHECKED(AOP_OrbitalBody, TOffset))
	{
		// Offset starting position of planet
		T = TOffset;
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(AOP_OrbitalBody, PeriapsisHeight))
	{
		PeriapsisHeight = FMath::Clamp(PeriapsisHeight, 0.0f, SemiMajorAxis);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(AOP_OrbitalBody, Eccentricity) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(AOP_OrbitalBody, SemiMajorAxis))
	{
		// Calculate Semi-Minor Axis from eccentricity and Semi-Major Axis
		Eccentricity = FMath::Clamp(Eccentricity, 0.f, 1.f);
		SemiMinorAxis = GetSemiMinorAxis(SemiMajorAxis, Eccentricity);

		// Make sure Periapse is not now above the SMA
		PeriapsisHeight = FMath::Clamp(PeriapsisHeight, 0.0f, SemiMajorAxis);
	}

	// Update the spline if needed
	RefreshOrbitSpline();
	UpdatePosition();
}


// Called every frame
void AOP_OrbitalBody::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	T += DeltaTime;

	// update the position
	UpdatePosition();

	// update the rotation
	RotatePlanet(DeltaTime);
}

