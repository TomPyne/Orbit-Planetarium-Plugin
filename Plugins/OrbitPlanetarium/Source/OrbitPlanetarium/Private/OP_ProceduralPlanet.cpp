// Fill out your copyright notice in the Description page of Project Settings.

#include "OP_ProceduralPlanet.h"
#include "FastNoise.h"
#include "UnrealFastNoisePlugin/Public/UFNBlueprintFunctionLibrary.h"
#include "UnrealFastNoisePlugin/Public/UFNNoiseGenerator.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "ImageUtils.h"
#include "Engine/Texture2D.h"
#include "OP_NoiseCube.h"
#include "OP_HeightmapDecal.h"
#include "OP_SurfaceFeature.h"
#include "Runtime/CoreUObject/Public/UObject/NoExportTypes.h"

// Sets default values
AOP_ProceduralPlanet::AOP_ProceduralPlanet()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RTMComponent = CreateDefaultSubobject<URuntimeMeshComponent>(TEXT("Mesh"));
	RootComponent = RTMComponent;

	// Ensure previous LODs are set to a high number to ensure LOD 0 is generated
	for (int i = 0; i < NUM_SECTIONS; i++)
	{
		PreviousLODs[i] = 127;
	}
}

// Called when the game starts or when spawned
void AOP_ProceduralPlanet::BeginPlay()
{
	Super::BeginPlay();

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
	for (UOP_SurfaceFeature* decal : Decals)
	{
		if (decal && decal->HeightMap)
		{
			UOP_HeightmapDecal* hmd = NewObject<UOP_HeightmapDecal>(this);
			hmd->CreateDecal(decal->HeightMap->GetSizeX(), decal->HeightMap, decal->MaxAmount);
			heightMapDecals.Add(hmd);
		}		
	}

	NoiseCube = NewObject<UOP_NoiseCube>(this);
	NoiseCube->Init(1024, NoiseType, Seed, Frequency, FractalGain, Interpolation, FractalType, Octaves, Lacunarity);

	RoughNoiseCube = NewObject<UOP_NoiseCube>(this);
	//RoughNoiseCube->Init(1024, RoughNoiseType, Seed, RoughFrequency, RoughFractalGain, RoughInterpolation, RoughFractalType, RoughOctaves, RoughLacunarity, heightMapDecals);

	if (bGenerateDebugTextures)
	{
		cubemap = NoiseCube->GetCubeTextures();
		//Steepnessmap = NoiseCube->GetSteepnessTextures();
	}

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

	// Create the 20 faces of the icosahedron as seperate mesh sections
	icoSections.Add(SetupSection( vs0, vs11, vs5, outer));
	icoSections.Add(SetupSection(vs0, vs5, vs1, outer));
	icoSections.Add(SetupSection(vs0, vs1, vs7, outer));
	icoSections.Add(SetupSection(vs0, vs7, vs10, outer));
	icoSections.Add(SetupSection(vs0, vs10, vs11, outer));
	icoSections.Add(SetupSection(vs1, vs5, vs9, outer));
	icoSections.Add(SetupSection(vs5, vs11, vs4, outer));
	icoSections.Add(SetupSection(vs11, vs10, vs2, outer));
	icoSections.Add(SetupSection(vs10, vs7, vs6, outer));
	icoSections.Add(SetupSection(vs7, vs1, vs8, outer));
	icoSections.Add(SetupSection(vs3, vs9, vs4, outer));
	icoSections.Add(SetupSection(vs3, vs4, vs2, outer));
	icoSections.Add(SetupSection(vs3, vs2, vs6, outer));
	icoSections.Add(SetupSection(vs3, vs6, vs8, outer));
	icoSections.Add(SetupSection(vs3, vs8, vs9, outer));
	icoSections.Add(SetupSection(vs4, vs9, vs5, outer));
	icoSections.Add(SetupSection(vs2, vs4, vs11, outer));
	icoSections.Add(SetupSection(vs6, vs2, vs10, outer));
	icoSections.Add(SetupSection(vs8, vs6, vs7, outer));
	icoSections.Add(SetupSection(vs9, vs8, vs1, outer));

	// Return the icosahedron
	return icoSections;
}

UOP_SectionData* AOP_ProceduralPlanet::SetupSection(FRuntimeMeshVertexSimple vs0, FRuntimeMeshVertexSimple vs1, FRuntimeMeshVertexSimple vs2, UObject* outer)
{
	UOP_SectionData* sectionData = NewObject<UOP_SectionData>(outer);
	sectionData->Vertices.Add(vs0);
	sectionData->Vertices.Add(vs1);
	sectionData->Vertices.Add(vs2);
	sectionData->Triangles.Add(0);
	sectionData->Triangles.Add(1);
	sectionData->Triangles.Add(2);

	// Calc section normal
	sectionData->SectionNormal = ((vs0.Position + vs1.Position + vs2.Position) / 3).GetSafeNormal();

	return sectionData;
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
		//FRuntimeMeshVertexSimple temp = v;
		FVector normal = (pos - v.Position).GetSafeNormal();
		GetVertexPositionFromNoise(v, normal);
		v2.Add(v);
		/*temp.Position = GetVertexPositionFromNoise(v.Position, normal);
		v2.Add(temp);*/
	}

	sectionData->Vertices = v2;
}

void AOP_ProceduralPlanet::GetVertexPositionFromNoise(FRuntimeMeshVertexSimple &vert, FVector n)
{
	if (NoiseCube == nullptr || RoughNoiseCube == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("No NoiseCube or RoughNoiseCube found AOP_ProceduralPlanet::GetVertexPositionFromNoise"));
		return;
	}

	// Get height from noise cubes
	float height = NoiseCube->SampleNoiseCube(n);
	//height += RoughnessInfluence * RoughNoiseCube->SampleNoiseCube(n);

	// Clamp the height from range -1..1 to 0..1
	height = (height + 1.0f) / 2.0f;

	// Redistribution
	height = pow(height, Redistribution);

	// Convert to spherical coords
	FOP_SphericalCoords sCoords = FOP_SphericalCoords(vert.Position);
	sCoords.Radius += Radius + (height * Scale);

	// convert back to cartesian and apply to vertex
	vert.Position = sCoords.ToCartesian();
	
	// VertexColor
	vert.Color = FColor(255 * height, 255 * height, 255 * height);
	
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
	bool updateMesh = RTMComponent->GetNumSections() == NUM_SECTIONS;

	// Check base icosahedron exists
	// If not, create it
	if (CachedIcosahedron.Num() != NUM_SECTIONS)
	{
		CachedIcosahedron = GenerateIcosahedronSectionData(this);
	}

	// The sections that will be used to generate the mesh
	TArray<UOP_SectionData* > sections;

	// Indicating which sections actually need updating here could potentially save a lot of
	// processing time
	TArray<uint8> sectionsToUpdate;

	// Try and get a cached version of each section, and create the section if the cache is missing
	for (int i = 0; i < NUM_SECTIONS; i++)
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
			if (Material) { RTMComponent->SetMaterial(i, Material); }
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
		if (cachedData->Data.Num() == NUM_SECTIONS && section >= 0 && section < NUM_SECTIONS)
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
			for (int i = 0; i < NUM_SECTIONS; i++)
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
}

void AOP_ProceduralPlanet::ClearPlanet()
{
	if (RTMComponent != nullptr)
	{
		RTMComponent->ClearAllMeshSections();
	}
}

/*void AOP_ProceduralPlanet::GenerateHeatMapTex(UOP_PlanetData* planetData)
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
}*/

/*void AOP_ProceduralPlanet::GenerateSteepnessMapTex(UOP_PlanetData * planetData)
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
}*/

int AOP_ProceduralPlanet::GetCurrentLODLevel(FVector target, FVector LODobject, FVector sectionNormal)
{
	if (PlayerPawn == nullptr) { return 0; }

	// Get camera position normal
	FVector camPosNrm = (GetActorLocation() - PlayerPawn->GetActorLocation()).GetSafeNormal();
	float angle = FMath::Acos(FVector::DotProduct(sectionNormal, -camPosNrm));

	// Check if the section is occluded
	if (angle * (180.0f / PI) > 70.0f) { return 1; }

	// if not then calculate LOD from distance
	float dist = (LODobject - target).Size();

	for (int i = 0; i < LODDistances.Num(); i++)
	{
		if (fabs(Radius - dist) < LODDistances[i])
		{

			return LODDistances.Num() - i;
		}
	}
	// if outside all bounds return 0 indicating min
	return 0;
}
