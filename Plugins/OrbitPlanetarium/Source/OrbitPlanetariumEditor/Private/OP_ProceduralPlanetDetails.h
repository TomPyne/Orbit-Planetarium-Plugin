#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "IDetailCustomization.h"

class IDetailLayoutBuilder;
class AOP_ProceduralPlanet;

class FOP_ProceduralPlanetDetails : public IDetailCustomization
{
public:

	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	// handle clicking the generate button
	FReply ClickedOnGeneratePlanet();

	// Handle clicking on clear button
	FReply ClickedOnClearPlanet();

	// Handle clicking on update button
	FReply ClickedOnUpdatePlanet();

	// Is the button enabled
	bool GeneratePlanetEnabled() const;
	bool ClearPlanetEnabled() const;
	bool UpdatePlanetEnabled() const;

	AOP_ProceduralPlanet* GetFirstSelectedProceduralPlanet() const;

	/** Cached array of selected objects */
	TArray< TWeakObjectPtr<UObject> > SelectedObjectsList;
};