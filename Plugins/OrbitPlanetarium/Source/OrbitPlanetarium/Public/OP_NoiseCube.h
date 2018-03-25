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
	
	// Sample the noise cube
	float SampleNoiseCube(FVector normal);

	// Returns the 6 faces of the cube as UTextures
	TArray<UTexture2D* > GetCubeTextures();

protected:

	// Creates the textures
	void GenerateNoiseCube();

	// The data
	TArray<float> XPosHeight;
	TArray<float> XNegHeight;
	TArray<float> YPosHeight;
	TArray<float> YNegHeight;
	TArray<float> ZPosHeight;
	TArray<float> ZNegHeight;

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

	static UTexture2D* NoiseToTexture(TArray<float> data, int resolution, UObject* outer, FString name);
	
	UFastNoise* CreateNoiseGenerator(FNoiseGeneratorParameters params, int32 seed, UObject* outer);

	// Uses the noise generator to create a heightmap array, samples between 0 and 1
	TArray<float> CreateFlatNoiseArray(UFastNoise* noiseGen, int resolution, float offset);

private:

	// The resolution of each of the cube faces
	int Resolution = 128;

	// 1 / Resolution
	float ResStep;

	// Height for each of the axes
	float GetXHeight(float perc, FVector pos);
	float GetYHeight(float perc, FVector pos);
	float GetZHeight(float perc, FVector pos);
};
