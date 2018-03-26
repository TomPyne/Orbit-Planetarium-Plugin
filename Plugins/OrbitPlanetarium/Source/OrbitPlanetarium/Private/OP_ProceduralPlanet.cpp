// Fill out your copyright notice in the Description page of Project Settings.

#include "OP_ProceduralPlanet.h"
#include "PackedNormal.h"
#include "FastNoise.h"
#include "UnrealFastNoisePlugin/Public/UFNBlueprintFunctionLibrary.h"
#include "UnrealFastNoisePlugin/Public/UFNNoiseGenerator.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "ImageUtils.h"
#include "Engine/Texture2D.h"
#include "OP_NoiseCube.h"

FString UOP_PlanetData::ToString()
{
	FString data = "";
	data += "V: " + FString::FromInt(Vertices.Num());
	data += "T: " + FString::FromInt(Triangles.Num()) + "\n";
	data += "N: " + FString::FromInt(Normals.Num()) + "\n";
	data += "Ta: " + FString::FromInt(Tangents.Num());
	return data;
}

// Sets default values
AOP_ProceduralPlanet::AOP_ProceduralPlanet()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ProcMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Mesh"));
	RootComponent = ProcMeshComponent;
}

// Called when the game starts or when spawned
void AOP_ProceduralPlanet::BeginPlay()
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

	// Generate the noise cubes
	GenerateNoiseCubes();
	cubemap = NoiseCube->GetCubeTextures();

	GenerateCubemapNoise();

	// Get references
	PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	// Check the LOD, this also generates the planet if the LOD has changed
	CheckLODRange(true);

	MakeTestTex();
}

void AOP_ProceduralPlanet::GenerateNoiseCubes()
{
	NoiseCube = NewObject<UOP_NoiseCube>(this);
	NoiseCube->Init(256, NoiseType, Seed, Frequency, FractalGain, Interpolation, FractalType, Octaves, Lacunarity);

	RoughNoiseCube = NewObject<UOP_NoiseCube>(this);
	RoughNoiseCube->Init(256, RoughNoiseType, Seed, RoughFrequency, RoughFractalGain, RoughInterpolation, RoughFractalType, RoughOctaves, RoughLacunarity);
}

// Called every frame
void AOP_ProceduralPlanet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CheckLODRange(false);
}

void AOP_ProceduralPlanet::GeneratePlanet(bool bIgnoreLOD)
{
	// If no procedural mesh component no point executing
	if (ProcMeshComponent == nullptr) { return; }

	UOP_PlanetData* planetData = TryGetCachedLOD(currentLOD);
	if (bIgnoreLOD)
	{
		planetData = NewObject<UOP_PlanetData>();
	}
	if (planetData == nullptr) { return; }

	// Generate the cube map noise
	GenerateCubemapNoise();

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
		TArray<FOP_TriangleIndices> faces;

		// 5 faces around point 0
		faces.Add(FOP_TriangleIndices(0, 11, 5));
		faces.Add(FOP_TriangleIndices(0, 5, 1));
		faces.Add(FOP_TriangleIndices(0, 1, 7));
		faces.Add(FOP_TriangleIndices(0, 7, 10));
		faces.Add(FOP_TriangleIndices(0, 10, 11));

		// 5 adjacent faces 
		faces.Add(FOP_TriangleIndices(1, 5, 9));
		faces.Add(FOP_TriangleIndices(5, 11, 4));
		faces.Add(FOP_TriangleIndices(11, 10, 2));
		faces.Add(FOP_TriangleIndices(10, 7, 6));
		faces.Add(FOP_TriangleIndices(7, 1, 8));

		// 5 faces around point 3
		faces.Add(FOP_TriangleIndices(3, 9, 4));
		faces.Add(FOP_TriangleIndices(3, 4, 2));
		faces.Add(FOP_TriangleIndices(3, 2, 6));
		faces.Add(FOP_TriangleIndices(3, 6, 8));
		faces.Add(FOP_TriangleIndices(3, 8, 9));

		// 5 adjacent faces 
		faces.Add(FOP_TriangleIndices(4, 9, 5));
		faces.Add(FOP_TriangleIndices(2, 4, 11));
		faces.Add(FOP_TriangleIndices(6, 2, 10));
		faces.Add(FOP_TriangleIndices(8, 6, 7));
		faces.Add(FOP_TriangleIndices(9, 8, 1));

		// Refine tris
		for (int i = 0; i < currentLOD; i++)
		{
			TArray<FOP_TriangleIndices> faces2;
			for (FOP_TriangleIndices tri : faces)
			{
				// Replace tri with 4 tris
				int a = GetMiddlePoint(tri.V1, tri.V2, planetData);
				int b = GetMiddlePoint(tri.V2, tri.V3, planetData);
				int c = GetMiddlePoint(tri.V3, tri.V1, planetData);

				faces2.Add(FOP_TriangleIndices(tri.V1, a, c));
				faces2.Add(FOP_TriangleIndices(tri.V2, b, a));
				faces2.Add(FOP_TriangleIndices(tri.V3, c, b));
				faces2.Add(FOP_TriangleIndices(a, b, c));
			}

			faces = faces2;
		}


		FVector position = GetActorLocation();
		//FVector ActorLocation = GetActorLocation();

		TArray<FOP_SphericalCoords> PolarVertices3D;
		for (int i = 0; i < planetData->Vertices.Num(); i++)
		{
			// Get height from the cubemap
			FVector vNormal = (GetActorLocation() - planetData->Vertices[i]).GetSafeNormal();
			//float height = GetCubemapHeight(planetData->Vertices[i], vNormal);

			float height = 0.0f;
			// Get height from noiseCube
			if (NoiseCube)
			{
				height = NoiseCube->SampleNoiseCube(vNormal);
			}

			if (RoughNoiseCube)
			{
				//height *= 1.0f - RoughnessInfluence;
				height += RoughnessInfluence * RoughNoiseCube->SampleNoiseCube(vNormal);
			}

			height *= Boost;


			// Get the height from the 3D noise using the vertex location
			/*float height = noiseGenerator->GetNoise3D(planetData->Vertices[i].X, planetData->Vertices[i].Y, planetData->Vertices[i].Z);
			height += roughNoiseGenerator->GetNoise3D(planetData->Vertices[i].X, planetData->Vertices[i].Y, planetData->Vertices[i].Z) / 2.0f;*/
			if (height >(1.0f - (MinWaterLevel * 2.0f))) // Clamp to water level
			{
				height = 1.0f - (MinWaterLevel * 2.0f);
			}

			// Calculate Normals using actor position
			FVector normal = ((position + planetData->Vertices[i]) - position).GetSafeNormal();
			planetData->Normals.Add(-normal);

			// Calculate VertexColour for shader
			float vcValue = (height + 1.0f) / 2.0f;

			// If the point is lower than MinWaterLevel ensure it is displayed as water in the material by
			// forcing value to 0
			if (vcValue >= 1 - MinWaterLevel) vcValue = 0.95f;

			vcValue = FMath::Clamp(vcValue, 0.0f, 1.0f);
			FLinearColor vColour = FLinearColor(vcValue, vcValue, vcValue);
			planetData->VertexColours.Add(vColour);

			// Redistribution
			height < 0.0f ? FMath::Pow(height * -1.0f, Redistribution) * -1.0f : FMath::Pow(height, Redistribution);

			// Convert to spherical coordinates to apply the height to the radius
			FOP_SphericalCoords sCoords = FOP_SphericalCoords(planetData->Vertices[i]);
			//sCoords.Radius = Radius + (height * Scale);
			sCoords.Radius += -Radius + (height * Scale);

			planetData->UV.Add(FVector2D(sCoords.Theta, sCoords.Phi));
			PolarVertices3D.Add(sCoords);
		}

		// Empty the vertex array and repopulate it with the polar vertices
		FVector lastV = FVector::ZeroVector;
		planetData->Vertices.Empty();
		for (FOP_SphericalCoords polarV : PolarVertices3D)
		{
			FVector cartesianVertex = polarV.ToCartesian();
			planetData->Vertices.Add(cartesianVertex);

		}

		// Create the Triangles
		for (FOP_TriangleIndices tri : faces)
		{
			planetData->Triangles.Add(tri.V1);
			planetData->Triangles.Add(tri.V2);
			planetData->Triangles.Add(tri.V3);
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
			planetData->Tangents.Add(FProcMeshTangent(tangent, true));
			}
			i += 3;
		}
		
	}



	UE_LOG(LogTemp, Warning, TEXT("MeshData: %s"), *planetData->ToString());

	// Create the mesh
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

	// Set the material
	if (Material)
	{
		ProcMeshComponent->SetMaterial(0, Material);
	}

	// Try to cache the LOD
	if (!bIgnoreLOD)
	{
		CacheLOD(currentLOD, planetData);
	}

	// Generate image from heightmap
	GenerateHeatMapTex(planetData);


}

void AOP_ProceduralPlanet::ClearPlanet()
{
	if (ProcMeshComponent != nullptr)
	{
		ProcMeshComponent->ClearAllMeshSections();
	}
}

void AOP_ProceduralPlanet::UpdatePlanet()
{
	// If no procedural mesh component no point executing
	if (ProcMeshComponent == nullptr) { return; }
	UOP_PlanetData* planetData = TryGetCachedLOD(currentLOD);

	if (planetData == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlanetData is not available, try generate planet first instead"));
		return;
	}
	else if (!planetData->IsPopulated())
	{
		UE_LOG(LogTemp, Warning, TEXT("PlanetData is not available, try generate planet first instead"));
		return;
	}

	// Generate the cube map noise
	GenerateCubemapNoise();

	TArray<FOP_SphericalCoords> PolarVertices3D;

	// Get the new heights for each vertex
	for (int i = 0; i < planetData->Vertices.Num(); i++)
	{
		// Get height from the cubemap
		FVector vNormal = (GetActorLocation() - planetData->Vertices[i]).GetSafeNormal();
		float height = GetCubemapHeight(planetData->Vertices[i], vNormal);

		// Clamp to water level
		if (height > (1.0f - (MinWaterLevel * 2.0f)))
		{
			height = 1.0f - (MinWaterLevel * 2.0f);
		}

		// Add normal data
		planetData->Normals.Add(-vNormal);

		// Calculate VertexColour for shader
		float vcValue = (height + 1.0f) / 2.0f;
		vcValue = FMath::Clamp(vcValue, 0.0f, 1.0f);
		FLinearColor vColour = FLinearColor(vcValue, vcValue, vcValue);
		planetData->VertexColours.Add(vColour);

		// Redistribution
		height < 0.0f ? FMath::Pow(height * -1.0f, Redistribution) * -1.0f : FMath::Pow(height, Redistribution);

		// Convert to spherical coordinates to apply the height to the radius
		FOP_SphericalCoords sCoords = FOP_SphericalCoords(planetData->Vertices[i]);
		sCoords.Radius = Radius + (height * Scale);
		planetData->UV.Add(FVector2D(sCoords.Theta, sCoords.Phi));
		PolarVertices3D.Add(sCoords);
	}

	// Empty the vertex array and repopulate it with the polar vertices
	planetData->Vertices.Empty();
	for (FOP_SphericalCoords polarV : PolarVertices3D)
	{
		FVector cartesianVertex = polarV.ToCartesian();
		planetData->Vertices.Add(cartesianVertex);

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
			planetData->Tangents.Add(FProcMeshTangent(tangent, true));
		}
		i += 3;
	}

	UE_LOG(LogTemp, Warning, TEXT("MeshData: %s"), *planetData->ToString());

	// Update the mesh
	//ProcMeshComponent->ClearAllMeshSections();
	ProcMeshComponent->UpdateMeshSection_LinearColor(
		0,
		planetData->Vertices,
		planetData->Normals,
		planetData->UV,
		planetData->VertexColours,
		planetData->Tangents);

	// Set the material
	if (Material)
	{
		ProcMeshComponent->SetMaterial(0, Material);
	}

	// Cache the changes
	CacheLOD(currentLOD, planetData);

	// Generate image from heightmap
	GenerateHeatMapTex(planetData);
}

int AOP_ProceduralPlanet::AddVertex(FVector v, UOP_PlanetData * planet)
{
	double Length = v.Size();
	planet->Vertices.Add(FVector(v.X / Length, v.Y / Length, v.Z / Length));
	return planet->Index++;
}

int AOP_ProceduralPlanet::GetMiddlePoint(int p1, int p2, UOP_PlanetData * planet)
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

UUFNNoiseGenerator * AOP_ProceduralPlanet::CreateNoiseGenerator()
{
	if (!bUseSeed) Seed = FMath::RandRange(0, 9999999);

	UUFNNoiseGenerator* noiseGen = UUFNBlueprintFunctionLibrary::CreateFractalNoiseGenerator(
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

	return noiseGen;
}

UUFNNoiseGenerator * AOP_ProceduralPlanet::CreateRoughNoiseGenerator()
{
	if (!bUseSeed) Seed = FMath::RandRange(0, 9999999);

	UUFNNoiseGenerator* noiseGen = UUFNBlueprintFunctionLibrary::CreateFractalNoiseGenerator(
		this,
		RoughNoiseType,
		Seed,
		RoughFrequency,
		RoughFractalGain,
		RoughInterpolation,
		RoughFractalType,
		RoughOctaves,
		RoughLacunarity
	);

	return noiseGen;
}

UUFNNoiseGenerator * AOP_ProceduralPlanet::GetNoiseGenerator()
{
	if (NoiseGenerator != nullptr)
	{
		return NoiseGenerator;
	}
	else
	{
		NoiseGenerator = CreateNoiseGenerator();

		if (NoiseGenerator != nullptr)
		{
			return NoiseGenerator;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Unable to create a noise Generator"));
			return nullptr;
		}
	}


}

UOP_PlanetData * AOP_ProceduralPlanet::TryGetCachedLOD(uint8 LOD)
{
	UOP_PlanetData** cachedDataPtr = CachedLODLevels.Find(LOD);
	UOP_PlanetData* cachedData = nullptr;
	if (cachedDataPtr == nullptr)
	{
		cachedData = NewObject<UOP_PlanetData>();
	}
	else
	{
		cachedData = *cachedDataPtr;
	}

	return cachedData;
}

void AOP_ProceduralPlanet::CacheLOD(uint8 LOD, UOP_PlanetData * data)
{
	CachedLODLevels.Add(LOD, data);
}

void AOP_ProceduralPlanet::CheckLODRange(bool bForceGeneration)
{
	if (bForceGeneration)
	{
		GeneratePlanet(false);
		return;
	}

	if (PlayerPawn && !bOverrideLOD)
	{
		float dist = FVector::Distance(PlayerPawn->GetActorLocation(), GetActorLocation());
		uint8 newRecursionLevel = 0;

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
		else if (dist < LOD3Distance)
		{
			newRecursionLevel = 3;
		}
		else
		{
			newRecursionLevel = 2;
		}

		if (newRecursionLevel != currentLOD)
		{
			UE_LOG(LogTemp, Warning, TEXT("Recursion Level = %d"), newRecursionLevel);
			currentLOD = newRecursionLevel;
			GeneratePlanet(false);
		}
	}


}

void AOP_ProceduralPlanet::GenerateHeatMapTex(UOP_PlanetData* planetData)
{
	int resolution = FMath::Sqrt(planetData->UV.Num());

	TArray<FColor> colorMap;
	colorMap.Init(FColor::Black, resolution * resolution);

	// Generate flat array of colors from vertexColor and UV arrays
	for (int i = 0; i < planetData->UV.Num(); i++)
	{
		FColor col = planetData->VertexColours[i].ToFColor(false);
		int xPos = (((planetData->UV[i].X / PI) + 1.0f) / 2.0f) * resolution;
		int yPos = (((planetData->UV[i].Y / PI) + 1.0f) / 2.0f) * resolution;
		int aPos = ((yPos)* resolution) + (xPos);
		if (aPos >= 0 && aPos < colorMap.Num())
		{
			colorMap[((yPos)* resolution) + (xPos)] = col;
		}

	}

	FCreateTexture2DParameters params;
	CombinedNoiseTex = FImageUtils::CreateTexture2D(resolution,
		resolution,
		colorMap,
		this,
		TEXT("CombinedNoise"),
		EObjectFlags::RF_Transient,
		params);
}

bool AOP_ProceduralPlanet::GenerateCubemapNoise()
{
	NoiseGenerator_XPos = CreateSeededNoiseGenerators(Seed);
	NoiseGenerator_XNeg = CreateSeededNoiseGenerators(Seed + 1);
	NoiseGenerator_YPos = CreateSeededNoiseGenerators(Seed + 2);
	NoiseGenerator_YNeg = CreateSeededNoiseGenerators(Seed + 3);
	NoiseGenerator_ZPos = CreateSeededNoiseGenerators(Seed + 4);
	NoiseGenerator_ZNeg = CreateSeededNoiseGenerators(Seed + 5);

	RoughNoiseGenerator_XPos = CreateSeededRoughNoiseGenerators(Seed);
	RoughNoiseGenerator_XNeg = CreateSeededRoughNoiseGenerators(Seed + 1);
	RoughNoiseGenerator_YPos = CreateSeededRoughNoiseGenerators(Seed + 2);
	RoughNoiseGenerator_YNeg = CreateSeededRoughNoiseGenerators(Seed + 3);
	RoughNoiseGenerator_ZPos = CreateSeededRoughNoiseGenerators(Seed + 4);
	RoughNoiseGenerator_ZNeg = CreateSeededRoughNoiseGenerators(Seed + 5);

	if (NoiseGenerator_XPos == nullptr
		|| NoiseGenerator_XNeg == nullptr
		|| NoiseGenerator_YPos == nullptr
		|| NoiseGenerator_YNeg == nullptr
		|| NoiseGenerator_ZPos == nullptr
		|| NoiseGenerator_ZNeg == nullptr
		|| RoughNoiseGenerator_XPos == nullptr
		|| RoughNoiseGenerator_XNeg == nullptr
		|| RoughNoiseGenerator_YPos == nullptr
		|| RoughNoiseGenerator_YNeg == nullptr
		|| RoughNoiseGenerator_ZPos == nullptr
		|| RoughNoiseGenerator_ZNeg == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("AOP_ProceduralPlanet::GenerateCubemapNoise failed"));
		return false;
	}
	
	return true;
}

UUFNNoiseGenerator * AOP_ProceduralPlanet::CreateSeededNoiseGenerators(int32 seed)
{
	UUFNNoiseGenerator* noiseGen = UUFNBlueprintFunctionLibrary::CreateFractalNoiseGenerator(
		this,
		NoiseType,
		seed,
		Frequency,
		FractalGain,
		Interpolation,
		FractalType,
		Octaves,
		Lacunarity
	);

	return noiseGen;
}

UUFNNoiseGenerator * AOP_ProceduralPlanet::CreateSeededRoughNoiseGenerators(int32 seed)
{
	UUFNNoiseGenerator* noiseGen = UUFNBlueprintFunctionLibrary::CreateFractalNoiseGenerator(
		this,
		RoughNoiseType,
		seed,
		RoughFrequency,
		RoughFractalGain,
		RoughInterpolation,
		RoughFractalType,
		RoughOctaves,
		RoughLacunarity
	);

	return noiseGen;
}

float AOP_ProceduralPlanet::GetCubemapHeight(FVector position, FVector normal)
{
	return GetXHeight(normal.X, normal) + GetYHeight(normal.Y, normal) + GetZHeight(normal.Z, normal);
}

float AOP_ProceduralPlanet::GetXHeight(float perc, FVector pos)
{
	pos.X = 0.0f;
	UUFNNoiseGenerator* noiseGen = perc > 0 ? NoiseGenerator_XPos : NoiseGenerator_XNeg;
	UUFNNoiseGenerator* roughNoiseGen = perc > 0 ? RoughNoiseGenerator_XPos : RoughNoiseGenerator_XNeg;
	if (noiseGen != nullptr && roughNoiseGen != nullptr)
	{ return perc * (noiseGen->GetNoise2D(pos.Y, pos.Z) + roughNoiseGen->GetNoise2D(pos.Y, pos.Z)); }
	return 0.0f;
}

float AOP_ProceduralPlanet::GetYHeight(float perc, FVector pos)
{
	pos.Y = 0.0f;
	UUFNNoiseGenerator* noiseGen = perc > 0 ? NoiseGenerator_YPos : NoiseGenerator_YNeg;
	UUFNNoiseGenerator* roughNoiseGen = perc > 0 ? RoughNoiseGenerator_YPos : RoughNoiseGenerator_YNeg;
	if (noiseGen != nullptr && roughNoiseGen != nullptr)
	{ return perc * (noiseGen->GetNoise2D(pos.X, pos.Z) + roughNoiseGen->GetNoise2D(pos.X, pos.Z)); }
	return 0.0f;
}

float AOP_ProceduralPlanet::GetZHeight(float perc, FVector pos)
{
	pos.Z = 0.0f;
	UUFNNoiseGenerator* noiseGen = perc > 0 ? NoiseGenerator_ZPos : NoiseGenerator_ZNeg;
	UUFNNoiseGenerator* roughNoiseGen = perc > 0 ? RoughNoiseGenerator_ZPos : RoughNoiseGenerator_ZNeg;
	if (noiseGen != nullptr && roughNoiseGen != nullptr)
	{ return perc * (noiseGen->GetNoise2D(pos.X, pos.Y) + roughNoiseGen->GetNoise2D(pos.X, pos.Y)); }
	return 0.0f;
}

void AOP_ProceduralPlanet::MakeTestTex()
{
	if (NoiseGenerator_XPos == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("No noisegen ref AOP_ProceduralPlanet::MakeTestTex"));
		return;
	}

	int resolution = 128;
	TArray<FColor> colorMap;
	colorMap.Init(FColor::Black, resolution * resolution);
	float step = 1.0f / resolution;
	int index = 0;
	for (int y = 0; y < resolution; y++)
	{
		for (int x = 0; x < resolution; x++)
		{
			float height = NoiseGenerator_XPos->GetNoise2D(step * x, step * y);
			colorMap[index] = FColor(height * 255, height * 255, height * 255);
			index++;
		}
	}

	FCreateTexture2DParameters params;
	TestHeightNoise = FImageUtils::CreateTexture2D(resolution,
		resolution,
		colorMap,
		this,
		TEXT("Noise"),
		EObjectFlags::RF_Transient,
		params);
}
