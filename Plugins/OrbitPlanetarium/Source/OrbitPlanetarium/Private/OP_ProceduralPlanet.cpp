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
#include "OP_HeightmapDecal.h"
#include "OP_SplatMaterialData.h"
#include "Runtime/CoreUObject/Public/UObject/NoExportTypes.h"
#include "PackedNormal.h"

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

	RTMComponent = CreateDefaultSubobject<URuntimeMeshComponent>(TEXT("Mesh"));
	RootComponent = RTMComponent;
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

	// Get references
	PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	// Check the LOD, this also generates the planet if the LOD has changed
	//CheckLODRange(true);

	GetWorld()->GetTimerManager().SetTimer(TimerHandle_UpdateMesh, this, &AOP_ProceduralPlanet::UpdatePlanetMeshSections, MeshUpdateRate, true);
}

void AOP_ProceduralPlanet::GenerateNoiseCubes()
{
	TArray<UOP_HeightmapDecal* > heightMapDecals;
	// Create the HeightmapDecals
	for (UTexture2D* decal : Decals)
	{
		UOP_HeightmapDecal* hmd = NewObject<UOP_HeightmapDecal>(this);
		
		hmd->CreateDecal(decal->GetSizeX(), decal, 10);
		heightMapDecals.Add(hmd);
	}

	NoiseCube = NewObject<UOP_NoiseCube>(this);
	NoiseCube->Init(256, NoiseType, Seed, Frequency, FractalGain, Interpolation, FractalType, Octaves, Lacunarity, heightMapDecals);

	RoughNoiseCube = NewObject<UOP_NoiseCube>(this);
	RoughNoiseCube->Init(256, RoughNoiseType, Seed, RoughFrequency, RoughFractalGain, RoughInterpolation, RoughFractalType, RoughOctaves, RoughLacunarity);
}

TArray<UOP_SectionData*> AOP_ProceduralPlanet::GenerateIcosahedronSectionData(UObject* outer)
{
	if (outer == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Bad Outer provided AOP_ProceduralPlanet::GenerateIcosahedronSectionData"));
	}
	float t = (1.0f + FMath::Sqrt(5.0f)) / 2.0f;

	// Create the 12 vertices of the icosahedron
	FVector v0 = FVector(1, -t, 0);
	FVector v1 = FVector(-1, -t, 0);
	FVector v2 = FVector(1, t, 0);
	FVector v3 = FVector(-1, t, 0);
	FVector v4 = FVector(0, 1, -t);
	FVector v5 = FVector(0, -1, -t);
	FVector v6 = FVector(0, 1, t);
	FVector v7 = FVector(0, -1, t);
	FVector v8 = FVector(-t, 0, 1);
	FVector v9 = FVector(-t, 0, -1);
	FVector v10 = FVector(t, 0, 1);
	FVector v11 = FVector(t, 0, -1);

	// Convert the, to the packed version with normal data
	FRuntimeMeshVertexSimple vs0 = FRuntimeMeshVertexSimple(v0, v0);
	FRuntimeMeshVertexSimple vs1 = FRuntimeMeshVertexSimple(v1, v1);
	FRuntimeMeshVertexSimple vs2 = FRuntimeMeshVertexSimple(v2, v2);
	FRuntimeMeshVertexSimple vs3 = FRuntimeMeshVertexSimple(v3, v3);
	FRuntimeMeshVertexSimple vs4 = FRuntimeMeshVertexSimple(v4, v4);
	FRuntimeMeshVertexSimple vs5 = FRuntimeMeshVertexSimple(v5, v5);
	FRuntimeMeshVertexSimple vs6 = FRuntimeMeshVertexSimple(v6, v6);
	FRuntimeMeshVertexSimple vs7 = FRuntimeMeshVertexSimple(v7, v7);
	FRuntimeMeshVertexSimple vs8 = FRuntimeMeshVertexSimple(v8, v8);
	FRuntimeMeshVertexSimple vs9 = FRuntimeMeshVertexSimple(v9, v9);
	FRuntimeMeshVertexSimple vs10 = FRuntimeMeshVertexSimple(v10, v10);
	FRuntimeMeshVertexSimple vs11 = FRuntimeMeshVertexSimple(v11, v11);

	// Create and initialise the section array
	TArray<UOP_SectionData* > icoSections;
	for (int i = 0; i < 20; i++) { icoSections.Add(NewObject<UOP_SectionData>(outer)); }

	// Create the 20 faces of the icosahedron as seperate mesh sections
	SetupSection(icoSections[0], vs0, vs11, vs5);
	SetupSection(icoSections[1], vs0, vs5, vs1);
	SetupSection(icoSections[2], vs0, vs1, vs7);
	SetupSection(icoSections[3], vs0, vs7, vs10);
	SetupSection(icoSections[4], vs0, vs10, vs11);
	SetupSection(icoSections[5], vs1, vs5, vs9);
	SetupSection(icoSections[6], vs5, vs11, vs4);
	SetupSection(icoSections[7], vs11, vs10, vs2);
	SetupSection(icoSections[8], vs10, vs7, vs6);
	SetupSection(icoSections[9], vs7, vs1, vs8);
	SetupSection(icoSections[10], vs3, vs9, vs4);
	SetupSection(icoSections[11], vs3, vs4, vs2);
	SetupSection(icoSections[12], vs3, vs2, vs6);
	SetupSection(icoSections[13], vs3, vs6, vs8);
	SetupSection(icoSections[14], vs3, vs8, vs9);
	SetupSection(icoSections[15], vs4, vs9, vs5);
	SetupSection(icoSections[16], vs2, vs4, vs11);
	SetupSection(icoSections[17], vs6, vs2, vs10);
	SetupSection(icoSections[18], vs8, vs6, vs7);
	SetupSection(icoSections[19], vs9, vs8, vs1);

	// Return the icosahedron
	return icoSections;
}

void AOP_ProceduralPlanet::SetupSection(UOP_SectionData * sectionData, FRuntimeMeshVertexSimple vs0, FRuntimeMeshVertexSimple vs1, FRuntimeMeshVertexSimple vs2)
{
	sectionData->Vertices.Add(vs0);
	sectionData->Vertices.Add(vs1);
	sectionData->Vertices.Add(vs2);
	sectionData->Triangles.Add(0);
	sectionData->Triangles.Add(1);
	sectionData->Triangles.Add(2);

	// Calc section normal
	sectionData->SectionNormal = ((vs0.Position + vs1.Position + vs2.Position) / 3).GetSafeNormal();
}

int AOP_ProceduralPlanet::GetMiddlePointOnSection(int p1, int p2, UOP_SectionData * sectionData)
{
	if (sectionData == nullptr) { return 0; }

	// Calculate it
	FVector point1 = sectionData->Vertices[p1].Position;
	FVector point2 = sectionData->Vertices[p2].Position;
	FVector middle = FVector(
		(point1.X + point2.X) / 2.0f,
		(point1.Y + point2.Y) / 2.0f,
		(point1.Z + point2.Z) / 2.0f);

	int i = AddVertexToSection(middle, sectionData);
	return i;
}

int AOP_ProceduralPlanet::AddVertexToSection(FVector v, UOP_SectionData * sectionData)
{
	if (sectionData == nullptr) { return 0; }

	double Length = v.Size();
	sectionData->Vertices.Add(FRuntimeMeshVertexSimple(FVector(v.X, v.Y, v.Z), FVector(v.X, v.Y, v.Z)));
	return sectionData->Index++;
}

void AOP_ProceduralPlanet::SubdivideMeshSection(UOP_SectionData * sectionData, int recursion)
{
	if (sectionData == nullptr) { return; }

	for (int i = 0; i < recursion; i++)
	{
		int tris = sectionData->Triangles.Num();
		TArray<int32> tris2;
		for (int j = 0; j < tris; j = j + 3)
		{
			int v1 = sectionData->Triangles[j];
			int v2 = sectionData->Triangles[j + 1];
			int v3 = sectionData->Triangles[j + 2];
			int a = GetMiddlePointOnSection(v1, v2, sectionData);
			int b = GetMiddlePointOnSection(v2, v3, sectionData);
			int c = GetMiddlePointOnSection(v3, v1, sectionData);

			tris2.Add(v1);
			tris2.Add(a);
			tris2.Add(c);
			tris2.Add(v2);
			tris2.Add(b);
			tris2.Add(a);
			tris2.Add(v3);
			tris2.Add(c);
			tris2.Add(b);
			tris2.Add(a);
			tris2.Add(b);
			tris2.Add(c);
		}

		sectionData->Triangles = tris2;
	}
}

void AOP_ProceduralPlanet::ApplyNoiseToMeshSection(UOP_SectionData * sectionData)
{
	if (sectionData == nullptr) { return; }

	FVector pos = GetActorLocation();

	TArray<FRuntimeMeshVertexSimple> v2;
	for (FRuntimeMeshVertexSimple v : sectionData->Vertices)
	{
		FRuntimeMeshVertexSimple temp = v;
		FVector normal = (pos - v.Position).GetSafeNormal();
		temp.Position = GetVertexPositionFromNoise(v.Position, normal);
		v2.Add(temp);
	}

	sectionData->Vertices = v2;
}

FVector AOP_ProceduralPlanet::GetVertexPositionFromNoise(FVector v, FVector n)
{
	if (NoiseCube == nullptr || RoughNoiseCube == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("No NoiseCube or RoughNoiseCube found AOP_ProceduralPlanet::GetVertexPositionFromNoise"));
		return v;
	}

	// Get height from noise cubes
	float height = NoiseCube->SampleNoiseCube(n);
	height += RoughnessInfluence * RoughNoiseCube->SampleNoiseCube(n);

	// Boosts the output
	height *= Boost;

	// Convert to spherical coords
	FOP_SphericalCoords sCoords = FOP_SphericalCoords(v);
	sCoords.Radius += Radius + (height * Scale);

	// convert back to cartesian and return
	return sCoords.ToCartesian();
}


void AOP_ProceduralPlanet::UpdatePlanetMeshSections()
{
	// Check RTMC exists
	// if not return
	if (RTMComponent == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("No RTMComponent AOP_ProceduralPlanet::UpdatePlanetMeshSections"));
		return;
	}

	if (PlayerPawn == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("No PlayerPawn AOP_ProceduralPlanet::UpdatePlanetMeshSections"));
		return;
	}

	if (NoiseCube == nullptr || RoughNoiseCube == nullptr)
	{
		GenerateNoiseCubes();
	}

	// Check mesh has already been created
	bool updateMesh = RTMComponent->GetNumSections() == 20;

	// Check base icosahedron exists
	// If not, create it
	if (CachedIcosahedron.Num() != 20)
	{
		CachedIcosahedron = GenerateIcosahedronSectionData(this);
	}

	// The sections that will be used to generate the mesh
	TArray<UOP_SectionData* > sections;

	// Indicating which sections actually need updating here could potentially save a lot of
	// processing time
	TArray<uint8> sectionsToUpdate;

	// Try and get a cached version of each section, and create the section if the cache is missing
	for (int i = 0; i < 20; i++)
	{
		/* Use base icosahedron to determine section normal.
		   Get the LOD level of the section */
		uint8 currLOD = GetCurrentLODLevel(PlayerPawn->GetActorLocation(), GetActorLocation(), CachedIcosahedron[i]->SectionNormal);

		// If the LOD has changed we doneed to update this mesh section
		if (currLOD != PreviousLODs[i])
		{
			PreviousLODs[i] = currLOD;
			sectionsToUpdate.Add(i);
			// Try and get the cached Lod
			UOP_SectionData* sectionData = TryGetCachedSectionData(currLOD, i);

			if (sectionData == nullptr)
			{
				// Create a new section and copy the data from the cached icosahedron
				sectionData = NewObject<UOP_SectionData>(this);
				sectionData->Copy(CachedIcosahedron[i]);

				// Subd that section LOD number of times
				SubdivideMeshSection(sectionData, currLOD);

				// Apply the noise
				ApplyNoiseToMeshSection(sectionData);

				// cache final data
				CacheSectionData(sectionData, currLOD, i);
			}

			sections.Add(sectionData);
		}
		
	}

	// If the mesh needs creating
	if (!updateMesh)
	{
		for (int i = 0; i < CachedIcosahedron.Num(); i++)
		{
			RTMComponent->CreateMeshSection(i, CachedIcosahedron[i]->Vertices, CachedIcosahedron[i]->Triangles);
		}
	}

		for (int i = 0; i < sectionsToUpdate.Num(); i++)
		{ 
			int index = sectionsToUpdate[i];
			RTMComponent->UpdateMeshSection(index, sections[i]->Vertices, sections[i]->Triangles);
		}
}

UOP_SectionData* AOP_ProceduralPlanet::TryGetCachedSectionData(uint8 LODLevel, int section)
{
	UOP_SectionDataContainer **cachedDataPtr = CachedSectionLODLevels.Find(LODLevel);
	UOP_SectionDataContainer* cachedData = nullptr;
	if (cachedDataPtr != nullptr)
	{
		cachedData = *cachedDataPtr;
		if (cachedData->Data.Num() == 20 && section >= 0 && section < 20)
		{
			return cachedData->Data[section];
		}
		else
		{
			//UE_LOG(LogTemp, Warning, TEXT("Cached data array size: %d , section number: %d AOP_ProceduralPlanet::TryGetCachedSectionData"), cachedData->Data.Num(), section);
			return nullptr;
		}
	}
	
	return nullptr;
}

void AOP_ProceduralPlanet::CacheSectionData(UOP_SectionData * sectionData, uint8 LODLevel, int section)
{
	UOP_SectionDataContainer **cachedDataPtr = CachedSectionLODLevels.Find(LODLevel);
	{
		if (cachedDataPtr == nullptr)
		{
			UOP_SectionDataContainer* sdc = NewObject<UOP_SectionDataContainer>(this);
			for (int i = 0; i < 20; i++)
			{
				sdc->Data.Add(NewObject<UOP_SectionData>(this));
			}

			sdc->Data[section] = sectionData;
		}
	}
	
	//sdc->Data = sectionData;
	//CachedSectionLODLevels.Add()
}

// Called every frame
void AOP_ProceduralPlanet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//CheckLODRange(false);
}

void AOP_ProceduralPlanet::GeneratePlanet(bool bIgnoreLOD)
{
	float sTime = FPlatformTime::Seconds();
	float dTime = 0.0f;
	float lTime = 0.0f;
	// If no procedural mesh component no point executing
	if (RTMComponent == nullptr) { return; }

	TArray<FRuntimeMeshTangent> rtTangents;

	// If there is no noiseCube or a seed hasn't been set, generate them
	if (NoiseCube == nullptr || RoughNoiseCube == nullptr || !bUseSeed)
	{
		GenerateNoiseCubes();
	}
	
	dTime = FPlatformTime::Seconds();
	UE_LOG(LogTemp, Warning, TEXT("Stage , NoiseCube created: t = %f , d = %f"), (FPlatformTime::Seconds() - sTime), dTime);

	UOP_PlanetData* planetData = TryGetCachedLOD(currentLOD);
	if (bIgnoreLOD)
	{
		planetData = NewObject<UOP_PlanetData>();
	}
	if (planetData == nullptr) { return; }

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

		lTime = FPlatformTime::Seconds() - sTime;
		UE_LOG(LogTemp, Warning, TEXT("Stage 1, Vertices generated and subdivided: t= %f , d = %f"), (FPlatformTime::Seconds() - sTime), (FPlatformTime::Seconds() - dTime));
		dTime = FPlatformTime::Seconds();

		// Create the Triangles
		for (FOP_TriangleIndices tri : faces)
		{
			planetData->Triangles.Add(tri.V1);
			planetData->Triangles.Add(tri.V2);
			planetData->Triangles.Add(tri.V3);
		}

		UE_LOG(LogTemp, Warning, TEXT("Tris created: %f , d = %f"), (FPlatformTime::Seconds() - sTime), (FPlatformTime::Seconds() - dTime));
		dTime = FPlatformTime::Seconds();

		//// if the searched for vert hasn't been discovered yet start at the index from the last search
		//int highestVert = 0;
		//int lastIndex;
		//for (int i = 0; i, planetData->Vertices.Num(); i++)
		//{
		//	// Find the triangle
		//	for (FOP_TriangleIndices tri : faces)
		//	{

		//	}
		//}

		FVector position = GetActorLocation();

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

			float steepness = NoiseCube->SampleSteepness(normal);

			vcValue = FMath::Clamp(vcValue, 0.0f, 1.0f);
			FLinearColor vlColour = FLinearColor(vcValue, vcValue, vcValue);
			FColor vColour = FColor(vcValue * 255, vcValue * 255, vcValue * 255);
			planetData->VertexColours.Add(vColour);
			planetData->LinearVertexColours.Add(vlColour);

			// Redistribution
			height < 0.0f ? FMath::Pow(height * -1.0f, Redistribution) * -1.0f : FMath::Pow(height, Redistribution);

			// Convert to spherical coordinates to apply the height to the radius
			FOP_SphericalCoords sCoords = FOP_SphericalCoords(planetData->Vertices[i]);
			//sCoords.Radius = Radius + (height * Scale);
			sCoords.Radius += -Radius + (height * Scale);

			planetData->UV.Add(FVector2D(sCoords.Theta, sCoords.Phi));
			PolarVertices3D.Add(sCoords);
		}

		UE_LOG(LogTemp, Warning, TEXT("Stage 2, vertex position modified: %f , d = %f"), (FPlatformTime::Seconds() - sTime), (FPlatformTime::Seconds() - dTime));
		dTime = FPlatformTime::Seconds();

		// Empty the vertex array and repopulate it with the polar vertices
		FVector lastV = FVector::ZeroVector;
		planetData->Vertices.Empty();
		
		for (int i = 0; i < PolarVertices3D.Num(); i++)
		{
			FVector cartesianVertex = PolarVertices3D[i].ToCartesian();
			planetData->Vertices.Add(cartesianVertex);
			/*if ((i + 1) % 3 == 0)
			{
				
			}*/
		}

		UE_LOG(LogTemp, Warning, TEXT("Stage 3, Spherical to cartesian: %f , d = %f"), (FPlatformTime::Seconds() - sTime), (FPlatformTime::Seconds() - dTime));
		dTime = FPlatformTime::Seconds();

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
			rtTangents.Add(FRuntimeMeshTangent(tangent, true));
			}
			i += 3;
		}

		UE_LOG(LogTemp, Warning, TEXT("Stage 5, Tangents calculated: %f , d = %f"), (FPlatformTime::Seconds() - sTime), (FPlatformTime::Seconds() - dTime));
		dTime = FPlatformTime::Seconds();
		
	}



	UE_LOG(LogTemp, Warning, TEXT("MeshData: %s"), *planetData->ToString());
	dTime = FPlatformTime::Seconds();

	// Create the mesh
	RTMComponent->ClearAllMeshSections();
	RTMComponent->CreateMeshSection(
		0,
		planetData->Vertices,
		planetData->Triangles,
		planetData->Normals,
		planetData->UV,
		planetData->VertexColours,
		rtTangents,
		false);

	UE_LOG(LogTemp, Warning, TEXT("Stage 6, Mesh generated: %f , d = %f"), (FPlatformTime::Seconds() - sTime), (FPlatformTime::Seconds() - dTime));
	dTime = FPlatformTime::Seconds();

	// Set the material
	if (Material)
	{
		RTMComponent->SetMaterial(0, Material);
	}

	// Try to cache the LOD
	if (!bIgnoreLOD)
	{
		CacheLOD(currentLOD, planetData);
	}

	// Generate image from heightmap
	GenerateHeatMapTex(planetData);
	//GenerateSteepnessMapTex(planetData);

	

	cubemap = NoiseCube->GetCubeTextures();
	Steepnessmap = NoiseCube->GetSteepnessTextures();

	UE_LOG(LogTemp, Warning, TEXT("Stage 7, Textures generated: t= %f , d = "), (FPlatformTime::Seconds() - sTime), (FPlatformTime::Seconds() - dTime));
	dTime = FPlatformTime::Seconds();
}

void AOP_ProceduralPlanet::ClearPlanet()
{
	if (RTMComponent != nullptr)
	{
		RTMComponent->ClearAllMeshSections();
	}
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

	TArray<FColor> heightColorMap;
	TArray<FColor> steepnessColorMap;
	heightColorMap.Init(FColor::Black, resolution * resolution);
	steepnessColorMap.Init(FColor::Black, resolution * resolution);


	// Generate flat array of colors from vertexColor and UV arrays
	for (int i = 0; i < planetData->UV.Num(); i++)
	{
		// Height map
		FColor col = planetData->VertexColours[i];
		int xPos = (((planetData->UV[i].X / PI) + 1.0f) / 2.0f) * resolution;
		int yPos = (((planetData->UV[i].Y / PI) + 1.0f) / 2.0f) * resolution;
		int aPos = ((yPos)* resolution) + (xPos);
		if (aPos >= 0 && aPos < heightColorMap.Num())
		{
			heightColorMap[((yPos)* resolution) + (xPos)] = col;
		}
	}

	for (int y = 0; y < resolution - 1; y++)
	{
		for (int x = 0; x < resolution - 1; x++)
		{
			float dx = heightColorMap[(y * resolution) + (x + 1)].R / 255.0f;
			float dy = heightColorMap[((y + 1) * resolution) + x].R / 255.0f;

			float steepness = fabs(dx) + fabs(dy);
			FColor steepnessColor = steepness > 0.5f ? FColor(255, 255 - (steepness * 255), 0) : FColor(steepness * 255, 255, 0);
			steepnessColorMap[(y * resolution + x)] = steepnessColor;
		}
	}

	FCreateTexture2DParameters hParams;
	CombinedNoiseTex = FImageUtils::CreateTexture2D(resolution,
		resolution,
		heightColorMap,
		this,
		TEXT("CombinedNoise"),
		EObjectFlags::RF_Transient,
		hParams);

	FCreateTexture2DParameters sParams;
	CombinedSteepnessTex = FImageUtils::CreateTexture2D(resolution,
		resolution,
		steepnessColorMap,
		this,
		TEXT("CombinedSteepnessNoise"),
		EObjectFlags::RF_Transient,
		sParams);
}

void AOP_ProceduralPlanet::GenerateSteepnessMapTex(UOP_PlanetData * planetData)
{
	if (NoiseCube == nullptr) { return; }

	FVector origin = GetActorLocation();

	int resolution = FMath::Sqrt(planetData->UV.Num());
	TArray<FColor> colorMap;
	colorMap.Init(FColor::Black, resolution * resolution);
	for (int i = 0; i < planetData->UV.Num(); i++)
	{
		float steepness = NoiseCube->SampleSteepness(origin - planetData->Vertices[i]);
		int xPos = (((planetData->UV[i].X / PI) + 1.0f) / 2.0f) * resolution;
		int yPos = (((planetData->UV[i].Y / PI) + 1.0f) / 2.0f) * resolution;
		int aPos = ((yPos)* resolution) + (xPos);
		if (aPos >= 0 && aPos < colorMap.Num())
		{
			colorMap[((yPos)* resolution) + (xPos)] = FColor(steepness * 255, steepness * 255, steepness * 255);
		}
		
	}

	FCreateTexture2DParameters params;
	CombinedSteepnessTex = FImageUtils::CreateTexture2D(resolution,
		resolution,
		colorMap,
		this,
		TEXT("CombinedSteepness"),
		EObjectFlags::RF_Transient,
		params);
}

int AOP_ProceduralPlanet::GetCurrentLODLevel(FVector target, FVector LODobject, FVector sectionNormal)
{
	if (PlayerPawn == nullptr) { return 0; }

	// Get camera position normal
	FVector camPosNrm = (GetActorLocation() - PlayerPawn->GetActorLocation()).GetSafeNormal();
	float angle = FMath::Acos(FVector::DotProduct(sectionNormal, -camPosNrm));

	// Check if the section is occluded
	if (angle * (180.0f / PI) > 90.0f) { return 1; }

	// if not then calculate LOD from distance
	float dist = (LODobject - target).Size();

	for (int i = 0; i < LODDistances.Num(); i++)
	{
		if (dist < LODDistances[i])
		{
			return i;
		}
	}
	// if outside all bounds return 0 indicating min
	return 0;
}
