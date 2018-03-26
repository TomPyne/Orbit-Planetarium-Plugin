// Tom Pyne - 2018

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "OP_HeightmapDecal.generated.h"

class UTexture2D;

UCLASS()
class ORBITPLANETARIUM_API UOP_HeightmapDecal : public UObject
{
	GENERATED_BODY()

public:

	// Converts the texture 2D into a flat array
	// resolution = the x and y resolution of the power2 image
	// heightDecalTex = the texture of the height decal
	// frequency = maximum amount that can appear on a planet
	void CreateDecal(int resolution, UTexture2D* heightDecalTex, int frequency);

	// Applys the decal to a flat noisemap
	// noiseMap = the flat array of noise data
	// noiseMapResolution = the power2 resolution of the noisemap
	// scale = the scale of the decal when applied to the noisemap
	// num = the number of decals you want to apply
	// additive = should the decal influence the existing data or replace it
	void ApplyDecalToNoiseMap(TArray<float> &noiseMap, int noiseMapResolution, float scale, int num = 1, bool additive = false);

	int MaxFrequency = 0;
private:

	// height data stored as an array of floats
	TArray<float> DecalHeightData;

	// Resolution of the decal
	int Resolution = 128;
};