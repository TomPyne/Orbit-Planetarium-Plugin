// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "OrbitMessageStructs.generated.h"

USTRUCT()
struct FRegisterPlanetMessage
{
	GENERATED_USTRUCT_BODY()

	class APlanet* Planet;

	FRegisterPlanetMessage() { Planet = nullptr; }
	FRegisterPlanetMessage(class APlanet* planet) {
		Planet = planet;
	}
};

/**
 * 
 */
UCLASS()
class ORBIT_API UOrbitMessageStructs : public UObject
{
	GENERATED_BODY()
	
	
	
	
};
