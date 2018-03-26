// Fill out your copyright notice in the Description page of Project Settings.

#include "OP_NoiseCube.h"
#include "Engine/Texture2D.h"
#include "ImageUtils.h"
#include "UnrealFastNoisePlugin/Public/UFNBlueprintFunctionLibrary.h"
#include "OP_HeightmapDecal.h"


void UOP_NoiseCube::Init(int resolution,
	EFractalNoiseType noiseType,
	int32 seed,
	float frequency,
	float fractalGain,
	EInterp interpolation,
	EFractalType fractalType,
	int32 octaves,
	float lacunarity)
{
	Resolution = resolution;
	ResStep = 1.0f / resolution;

	FNoiseGeneratorParameters params = FNoiseGeneratorParameters(noiseType,
		frequency,
		fractalGain,
		interpolation,
		fractalType,
		octaves,
		lacunarity
	);

	// X+
	NoiseGenerator_XPos = CreateNoiseGenerator(params, seed, this);
	XPosHeight = CreateFlatNoiseArray(NoiseGenerator_XPos, resolution, 0.0f);
	// X-
	NoiseGenerator_XNeg = CreateNoiseGenerator(params, seed + 10, this);
	XNegHeight = CreateFlatNoiseArray(NoiseGenerator_XNeg, resolution, 1.0f);
	// Y+
	NoiseGenerator_YPos = CreateNoiseGenerator(params, seed + 20, this);
	YPosHeight = CreateFlatNoiseArray(NoiseGenerator_YPos, resolution, 2.0f);
	// Y-
	NoiseGenerator_YNeg = CreateNoiseGenerator(params, seed + 30, this);
	YNegHeight = CreateFlatNoiseArray(NoiseGenerator_YNeg, resolution, 3.0f);
	// Z+
	NoiseGenerator_ZPos = CreateNoiseGenerator(params, seed + 40, this);
	ZPosHeight = CreateFlatNoiseArray(NoiseGenerator_ZPos, resolution, 4.0f);
	// Z-
	NoiseGenerator_ZNeg = CreateNoiseGenerator(params, seed + 50, this);
	ZNegHeight = CreateFlatNoiseArray(NoiseGenerator_ZNeg, resolution, 5.0f);
}


void UOP_NoiseCube::Init(int resolution, EFractalNoiseType noiseType, int32 seed, float frequency, float fractalGain, EInterp interpolation, EFractalType fractalType, int32 octaves, float lacunarity, TArray<UOP_HeightmapDecal*> decals)
{
	Init(resolution, noiseType, seed, frequency, fractalGain, interpolation, fractalType, octaves, lacunarity);
	for (UOP_HeightmapDecal* hmd : decals)
	{
		if (hmd != nullptr)
		{
			int xn, xp, yn, yp, zn, zp;
			xn = xp = yn = yp = zn = zp = 0;
			for (int i = 0; i < hmd->MaxFrequency; i++)
			{
				switch (FMath::RandRange(0, 5))
				{
				case 0:	xp++;
					break;
				case 1:	xn++;
					break;
				case 2:	yp++;
					break;
				case 3:	yn++;
					break;
				case 4:	zp++;
					break;
				case 5:	zn++;
					break;
				}
			}

			if (xp > 0) ApplyHeightDecalsToFace(XPosHeight, hmd, xp);
			if (xn > 0) ApplyHeightDecalsToFace(XNegHeight, hmd, xn);
			if (yp > 0) ApplyHeightDecalsToFace(YPosHeight, hmd, yp);
			if (yn > 0) ApplyHeightDecalsToFace(YNegHeight, hmd, yn);
			if (zp > 0) ApplyHeightDecalsToFace(ZPosHeight, hmd, zp);
			if (zn > 0) ApplyHeightDecalsToFace(ZNegHeight, hmd, zn);
		}

	}
}

float UOP_NoiseCube::SampleNoiseCube(FVector normal)
{
	return GetXHeight(normal.X, normal) + GetYHeight(normal.Y, normal) + GetZHeight(normal.Z, normal);
}

TArray<UTexture2D*> UOP_NoiseCube::GetCubeTextures()
{
	TArray<UTexture2D*> cubeTextures;
	cubeTextures.Add(NoiseToTexture(XPosHeight, Resolution, this, TEXT("XPos")));
	cubeTextures.Add(NoiseToTexture(XNegHeight, Resolution, this, TEXT("XNeg")));
	cubeTextures.Add(NoiseToTexture(YPosHeight, Resolution, this, TEXT("YPos")));
	cubeTextures.Add(NoiseToTexture(YNegHeight, Resolution, this, TEXT("YNeg")));
	cubeTextures.Add(NoiseToTexture(ZPosHeight, Resolution, this, TEXT("ZPos")));
	cubeTextures.Add(NoiseToTexture(ZNegHeight, Resolution, this, TEXT("ZNeg")));

	return cubeTextures;
}

void UOP_NoiseCube::GenerateNoiseCube()
{

}

UTexture2D * UOP_NoiseCube::NoiseToTexture(TArray<float> data, int resolution, UObject* outer, FString name)
{
	TArray<FColor> colorMap;
	colorMap.Init(FColor::Black, resolution * resolution);
	float step = 1.0f / resolution;
	int index = 0;
	for (int y = 0; y < resolution; y++)
	{
		for (int x = 0; x < resolution; x++)
		{
			float height = data[(y * resolution) + x];
			colorMap[index] = FColor(height * 255, height * 255, height * 255);
			index++;
		}
	}

	FString imgName = TEXT("Noise ") + name;

	FCreateTexture2DParameters params;
	return FImageUtils::CreateTexture2D(resolution,
		resolution,
		colorMap,
		outer,
		imgName,
		EObjectFlags::RF_Transient,
		params);
}

UFastNoise * UOP_NoiseCube::CreateNoiseGenerator(FNoiseGeneratorParameters params, int32 seed, UObject* outer)
{

	UFastNoise* noiseGen = NewObject<UFastNoise>(outer);

	switch (params.NoiseType)
	{
	case EFractalNoiseType::FractalGradient:
		noiseGen->SetNoiseType(ENoiseType::GradientFractal);
		break;
	case EFractalNoiseType::FractalSimplex:
		noiseGen->SetNoiseType(ENoiseType::SimplexFractal);
		break;
	case EFractalNoiseType::FractalValue:
		noiseGen->SetNoiseType(ENoiseType::ValueFractal);
		break;
	}

	noiseGen->SetSeed(seed);
	noiseGen->SetFractalOctaves(params.Octaves);
	noiseGen->SetFrequency(params.Frequency);
	noiseGen->SetFractalType(params.FractalType);
	noiseGen->SetFractalGain(params.FractalGain);
	noiseGen->SetFractalLacunarity(params.Lacunarity);
	noiseGen->SetInterp(params.Interpolation);

	return noiseGen;
}

TArray<float> UOP_NoiseCube::CreateFlatNoiseArray(UFastNoise * noiseGen, int resolution, float offset)
{
	if (noiseGen == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("UOP_NoiseCube::CreateFlatNoiseArray noiseGen is null"));
		return TArray<float>();
	}
	
	// Init the array to the desired resolution
	TArray<float> flatArray;
	flatArray.Init(0.0f, resolution * resolution);

	// Constrain the sampled noise to between 0 and 1 for consistency between resolutions
	float step = 1.0f / resolution;

	for (int y = 0; y < resolution; y++)
	{
		for (int x = 0; x < resolution; x++)
		{
			// Get the height at the coordinate
			float xPos = offset + (x * step);
			float yPos = y * step;
			flatArray[(y * resolution) + x] = noiseGen->GetNoise2D(xPos, yPos);
		}
	}

	return flatArray;
}

void UOP_NoiseCube::ApplyHeightDecalsToFace(TArray<float> &data, UOP_HeightmapDecal * decal, int num)
{
	float scale = FMath::RandRange(0.2f, 1.0f);
	decal->ApplyDecalToNoiseMap(data, Resolution, scale, num, false);
}

float UOP_NoiseCube::GetXHeight(float perc, FVector pos)
{
	int32 y = ((pos.Y + 1.0f) / 2.0f) / ResStep;
	int32 z = ((pos.Z + 1.0f) / 2.0f) / ResStep;
	int32 index = ((z * Resolution) + y);
	if (index >= XPosHeight.Num() || index >= XNegHeight.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("UOP_NoiseCube::GetXHeight out of array bounds"));
		return 0.0f;
	}
	return FMath::Abs(perc) * (perc > 0.0f ? XPosHeight[index] : XNegHeight[FMath::Abs(index)]);
}

float UOP_NoiseCube::GetYHeight(float perc, FVector pos)
{
	int32 x = ((pos.X + 1.0f) / 2.0f) / ResStep;
	int32 z = ((pos.Z + 1.0f) / 2.0f) / ResStep;
	int32 index = ((z * Resolution) + x);
	if (index >= YPosHeight.Num() || index >= YNegHeight.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("UOP_NoiseCube::GetYHeight out of array bounds"));
		return 0.0f;
	}
	return FMath::Abs(perc) * (perc > 0.0f ? YPosHeight[index] : YNegHeight[FMath::Abs(index)]);
}

float UOP_NoiseCube::GetZHeight(float perc, FVector pos)
{
	//int32 x = FMath::Abs(pos.X / ResStep);
	//int32 y = FMath::Abs(pos.Y / ResStep);
	int32 x = ((pos.X + 1.0f) / 2.0f) / ResStep;
	int32 y = ((pos.Y + 1.0f) / 2.0f) / ResStep;
	int32 index = ((y * Resolution) + x);
	if (index >= ZPosHeight.Num() || index >= ZNegHeight.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("UOP_NoiseCube::GetZHeight out of array bounds"));
		return 0.0f;
	}
	return FMath::Abs(perc) * (perc > 0.0f ? ZPosHeight[index] : ZNegHeight[FMath::Abs(index)]);
}
