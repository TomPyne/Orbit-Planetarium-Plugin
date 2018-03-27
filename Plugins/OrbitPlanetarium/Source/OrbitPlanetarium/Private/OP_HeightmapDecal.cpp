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
	
	int decalRes = Resolution * scale;

	if (decalRes > noiseMapResolution)
	{
		UE_LOG(LogTemp, Warning, TEXT("UHeightmapDecal::ApplyDecalToNoiseMap attempting to place a decal bigger than the noisemap"));
		return;
	}

	if (noiseMap.Num() != noiseMapResolution * noiseMapResolution)
	{
		UE_LOG(LogTemp, Warning, TEXT("UHeightmapDecal::ApplyDecalToNoiseMap noiseMapResolution is not equal to the size of the noisemap"));
		return;
	}

	// For how many decals we want to paint
	for (int i = 0; i < num; i++)
	{
		// Pick a random start location that fits the decal within the bounds of the noisemap
		int startX = FMath::RandRange(0, (noiseMapResolution - decalRes) - 1);
		int startY = FMath::RandRange(0, (noiseMapResolution - decalRes) - 1);

		// Record the starting pixel on the base noisemap
		int baseY = startY;
		int baseX = startX;
		
		int step = 1 / scale;

		for (int y = startY; y < startY + decalRes; y++)
		{
			for (int x = startX; x < startX + decalRes; x++)
			{
				int index = (((y - startY) * Resolution) * step) + ((x - startX) * step);
				if (additive)
				{
					noiseMap[(y * noiseMapResolution) + x] += DecalHeightData[index];
				}
				else
				{
					noiseMap[(y * noiseMapResolution) + x] = DecalHeightData[index];
				}
			}
		}
	}	
}
