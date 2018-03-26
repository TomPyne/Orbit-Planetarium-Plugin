// Tom Pyne - 2018

#include "OP_HeightmapDecal.h"
#include "Engine/Texture2D.h"


void UOP_HeightmapDecal::CreateDecal(int resolution, UTexture2D* heightDecalTex, int frequency)
{
	Resolution = resolution;
	MaxFrequency = frequency;

	FTexture2DMipMap* mipmap = &heightDecalTex->PlatformData->Mips[0];
	FByteBulkData* rawImageData = &mipmap->BulkData;
	FColor* formattedImageData = static_cast<FColor*>(rawImageData->Lock(LOCK_READ_ONLY));

	int sqrdRes = resolution * resolution;
	DecalHeightData.Init(0.0f, resolution * resolution);
	for (int i = 0; i < sqrdRes; i++)
	{
		DecalHeightData[i] = formattedImageData[i].R / 255.0f;
	}

	rawImageData->Unlock();
}

void UOP_HeightmapDecal::ApplyDecalToNoiseMap(TArray<float>& noiseMap, int noiseMapResolution, float scale, int num, bool additive)
{
	if (DecalHeightData.Num() != Resolution * Resolution || DecalHeightData.Num() < 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("UHeightmapDecal::ApplyDecalToNoiseMap DecalHeightData has not been initialised correctly first"));
		return;
	}

	if (Resolution / scale > noiseMapResolution)
	{
		UE_LOG(LogTemp, Warning, TEXT("UHeightmapDecal::ApplyDecalToNoiseMap attempting to place a decal bigger than the noisemap"));
		return;
	}

	if (noiseMap.Num() != noiseMapResolution * noiseMapResolution)
	{
		UE_LOG(LogTemp, Warning, TEXT("UHeightmapDecal::ApplyDecalToNoiseMap noiseMapResolution is not equal to the size of the noisemap"));
		return;
	}

	for (int i = 0; i < num; i++)
	{
		// Pick a random start location that fits the decal within the bounds of the noisemap
		int startX = FMath::RandRange(0, (noiseMapResolution - Resolution) - 1);
		int startY = FMath::RandRange(0, (noiseMapResolution - Resolution) - 1);

		int increment = 1 / scale;
		increment = increment < 1 ? 1 : increment;

		for (int y = 0; y < Resolution; y = y + increment)
		{
			for (int x = 0; x < Resolution; x = x + increment)
			{
				if (additive)
				{
					noiseMap[((startY + y) * noiseMapResolution) + (startX + x)] += DecalHeightData[(y * Resolution) + x];
				}
				else
				{
					noiseMap[((startY + y) * noiseMapResolution) + (startX + x)] = DecalHeightData[(y * Resolution) + x];
				}
			}
		}
	}
}
