// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UnrealFastNoisePlugin/Public/FastNoise/FastNoise.h"
#include "ProceduralMeshComponent.h"
#include "ProceduralIcosahedron.generated.h"

USTRUCT()
struct FTriangleIndices
{
	GENERATED_USTRUCT_BODY()

	int V1, V2, V3;

	FTriangleIndices() { V1 = V2 = V3 = 0; }
	FTriangleIndices(int v1, int v2, int v3)
	{
		V1 = v1;
		V2 = v2;
		V3 = v3;
	}
};

USTRUCT()
struct FSphericalCoords
{
	GENERATED_USTRUCT_BODY()

	float Theta;
	float Phi;
	float Radius;

	FSphericalCoords() { Theta = Phi = Radius = 0.0f; }
	// Spherical constructor
	FSphericalCoords(float theta, float phi, float radius)
	{
		Theta = theta;
		Phi = phi;
		Radius = radius;
	}
	// Cartesian Conversion constructor
	FSphericalCoords(FVector pos)
	{
		Radius = pos.Size();
		Phi = FMath::Acos(pos.Z / Radius);
		Theta = FMath::Atan2(pos.Y , pos.X);
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
class ORBIT_API UPlanetData : public UObject
{
	GENERATED_BODY()

public:
	// Default Constructor
	UPlanetData() {}

	// Constructor
	/*void Init(TArray<FVector> verts,
		TArray<int32> tris,
		TArray<FVector> norms,
		TArray<FVector2D> uv,
		TArray<FLinearColor> vColour);*/

	UPROPERTY()
	TArray<FVector> Vertices;

	UPROPERTY()
	TArray<int32> Triangles;

	UPROPERTY()
	TArray<FVector> Normals;

	UPROPERTY()
	TArray<FVector2D> UV;

	UPROPERTY()
	TArray<FLinearColor> VertexColours;

	UPROPERTY()
	TArray<FProcMeshTangent> Tangents;

	int32 Index = 0;

	FORCEINLINE bool IsPopulated() { return Vertices.Num() > 0 && Triangles.Num() > 0; }

	FString ToString();
};

UCLASS()
class ORBIT_API AProceduralIcosahedron : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProceduralIcosahedron();

	UPROPERTY(EditAnywhere, Category = Test)
	bool OverrideLOD = false;

	UPROPERTY(EditAnywhere, Category = Icosahedron)
	uint8 Recursionlevel = 0;

	UPROPERTY(EditAnywhere, Category = "Random")
	bool bUseSeed;

	UPROPERTY(EditAnywhere, Category = "Random")
	int32 Seed = 600;

	UPROPERTY()
	class UProceduralMeshComponent* ProcMeshComponent;

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
		float HeightBoost = 1.0f;

	UPROPERTY(EditAnywhere, Category = "TerrainGen")
		float Scale = 1.0f;

	UPROPERTY(EditAnywhere, Category = "TerrainGen")
		float Radius = 50.0f;

	UPROPERTY(EditAnywhere, Category = "TerrainGen")
		float Resolution = 128;

	UPROPERTY(EditAnywhere, Category = "TerrainGen")
		float Redistribution = 2.5f;

	UPROPERTY(EditAnywhere, Category = "TerrainGen")
		float RandVarianceRange = 0.1f;

	UPROPERTY(EditAnywhere, Category = "TerrainGen")
	float MinWaterLevel = 0.2f;
	

	UPROPERTY(EditAnywhere, Category = "TerrainGen")
	FLinearColor Color;

	UPROPERTY(EditAnywhere, Category = "TerrainGen")
	class UMaterialInstance* Material;

	UPROPERTY(EditAnywhere, Category = "Atmosphere")
	float AtmosphereMinHeight = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Atmosphere")
	float AtmosphereMaxHeight = 10000.0f;

	UPROPERTY(VisibleAnywhere, Category = "Atmosphere")
	float CurrentAtmosphere = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Atmosphere")
	class AAtmosphericFog* AtmosphereComponent;

	// How far above the radius a position when queried for surface pos
	UPROPERTY(EditAnywhere, Category = "Surface")
	float SurfaceOffset = 100.0f;

	UPROPERTY(VisibleAnywhere, Category = LOD)
	float LOD1Distance = 100000.0f;

	float LOD2Distance = 267210.0f; //89070.0f;
	float LOD3Distance = 226710.0f; //75570.0f;
	float LOD4Distance = 186210.0f; //62070.0f;
	float LOD5Distance = 136710.0f; //45570.0f;
	float LOD6Distance = 96210.0f; //32070.0f;
	float LOD7Distance = 55710.0f; //18570.0f;
	float LOD8Distance = 33000.0f; //11000.0f;

	float LOD1DistanceSqrd = 0.0f;
	float LOD2DistanceSqrd = 0.0f;
	float LOD3DistanceSqrd = 0.0f;
	float LOD4DistanceSqrd = 0.0f;
	float LOD5DistanceSqrd = 0.0f;
	float LOD6DistanceSqrd = 0.0f;
	float LOD7DistanceSqrd = 0.0f;
	float LOD8DistanceSqrd = 0.0f;

	// Get Point and facing direction so a camera knows how to orient itself on the surface
	UFUNCTION(BlueprintCallable)
	void GetSurfacePosition(FVector currentPosition, FVector &surfacePosition, FRotator &lookRotation);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void CreateIcosahedron(bool Update = false);

	void CreatePlanet();

	static int AddVertex(FVector v, UPlanetData* planet);
	int GetMiddlePoint(int p1, int p2, UPlanetData* planet);

	FORCEINLINE FVector GetFaceMidPoint(FVector a, FVector b, FVector c){return (a + b + c) / 3.0f;}

	TArray<FVector> Vertices;

	// Create the noise Generator
	void GenerateNoise();

	// The noise generator
	UFastNoise* NoiseGen;

	UPROPERTY()
	class UUFNNoiseGenerator* NoiseGenerator;

	// Calculate and cache LOD levels so they only have to be calculated once
	UPROPERTY()
	TMap<uint8, UPlanetData* > CachedLODLevels;

	// Checks if LOD is cached and returns a TArray, if the TArray is empty, there are no cached LODs
	UPlanetData* TryGetCachedLOD(uint8 LOD);

	void CacheLOD(uint8 LOD, UPlanetData* data);


	// Store Player Pawn
	UPROPERTY()
	APawn* PlayerPawn;

	FVector2D MercatorUV(float longitude, float latitude, float radius);

	// d is unit vector to sphere origin
	FVector2D SphereUV(FVector d);


	// Atmosphere
	void UpdateAtmosphericFogAmount(float dist);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
