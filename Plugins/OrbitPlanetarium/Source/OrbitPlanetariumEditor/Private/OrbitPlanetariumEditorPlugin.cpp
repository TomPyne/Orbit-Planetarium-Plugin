// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "OP_ProceduralPlanet.h"
#include "IOrbitPlanetariumEditorPlugin.h"
#include "OP_ProceduralPlanetDetails.h"


class FOrbitPlanetariumEditorPlugin : public IOrbitPlanetariumEditorPlugin
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE(FOrbitPlanetariumEditorPlugin, OrbitPlanetariumEditor)

void FOrbitPlanetariumEditorPlugin::StartupModule()
{
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.RegisterCustomClassLayout(AOP_ProceduralPlanet::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FOP_ProceduralPlanetDetails::MakeInstance));
	}
}


void FOrbitPlanetariumEditorPlugin::ShutdownModule()
{

}