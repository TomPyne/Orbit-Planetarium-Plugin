// Tom Pyne - 2018

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "OP_SplatMaterialData.generated.h"

UCLASS(EditInlineNew)
class ORBITPLANETARIUM_API UOP_SplatMaterialData : public UDataAsset
{
	GENERATED_BODY()
	
public:

	// The material
	UPROPERTY(EditAnywhere)
	class UMaterialInterface* Material;

	// How much influence this material has compared to other materials
	UPROPERTY(EditAnywhere)
	float Influence = 0.0f;	
	
};
