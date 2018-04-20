// Tom Pyne - 2018

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "OP_SurfaceFeature.generated.h"

/**
 * 
 */
UCLASS(EditInlineNew)
class ORBITPLANETARIUM_API UOP_SurfaceFeature : public UDataAsset
{
	GENERATED_BODY()
	
public:

	// The Heightmap decal
	UPROPERTY(EditAnywhere)
	class UTexture2D* HeightMap;
	
	// The maximum amount that can appear on a surface
	UPROPERTY(EditAnywhere )
	int MaxAmount = 10;	
};
