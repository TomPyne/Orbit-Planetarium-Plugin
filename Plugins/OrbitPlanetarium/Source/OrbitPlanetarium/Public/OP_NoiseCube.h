// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UnrealFastNoisePlugin/Public/FastNoise/FastNoise.h"
#include "OP_NoiseCube.generated.h"

USTRUCT()
struct FNoiseGeneratorParameters
{
	GENERATED_USTRUCT_BODY()

	EFractalNoiseType NoiseType;
	float Frequency;
	float FractalGain;
	EInterp Interpolation;
	EFractalType FractalType;
	int32 Octaves;
	float Lacunarity;

	FNoiseGeneratorParameters() {}

	FNoiseGeneratorParameters(EFractalNoiseType noiseType,
		float frequency,
		float fractalGain,
		EInterp interpolation,
		EFractalType fractalType,
		int32 octaves,
		float lacunarity)
	{
		NoiseType = noiseType;
		Frequency = frequency;
		FractalGain = fractalGain;
		Interpolation = interpolation;
		FractalType = fractalType;
		Octaves = octaves;
		Lacunarity = lacunarity;
	}
};

class UTexture2D;
class UOP_HeightmapDecal;

/**
 * 
 */
UCLASS()
class ORBITPLANETARIUM_API UOP_NoiseCube : public UObject
{
	GENERATED_BODY()
	
public:

	// Generate the textures and noise
	void Init(int resolution,
		EFractalNoiseType noiseType,
		int32 seed,
		float frequency,
		float fractalGain,
		EInterp interpolation,
		EFractalType fractalType,
		int32 octaves,
		float lacunarity);

	void Init(int resolution,
		EFractalNoiseType noiseType,
		int32 seed,
		float frequency,
		float fractalGain,
		EInterp interpolation,
		EFractalType fractalType,
		int32 octaves,
		float lacunarity,
		TArray<UOP_HeightmapDecal* > decals);
	
	// Sample the noise cube
	float SampleNoiseCube(FVector normal);

	// Sample steepness from the noise cube
	float SampleSteepness(FVector normal);

	// Returns the 6 faces of the cube as UTextures
	TArray<UTexture2D* > GetCubeTextures();

	// Returns the 6 faces of the cube as steepness textures
	TArray<UTexture2D* > GetSteepnessTextures();

protected:

	// Creates the steepness data
	void GenerateSteepnessData();

	// Height data
	TArray<float> XPosHeight;
	TArray<float> XNegHeight;
	TArray<float> YPosHeight;
	TArray<float> YNegHeight;
	TArray<float> ZPosHeight;
	TArray<float> ZNegHeight;

	// Steepness data
	TArray<float> XPosSteepness;
	TArray<float> XNegSteepness;
	TArray<float> YPosSteepness;
	TArray<float> YNegSteepness;
	TArray<float> ZPosSteepness;
	TArray<float> ZNegSteepness;

	// Data stored as textures
	UPROPERTY()
	UTexture2D* XPosTex;
	UPROPERTY()
	UTexture2D* XNegTex;
	UPROPERTY()
	UTexture2D* YPosTex;
	UPROPERTY()
	UTexture2D* YNegTex;
	UPROPERTY()
	UTexture2D* ZPosTex;
	UPROPERTY()
	UTexture2D* ZNegTex;

	// CubeMap
	UPROPERTY()
	UFastNoise* NoiseGenerator_XPos;

	UPROPERTY()
	UFastNoise* NoiseGenerator_XNeg;

	UPROPERTY()
	UFastNoise* NoiseGenerator_YPos;

	UPROPERTY()
	UFastNoise* NoiseGenerator_YNeg;

	UPROPERTY()
	UFastNoise* NoiseGenerator_ZPos;

	UPROPERTY()
	UFastNoise* NoiseGenerator_ZNeg;

	UPROPERTY()
	UFastNoise* DetailNoiseGenerator_XPos;

	UPROPERTY()
	UFastNoise* DetailNoiseGenerator_XNeg;

	UPROPERTY()
	UFastNoise* DetailNoiseGenerator_YPos;

	UPROPERTY()
	UFastNoise* DetailNoiseGenerator_YNeg;

	UPROPERTY()
	UFastNoise* DetailNoiseGenerator_ZPos;

	UPROPERTY()
	UFastNoise* DetailNoiseGenerator_ZNeg;

	static UTexture2D* NoiseToTexture(const TArray<float> &data, int resolution, UObject* outer, FString name);
	
	UFastNoise* CreateNoiseGenerator(FNoiseGeneratorParameters params, int32 seed, UObject* outer);

	// Uses the noise generator to create a heightmap array, samples between 0 and 1
	TArray<float> CreateFlatNoiseArray(UFastNoise * noiseGen, UFastNoise* detailNoiseGen, int resolution, float offset);

	void ApplyHeightDecalsToFace(TArray<float> &data, UOP_HeightmapDecal* decal, int num);

private:

	int Octaves = 1;

	// The resolution of each of the cube faces
	int Resolution = 128;

	// 1 / Resolution
	float ResStep;

	// Height for each of the axes
	float GetXHeight(float perc, FVector pos);
	float GetYHeight(float perc, FVector pos);
	float GetZHeight(float perc, FVector pos);

	

	bool SteepnessDataGenerated();

	// Populates the steepness data arrays based on the height data
	static void CalculateSteepness(const TArray<float> &heightData, TArray<float> &outSteepness, int resolution);

	// Samples a linear array with 2D coordinates, if no resolution is provided it is calculated (slower)
	static float SampleData(int x, int y, const TArray<float> &data, int resolution = 0);
};
