// Fill out your copyright notice in the Description page of Project Settings.

#include "OP_OrbitSpline.h"
#include "Components/SplineComponent.h"
#include "OrbitPlanetarium/Public/OP_OrbitalBody.h"


// Sets default values
AOP_OrbitSpline::AOP_OrbitSpline()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Attach the spline component
	Spline = CreateDefaultSubobject <USplineComponent>(TEXT("Spline"));
	RootComponent = Spline;

	if (Spline)
	{
		UE_LOG(LogTemp, Warning, TEXT("Spline Created"));
	}
}

void AOP_OrbitSpline::GenerateSplineForBody(AActor * body, int numPoints, bool parentBodyMoving)
{
	if (Spline == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Spline Component on OrbitSpline"));
		return;
	}
		

	AOP_OrbitalBody* orbitBody = Cast<AOP_OrbitalBody>(body);
	if (orbitBody == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Type passed to AOP_OrbitalBody::GenerateSplineForBody is not of type AOrbit_Body"));
		return;
	}

	if (orbitBody->ParentBody == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("orbitBody in AOP_OrbitalBody::GenerateSplineForBody does not have a valid ParentBody"));
		return;
	}

	SplinePoints = numPoints >= 4 ? numPoints : 4;

	// Clear the spline
	Spline->ClearSplinePoints();

	// Get the points
	TArray<FVector> points = GetEllipsePoints(orbitBody->SemiMajorAxis, orbitBody->SemiMinorAxis, SplinePoints);

	// Add, offset and incline the points
	for (FVector pos : points)
	{
		pos.X += (orbitBody->SemiMajorAxis - orbitBody->PeriapsisHeight);
		pos = pos.RotateAngleAxis(orbitBody->Inclination, FVector::ForwardVector);
		Spline->AddSplinePoint(/*orbitBody->ParentBody->GetActorLocation() +*/ pos, ESplineCoordinateSpace::Local);
	}

	// Set the rotation
	FRotator rot;
	rot.Yaw = orbitBody->PeriapsisLongitude;
	rot.Pitch = orbitBody->PeriapsisLattitude;
	rot.Roll = 0.f;
	(SetActorRotation(rot));
	/*{
	GEngine->AddOnScreenDebugMessage(-1, 5.0, FColor::Red, rot.ToString() );
	}*/

	// Close the loop
	Spline->SetClosedLoop(true);

	// Set the splin centre pos to the parent bodies position
	UpdateOrbitPostion();

	// If the parent body is moving we need to follow it with this actor
	if (parentBodyMoving)
	{
		ParentBody = orbitBody->ParentBody;
		SetActorTickEnabled(true);
		UpdatePosition = true;
	}
	else
	{
		SetActorTickEnabled(false);
	}
}

// Called when the game starts or when spawned
void AOP_OrbitSpline::BeginPlay()
{
	Super::BeginPlay();	
}

void AOP_OrbitSpline::UpdateOrbitPostion()
{
	if (ParentBody)
	{
		SetActorLocation(ParentBody->GetActorLocation());
	}
}

TArray<FVector> AOP_OrbitSpline::GetEllipsePoints(float a, float b, int num)
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

void AOP_OrbitSpline::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = PropertyChangedEvent.GetPropertyName();
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AOP_OrbitSpline, SplinePoints))
	{
		if (SplinePoints < 4) SplinePoints = 4;
	}
}

// Called every frame
void AOP_OrbitSpline::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (UpdatePosition && ParentBody != nullptr)
	{
		UpdateOrbitPostion();
	}
}

