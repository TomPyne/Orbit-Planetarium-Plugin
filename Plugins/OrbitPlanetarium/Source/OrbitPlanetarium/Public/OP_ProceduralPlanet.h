// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "UnrealFastNoisePlugin/Public/FastNoise/FastNoise.h"
#include "RuntimeMeshComponent.h"
#include "OP_ProceduralPlanet.generated.h"

USTRUCT()
struct FOP_TriangleIndices
{
	GENERATED_USTRUCT_BODY()

		int V1, V2, V3;

	FOP_TriangleIndices() { V1 = V2 = V3 = 0; }
	FOP_TriangleIndices(int v1, int v2, int v3)
	{
		V1 = v1;
		V2 = v2;
		V3 = v3;
	}
};

USTRUCT()
struct FOP_SphericalCoords
{
	GENERATED_USTRUCT_BODY()

		float Theta;
	float Phi;
	float Radius;

	FOP_SphericalCoords() { Theta = Phi = Radius = 0.0f; }
	// Spherical constructor
	FOP_SphericalCoords(float theta, float phi, float radius)
	{
		Theta = theta;
		Phi = phi;
		Radius = radius;
	}
	// Cartesian Conversion constructor
	FOP_SphericalCoords(FVector pos)
	{
		Radius = pos.Size();
		Phi = FMath::Acos(pos.Z / Radius);
		Theta = FMath::Atan2(pos.Y, pos.X);
	}

	FVector ToCartesian()
	{
		FVector cart;
		cart.X = Radius * FMath::Sin(Phi) * FMath::Cos(Theta);
		cart.Y = Radius * FMath::Sin(Phi) * FMath::Sin(Theta);
		cart.Z = Radius * FMath::Cos(Phi);
		return cart;
	}
};

UCLASS()
class ORBITPLANETARIUM_API UOP_PlanetData : public UObject
{
	GENERATED_BODY()

public:
	// Default Constructor
	UOP_PlanetData() {}

	UPROPERTY()
	TArray<FVector> Vertices;

	UPROPERTY()
	TArray<int32> Triangles;

	UPROPERTY()
	TArray<FVector> Normals;

	UPROPERTY()
	TArray<FVector2D> UV;

	UPROPERTY()
	TArray<FLinearColor> LinearVertexColours;

	UPROPERTY()
	TArray<FColor> VertexColours;

	UPROPERTY()
	TArray<FProcMeshTangent> Tangents;

	int32 Index = 0;

	FORCEINLINE bool IsPopulated() { return Vertices.Num() > 0 && Triangles.Num() > 0; }

	FString ToString();
};

UCLASS()
class UOP_SectionData : public UObject
{
	GENERATED_BODY()

public:

	UOP_SectionData() {}

	TArray<int32> Triangles;
	TArray<FRuntimeMeshVertexSimple> Vertices;

	// The normal of this section to tell when it is visible to the player or occluded
	// on a sphere we know if the angle between the camera normal and face normal is less than
	// 90 it is occluded
	FVector SectionNormal;

	int Index = 3;

	FORCEINLINE void Copy(UOP_SectionData* other)
	{
		if (other != nullptr)
		{
			Triangles = other->Triangles;
			Vertices = other->Vertices;
			SectionNormal = other->SectionNormal;
			Index = other->Index;
		}
	}
};

UCLASS()
class UOP_SectionDataContainer : public UObject
{
	GENERATED_BODY()

public:

	TArray<UOP_SectionData*> Data;

	UOP_SectionDataContainer() {}
};

class UTexture2D;

UCLASS()
class ORBITPLANETARIUM_API AOP_ProceduralPlanet : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOP_ProceduralPlanet();


	static const int NUM_SECTIONS = 20;

	FTimerHandle TimerHandle_UpdateMesh;
	
	UPROPERTY(EditAnywhere)
	float MeshUpdateRate = 2.0f;

	// LOD ////////////////////////////////////////////////////////////////////////

	// Use the assigned LOD over the one generated
	UPROPERTY(EditAnywhere, Category = LOD)
	bool bOverrideLOD = false;

	// The current LOD level or the LOD level used on startup if bOverrideLOD is true
	UPROPERTY(EditAnywhere, Category = LOD)
	uint8 currentLOD = 0;

	// Random //////////////////////////////////////////////////////////////////////

	// Use the assigned seed, if false generate a new one each time
	UPROPERTY(EditAnywhere, Category = "Random")
	bool bUseSeed;

	// Seed for RNG
	UPROPERTY(EditAnywhere, Category = "Random")
	int32 Seed = 600;

	// Generation ///////////////////////////////////////////////////////////////////

	// Noise type to use
	UPROPERTY(EditAnywhere, Category = "TerrainGen")
	EFractalNoiseType NoiseType;

	UPROPERTY(EditAnywhere, Category = "TerrainGen")
	int32 Octaves = 6;

	UPROPERTY(EditAnywhere, Category = "TerrainGen")
	float Frequency = 0.5f;

	UPROPERTY(EditAnywhere, Category = "TerrainGen")
	EFractalType FractalType;

	UPROPERTY(EditAnywhere, Category = "TerrainGen")
	float FractalGain = 2.0f;

	UPROPERTY(EditAnywhere, Category = "TerrainGen")
	float Lacunarity = 0.4f;

	UPROPERTY(EditAnywhere, Category = "TerrainGen")
	EInterp Interpolation;

	UPROPERTY(EditAnywhere, Category = "TerrainGen")
	float Scale = 1.0f;

	UPROPERTY(EditAnywhere, Category = "TerrainGen")
	float Radius = 50.0f;

	UPROPERTY(EditAnywhere, Category = "TerrainGen")
	float Redistribution = 2.5f;

	UPROPERTY(EditAnywhere, Category = "TerrainGen")
	float MinWaterLevel = 0.2f;

	// ROUGH //////////////////////////////

	// Noise type to use
	UPROPERTY(EditAnywhere, Category = "TerrainGen")
		EFractalNoiseType RoughNoiseType;

	UPROPERTY(EditAnywhere, Category = "TerrainGen")
		int32 RoughOctaves = 6;

	UPROPERTY(EditAnywhere, Category = "TerrainGen")
		float RoughFrequency = 0.5f;

	UPROPERTY(EditAnywhere, Category = "TerrainGen")
		EFractalType RoughFractalType;

	UPROPERTY(EditAnywhere, Category = "TerrainGen")
		float RoughFractalGain = 2.0f;

	UPROPERTY(EditAnywhere, Category = "TerrainGen")
		float RoughLacunarity = 0.4f;

	UPROPERTY(EditAnywhere, Category = "TerrainGen")
		EInterp RoughInterpolation;

	// Detail ///////////////////////////////////////////////////////////////////

	// The material to apply to the mesh
	UPROPERTY(EditAnywhere, Category = "Detail")
	class UMaterialInterface* Material;

	UPROPERTY(EditAnywhere, Category = "Detail")
	class UTexture2D* TestHeightDecal;

	UPROPERTY(EditAnywhere, Category = "Detail")
		class UTexture2D* TestHeightNoise;

	// CUBE ///////////////////////////////////////////////////////////////////

	UPROPERTY()
	class UOP_NoiseCube* NoiseCube;

	UPROPERTY()
	class UOP_NoiseCube* RoughNoiseCube;

	// Creates the UOP_NoiseCubes using the parameters
	void GenerateNoiseCubes();

	UPROPERTY(EditAnywhere, Category = "Cube")
	TArray<UTexture2D*> cubemap;

	UPROPERTY(EditAnywhere, Category = "Cube")
	TArray<UTexture2D*> Steepnessmap;

	UPROPERTY(EditAnywhere, Category = "Cube")
	float RoughnessInfluence = 0.2f;

	UPROPERTY(EditAnywhere, Category = "Cube")
	float Boost = 1.4f;

	UPROPERTY(EditAnywhere, Category = "Cube")
	TArray<UTexture2D*> Decals;

	// MATERIAL ///////////////////////////////////////////////////////////////////
	TArray<class UOP_SplatMaterialData* > SplatMaterials;

	// SECTIONDATA ///////////////////////////////////////////////////////////////////

	// Cache a version of the icosahedron for quickly accessing basic section data
	UPROPERTY()
		TArray<UOP_SectionData* > CachedIcosahedron;

	UPROPERTY()
	TMap<uint8, UOP_SectionDataContainer* > CachedSectionLODLevels;

	static TArray<UOP_SectionData* > GenerateIcosahedronSectionData(UObject* outer);

	static UOP_SectionData* SetupSection(FRuntimeMeshVertexSimple vs0, FRuntimeMeshVertexSimple vs1, FRuntimeMeshVertexSimple vs2, UObject* outer);

	static int GetMiddlePointOnSection(int p1, int p2, UOP_SectionData* sectionData);
	static int AddVertexToSection(FVector v, UOP_SectionData* sectionData);

	static void SubdivideMeshSection(UOP_SectionData* sectionData, int recursion);

	void ApplyNoiseToMeshSection(UOP_SectionData* sectionData);

	void GetVertexPositionFromNoise(FRuntimeMeshVertexSimple &vert, FVector n);
	void UpdatePlanetMeshSections();

	uint8 PreviousLODs[20];

	// Sets outSectionData to the cached value if there is one, if not sets it to nullptr and
	// returns false
	UOP_SectionData* TryGetCachedSectionData(uint8 LODLevel, int section);

	// Caches the section data
	void CacheSectionData(UOP_SectionData* sectionData, uint8 LODLevel, int section);

	// LOD ///////////////////////////////////////////////////////////////////

	// Each array value represents the upper limit of each LOD range, if 0 is 100
	// then while the distance is < 100 the LOD is set to 0
	UPROPERTY(EditAnywhere, Category = LOD)
	TArray<float> LODDistances;
	
	float LOD1Distance = 300000.0f;
	float LOD2Distance = 267210.0f; //89070.0f;
	float LOD3Distance = 226710.0f; //75570.0f;
	float LOD4Distance = 186210.0f; //62070.0f;
	float LOD5Distance = 136710.0f; //45570.0f;
	float LOD6Distance = 96210.0f; //32070.0f;
	float LOD7Distance = 55710.0f; //18570.0f;
	float LOD8Distance = 33000.0f; //11000.0f;

	// pre-calculate LOD levels
	float LOD1DistanceSqrd = 0.0f;
	float LOD2DistanceSqrd = 0.0f;
	float LOD3DistanceSqrd = 0.0f;
	float LOD4DistanceSqrd = 0.0f;
	float LOD5DistanceSqrd = 0.0f;
	float LOD6DistanceSqrd = 0.0f;
	float LOD7DistanceSqrd = 0.0f;
	float LOD8DistanceSqrd = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = TestTex)
	UTexture2D* CombinedNoiseTex;

	UPROPERTY(VisibleAnywhere, Category = TestTex)
	UTexture2D* CombinedSteepnessTex;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Generate the planet
	void GeneratePlanet(bool bIgnoreLOD);

	// Clear the mesh
	void ClearPlanet();



protected:

	// Calculate and cache LOD levels so they only have to be calculated once
	UPROPERTY()
	TMap<uint8, UOP_PlanetData* > CachedLODLevels;

	// Reference to the player
	UPROPERTY()
	APawn* PlayerPawn;

	// The procedural mesh component
	UPROPERTY()
	class UProceduralMeshComponent* ProcMeshComponent;

	// THe runtime mesh component;
	UPROPERTY(EditAnywhere)
	class URuntimeMeshComponent* RTMComponent;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Add a vertex to the planet data
	static int AddVertex(FVector v, UOP_PlanetData* planet);
	static int GetMiddlePoint(int p1, int p2, UOP_PlanetData* planet);

	// Checks if LOD is cached and returns a TArray, if the TArray is empty, there are no cached LODs
	UOP_PlanetData* TryGetCachedLOD(uint8 LOD);
	void CacheLOD(uint8 LOD, UOP_PlanetData* data);

	// Check LOD range and create mesh based on that range, bForceGeneration is used if you
	// want to update the mesh even if the LOD is the same
	void CheckLODRange(bool bForceGeneration);

	void GenerateHeatMapTex(UOP_PlanetData* planetData);

	void GenerateSteepnessMapTex(UOP_PlanetData* planetData);

	int GetCurrentLODLevel(FVector target, FVector LODobject, FVector sectionNormal);

private:

	TArray<float> SobelVertexSteepness;


};
