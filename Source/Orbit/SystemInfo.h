// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Messaging.h"
#include "SystemInfo.generated.h"

/**
 * 
 */
UCLASS()
class ORBIT_API ASystemInfo : public AInfo
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = Planets)
	TArray<class APlanet*> SystemPlanets;

	UPROPERTY(EditAnywhere, Category = Time)
		float TimeScale = 1.0f;

public:

	TArray<class APlanet*> GetPlanets;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	FMessageEndpointPtr ReceiverEndpoint_RegisterPlanet;

	void HandleRegisterPlanetMessage(const struct FRegisterPlanetMessage &Message, const IMessageContextRef &Context);
	
};
