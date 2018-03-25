// Fill out your copyright notice in the Description page of Project Settings.

#include "SystemInfo.h"
#include "OrbitMessageStructs.h"
#include "Planet.h"

void ASystemInfo::BeginPlay()
{
	Super::BeginPlay();

	// Set up the messaging
	ReceiverEndpoint_RegisterPlanet = FMessageEndpoint::Builder("RegisterPlanet").Handling<FRegisterPlanetMessage>(
		this, &ASystemInfo::HandleRegisterPlanetMessage);
	if (ReceiverEndpoint_RegisterPlanet.IsValid())
	{
		ReceiverEndpoint_RegisterPlanet->Subscribe<FRegisterPlanetMessage>();
	}
}

void ASystemInfo::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ReceiverEndpoint_RegisterPlanet.Reset();

	Super::EndPlay(EndPlayReason);
}

void ASystemInfo::HandleRegisterPlanetMessage(const FRegisterPlanetMessage &Message, const IMessageContextRef & Context)
{
	if (Message.Planet)
	{
		// Add the new planet
		SystemPlanets.Add(Message.Planet);

		for (APlanet* planet : SystemPlanets)
		{
			planet->UpdateInfluencingBodies(SystemPlanets);
		}
	}
}
