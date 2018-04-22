// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "UnrealFastNoisePlugin/Public/FastNoise/FastNoise.h"
#include "RuntimeMeshComponent.h"
#include "OP_ProceduralPlanet.generated.h"

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

	// Random //////////////////////////////////////////////////////////////////////

	// Use the assigned seed, if false generate a new one each time
	UPROPERTY(EditAnywhere, Category = "Random")
	bool bUseSeed;

	// Seed for RNG
	UPROPERTY(EditAnywhere, Category = "Random")
	int32 Seed = 600;

	// Generation ///////////////////////////////////////////////////////////////////

	UPROPERTY(EditAnywhere, Category = "TerrainGen")
	bool bGenerateDebugTextures;

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

	UPROPERTY(EditAnywhere, Instanced, Category = "Cube")
	TArray<class UOP_SurfaceFeature* > Decals;


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

	UPROPERTY(VisibleAnywhere, Category = TestTex)
	UTexture2D* CombinedNoiseTex;

	UPROPERTY(VisibleAnywhere, Category = TestTex)
	UTexture2D* CombinedSteepnessTex;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Clear the mesh
	void ClearPlanet();

protected:


	// Reference to the player
	UPROPERTY()
	APawn* PlayerPawn;

	// THe runtime mesh component;
	UPROPERTY(EditAnywhere)
	class URuntimeMeshComponent* RTMComponent;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	int GetCurrentLODLevel(FVector target, FVector LODobject, FVector sectionNormal);

private:

	TArray<float> SobelVertexSteepness;


};
