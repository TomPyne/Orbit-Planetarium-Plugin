// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "SettingsInfo.generated.h"

/**
 * 
 */
UCLASS()
class ORBIT_API ASettingsInfo : public AInfo
{
	GENERATED_BODY()
	

protected:

	// Property for editing the timescale
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Time)
		float UpdateTimeScale;

};
