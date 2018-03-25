// Fill out your copyright notice in the Description page of Project Settings.

#include "ProceduralIcosahedron.h"
#include "FastNoise.h"
#include "UnrealFastNoisePlugin/Public/UFNNoiseGenerator.h"
#include "UnrealFastNoisePlugin/Public/UFNBlueprintFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Atmosphere/AtmosphericFogComponent.h"
#include "Atmosphere/AtmosphericFog.h"
#include "Materials/MaterialInstance.h"
#include "PackedNormal.h"


// Sets default values
AProceduralIcosahedron::AProceduralIcosahedron()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AProceduralIcosahedron::GetSurfacePosition(FVector currentPosition, FVector & surfacePosition, FRotator & lookRotation)
{
	// Get direction Vector to currentPosition
	surfacePosition = (currentPosition - GetActorLocation()).GetSafeNormal();
	surfacePosition *= (Radius + SurfaceOffset);

	// Get a look direction perpendicular to the surface
	FVector rot = (GetActorLocation() - currentPosition).GetSafeNormal();
	FVector arb = (GetActorLocation() - (currentPosition + FVector::UpVector)).GetSafeNormal();

	rot = FVector::CrossProduct(rot, arb);
	lookRotation = rot.Rotation();	
}

// Called when the game starts or when spawned
void AProceduralIcosahedron::BeginPlay()
{
	Super::BeginPlay();

	// Precomupute LOD Squares
	LOD8DistanceSqrd = FMath::Square(LOD8Distance);
	LOD7DistanceSqrd = FMath::Square(LOD7Distance);
	LOD6DistanceSqrd = FMath::Square(LOD6Distance);
	LOD5DistanceSqrd = FMath::Square(LOD5Distance);
	LOD4DistanceSqrd = FMath::Square(LOD4Distance);
	LOD3DistanceSqrd = FMath::Square(LOD3Distance);
	LOD2DistanceSqrd = FMath::Square(LOD2Distance);
	LOD1DistanceSqrd = FMath::Square(LOD1Distance);

	// Get references
	PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	ProcMeshComponent = FindComponentByClass<UProceduralMeshComponent>();
	if (ProcMeshComponent == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Procedural mesh component found in AProceduralIcosahedron"));
	}
	else
	{
		ProcMeshComponent->bUseAsyncCooking = true;
	}

	// Get the noise generator
	GenerateNoise();

	// Create the mesh
	CreateIcosahedron();
}

void AProceduralIcosahedron::CreateIcosahedron(bool Update)
{
	// If no procedural mesh component no point executing
	if (ProcMeshComponent == nullptr) return;

	UPlanetData* planetData = TryGetCachedLOD(Recursionlevel);

	// If the planetData isn't populated we need to generate it
	if (!planetData->IsPopulated())
	{
		// Create the 12 vertices of the icosahedron
		float t = (1.0f + FMath::Sqrt(5.0f)) / 2.0f;

		AddVertex(FVector(-1, t, 0), planetData);
		AddVertex(FVector(1, t, 0), planetData);
		AddVertex(FVector(-1, -t, 0), planetData);
		AddVertex(FVector(1, -t, 0), planetData);

		AddVertex(FVector(0, -1, t), planetData);
		AddVertex(FVector(0, 1, t), planetData);
		AddVertex(FVector(0, -1, -t), planetData);
		AddVertex(FVector(0, 1, -t), planetData);

		AddVertex(FVector(t, 0, -1), planetData);
		AddVertex(FVector(t, 0, 1), planetData);
		AddVertex(FVector(-t, 0, -1), planetData);
		AddVertex(FVector(-t, 0, 1), planetData);

		// Create the 20 triangles of the icosahedron
		TArray<FTriangleIndices> faces;

		// 5 faces around point 0
		faces.Add(FTriangleIndices(0, 11, 5));
		faces.Add(FTriangleIndices(0, 5, 1));
		faces.Add(FTriangleIndices(0, 1, 7));
		faces.Add(FTriangleIndices(0, 7, 10));
		faces.Add(FTriangleIndices(0, 10, 11));

		// 5 adjacent faces 
		faces.Add(FTriangleIndices(1, 5, 9));
		faces.Add(FTriangleIndices(5, 11, 4));
		faces.Add(FTriangleIndices(11, 10, 2));
		faces.Add(FTriangleIndices(10, 7, 6));
		faces.Add(FTriangleIndices(7, 1, 8));

		// 5 faces around point 3
		faces.Add(FTriangleIndices(3, 9, 4));
		faces.Add(FTriangleIndices(3, 4, 2));
		faces.Add(FTriangleIndices(3, 2, 6));
		faces.Add(FTriangleIndices(3, 6, 8));
		faces.Add(FTriangleIndices(3, 8, 9));

		// 5 adjacent faces 
		faces.Add(FTriangleIndices(4, 9, 5));
		faces.Add(FTriangleIndices(2, 4, 11));
		faces.Add(FTriangleIndices(6, 2, 10));
		faces.Add(FTriangleIndices(8, 6, 7));
		faces.Add(FTriangleIndices(9, 8, 1));

		TArray<float> LODRanges;
		LODRanges.Add(LOD1DistanceSqrd);
		LODRanges.Add(LOD2DistanceSqrd);
		LODRanges.Add(LOD3DistanceSqrd);
		LODRanges.Add(LOD4DistanceSqrd);
		LODRanges.Add(LOD5DistanceSqrd);
		LODRanges.Add(LOD6DistanceSqrd);
		LODRanges.Add(LOD7DistanceSqrd);
		LODRanges.Add(LOD8DistanceSqrd);

		// Refine tris // DEFAULT
		for (int i = 0; i < Recursionlevel; i++)
		{
			TArray<FTriangleIndices> faces2;
			for (FTriangleIndices tri : faces)
			{
				// Replace tri with 4 tris
				int a = GetMiddlePoint(tri.V1, tri.V2, planetData);
				int b = GetMiddlePoint(tri.V2, tri.V3, planetData);
				int c = GetMiddlePoint(tri.V3, tri.V1, planetData);

				faces2.Add(FTriangleIndices(tri.V1, a, c));
				faces2.Add(FTriangleIndices(tri.V2, b, a));
				faces2.Add(FTriangleIndices(tri.V3, c, b));
				faces2.Add(FTriangleIndices(a, b, c));
			}

			faces = faces2;
		}		

		// TEST SAMPLE FROM 3D NOISE
		if (NoiseGenerator)
		{
			FVector position = GetActorLocation();
			//FVector ActorLocation = GetActorLocation();
			
			TArray<FSphericalCoords> PolarVertices3D;
			for (int i = 0; i < planetData->Vertices.Num(); i++)
			{
				// Get the height from the 3D noise using the vertex location
				float height = NoiseGenerator->GetNoise3D(planetData->Vertices[i].X, planetData->Vertices[i].Y, planetData->Vertices[i].Z);
				if (height > (1.0f - (MinWaterLevel * 2.0f))) // Clamp to water level
				{
					height = 1.0f - (MinWaterLevel * 2.0f);
				}

				// Calculate Normals using actor position
				FVector normal = ((position + planetData->Vertices[i]) - position).GetSafeNormal();
				planetData->Normals.Add(-normal);

				// Calculate VertexColour for shader
				float vcValue = (height + 1.0f) / 2.0f;
				FLinearColor vColour = FLinearColor(vcValue, vcValue, vcValue);
				planetData->VertexColours.Add(vColour);

				// Redistribution
				height < 0.0f ? FMath::Pow(height * -1.0f, Redistribution) * -1.0f : FMath::Pow(height, Redistribution);

				// Convert to spherical coordinates to apply the height to the radius
				FSphericalCoords sCoords = FSphericalCoords(planetData->Vertices[i]);
				sCoords.Radius += -Radius + (height * Scale);
				planetData->UV.Add(FVector2D(sCoords.Theta, sCoords.Phi));
				PolarVertices3D.Add(sCoords);
			}

			// Empty the vertex array and repopulate it with the polar vertices
			FVector lastV = FVector::ZeroVector;
			planetData->Vertices.Empty();
			for (FSphericalCoords polarV : PolarVertices3D)
			{
				FVector cartesianVertex = polarV.ToCartesian();
				planetData->Vertices.Add(cartesianVertex);
			
			}
		}

		// Create the Triangles
		for (FTriangleIndices tri : faces)
		{
			planetData->Triangles.Add(tri.V1);
			planetData->Triangles.Add(tri.V2);
			planetData->Triangles.Add(tri.V3);

			/*FVector tangent = planetData->Vertices[tri.V1] - planetData->Vertices[tri.V2];
			tangent = tangent.GetSafeNormal();
			planetData->Tangents.Add(FProcMeshTangent(tangent, true));*/
		}

		// Calculate tangents
		TArray<FVector> tangents;
		tangents.Init(FVector::ZeroVector, planetData->Vertices.Num());
		for (int i = 0; i < planetData->Triangles.Num();)
		{
			if (tangents[planetData->Triangles[i]] == FVector::ZeroVector)
			{
				FVector p0 = planetData->Vertices[planetData->Triangles[i]];
				FVector p1 = planetData->Vertices[planetData->Triangles[i + 1]];
				FVector tangent = (p1 - p0).GetSafeNormal();
				tangents[planetData->Triangles[i]] = tangent;				
				planetData->Tangents.Add( FProcMeshTangent(FPackedNormal(tangent), true));
			}
			i += 3;
		}

		/*int32 lastI = planetData->Triangles[planetData->Triangles.Num() - 1];
		for (int32 i : planetData->Triangles)
		{
			FVector tangent = planetData->Vertices[i] - planetData->Vertices[lastI];
			tangent = tangent.GetSafeNormal();

			planetData->Tangents.Add(FProcMeshTangent(tangent, true));
			planetData->Tangents.Add(FProcMeshTangent(tangent, true));
			planetData->Tangents.Add(FProcMeshTangent(tangent, true));

			lastI = i;
		}*/
	}

 // Use the planetData to create the mesh

	// Create the mesh

	UE_LOG(LogTemp, Warning, TEXT("MeshData: %s"), *planetData->ToString());

	ProcMeshComponent->ClearAllMeshSections();
	ProcMeshComponent->CreateMeshSection_LinearColor(
		0,
		planetData->Vertices,
		planetData->Triangles,
		planetData->Normals,
		planetData->UV,
		planetData->VertexColours,
		planetData->Tangents,
		false);

	/*ProcMeshComponent->UpdateMeshSection_LinearColor(
		0,
		planetData->Vertices,
		planetData->Normals,
		planetData->UV,
		planetData->VertexColours,
		planetData->Tangents);*/


	// Set the material
	if (Material)
	{
		ProcMeshComponent->SetMaterial(0, Material);
	}

	// Try to cache the LOD
	CacheLOD(Recursionlevel, planetData);
}

void AProceduralIcosahedron::CreatePlanet()
{
}

int AProceduralIcosahedron::AddVertex(FVector v, UPlanetData* planet)
{
	double Length = v.Size();
	planet->Vertices.Add(FVector(v.X / Length, v.Y / Length, v.Z / Length));
	return planet->Index++;
}

int AProceduralIcosahedron::GetMiddlePoint(int p1, int p2, UPlanetData* planet)
{
	//Calculate it
	FVector point1 = planet->Vertices[p1];
	FVector point2 = planet->Vertices[p2];
	FVector middle = FVector(
		(point1.X + point2.X) / 2.0f,
		(point1.Y + point2.Y) / 2.0f,
		(point1.Z + point2.Z) / 2.0f);

	// Add vertex makes sure our point is on the unit sphere
	int i = AddVertex(middle, planet);

	return i;
}

void AProceduralIcosahedron::GenerateNoise()
{
	if (!bUseSeed) Seed = FMath::RandRange(0, 9999999);

	NoiseGenerator = UUFNBlueprintFunctionLibrary::CreateFractalNoiseGenerator(
		this,
		NoiseType,
		Seed,
		Frequency,
		FractalGain,
		Interpolation,
		FractalType,
		Octaves,
		Lacunarity
	);
}

UPlanetData* AProceduralIcosahedron::TryGetCachedLOD(uint8 LOD)
{
	UPlanetData** cachedDataPtr = CachedLODLevels.Find(LOD);
	UPlanetData* cachedData = nullptr;
	if (cachedDataPtr == nullptr)
	{
		cachedData = NewObject<UPlanetData>();
	}
	else
	{
		cachedData = *cachedDataPtr;
	}

	return cachedData;
}

void AProceduralIcosahedron::CacheLOD(uint8 LOD, UPlanetData* data)
{
	CachedLODLevels.Add(LOD, data);
}

FVector2D AProceduralIcosahedron::MercatorUV(float longitude, float latitude, float radius)
{
	FVector2D UV;
	UV.X = Radius * longitude;
	UV.Y = FMath::Loge(FMath::Tan((latitude + (PI / 2.0f)) / 2.0f));
	return UV;
}

FVector2D AProceduralIcosahedron::SphereUV(FVector d)
{
	d = d.GetSafeNormal();
	FVector2D UV;
	UV.X = 0.5f + (FMath::Atan2(d.X, d.Z) / (2.0f * PI));
	UV.Y = 0.5f + (d.Y * 0.5f);
	return UV;
}

void AProceduralIcosahedron::UpdateAtmosphericFogAmount(float dist)
{
	if (AtmosphereComponent == nullptr) return;

	float perc = (dist - AtmosphereMinHeight) / (AtmosphereMaxHeight - AtmosphereMinHeight);
	perc = 1.0f - perc;
	perc = FMath::Clamp(perc, 0.0f, 1.0f);

	if (perc > 0.0f) UE_LOG(LogTemp, Warning, TEXT("Fog %f"), perc);
	

	AtmosphereComponent->GetAtmosphericFogComponent()->SetFogMultiplier(perc * 6.0f);

}

// Called every frame
void AProceduralIcosahedron::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PlayerPawn && !OverrideLOD)
	{
		float dist = FVector::Distance(PlayerPawn->GetActorLocation(), GetActorLocation());
		uint8 newRecursionLevel = 0;

		UpdateAtmosphericFogAmount(dist);

		if (dist < LOD8Distance)
		{
			newRecursionLevel = 8;
		}
		else if (dist < LOD7Distance)
		{
			newRecursionLevel = 7;
		}
		else if (dist < LOD6Distance)
		{
			newRecursionLevel = 6;
		}
		else if (dist < LOD5Distance)
		{
			newRecursionLevel = 5;
		}
		else if (dist < LOD4Distance)
		{
			newRecursionLevel = 4;
		}
		else
		{
			newRecursionLevel = 3;
		}

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(0, 0.1f, FColor::Green, FString::SanitizeFloat(dist));
		}
		if (newRecursionLevel != Recursionlevel)
		{
			UE_LOG(LogTemp, Warning, TEXT("Recursion Level = %d"), newRecursionLevel);
			Recursionlevel = newRecursionLevel;
			CreateIcosahedron(true);
		}
	}

}

FString UPlanetData::ToString()
{
	FString data = "";
	data += "V: " + FString::FromInt(Vertices.Num());
	data += "T: " + FString::FromInt(Triangles.Num()) + "\n";
	data += "N: " + FString::FromInt(Normals.Num()) + "\n";
	data += "Ta: " + FString::FromInt(Tangents.Num());
	return data;
}
