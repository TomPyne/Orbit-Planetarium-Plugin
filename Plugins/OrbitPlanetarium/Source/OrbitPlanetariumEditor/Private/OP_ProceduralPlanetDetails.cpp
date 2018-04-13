#include "OP_ProceduralPlanetDetails.h"
#include "Widgets/Input/SButton.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"
#include "IDetailsView.h"
#include "OP_ProceduralPlanet.h"

#define LOCTEXT_NAMESPACE "OP_ProceduralPlanetDetails"

TSharedRef<IDetailCustomization> FOP_ProceduralPlanetDetails::MakeInstance()
{
	return MakeShareable(new FOP_ProceduralPlanetDetails);
}

void FOP_ProceduralPlanetDetails::CustomizeDetails(IDetailLayoutBuilder & DetailBuilder)
{
	IDetailCategoryBuilder& procPlanetCategory = DetailBuilder.EditCategory("Generation");

	const FText GeneratePlanetText = LOCTEXT("GeneratePlanet", "Generate Planet");
	const FText ClearPlanetText = LOCTEXT("ClearPlanet", "Clear Planet");

	// Cache set of selected things
	SelectedObjectsList = DetailBuilder.GetDetailsView().GetSelectedObjects();

	procPlanetCategory.AddCustomRow(GeneratePlanetText, false)
		.NameContent()
		[
			SNullWidget::NullWidget
		]
	.ValueContent()
		.VAlign(VAlign_Center)
		.MaxDesiredWidth(250)
		[
			SNew(SButton)
			.VAlign(VAlign_Center)
		.ToolTipText(LOCTEXT("GeneratePlanetTooltip", "Regenerate the planet from the details in the inspector"))
		.OnClicked(this, &FOP_ProceduralPlanetDetails::ClickedOnGeneratePlanet)
		.IsEnabled(this, &FOP_ProceduralPlanetDetails::GeneratePlanetEnabled)
		.Content()
		[
			SNew(STextBlock)
			.Text(GeneratePlanetText)
		]
		];

	procPlanetCategory.AddCustomRow(ClearPlanetText, false)
		.NameContent()
		[
			SNullWidget::NullWidget
		]
	.ValueContent()
		.VAlign(VAlign_Center)
		.MaxDesiredWidth(250)
		[
			SNew(SButton)
			.VAlign(VAlign_Center)
		.ToolTipText(LOCTEXT("ClearPlanetTooltip", "Clear the Planet data"))
		.OnClicked(this, &FOP_ProceduralPlanetDetails::ClickedOnClearPlanet)
		.IsEnabled(this, &FOP_ProceduralPlanetDetails::ClearPlanetEnabled)
		.Content()
		[
			SNew(STextBlock)
			.Text(ClearPlanetText)
		]
		];
}

FReply FOP_ProceduralPlanetDetails::ClickedOnGeneratePlanet()
{
	// Find first selected procplanet
	AOP_ProceduralPlanet* procPlanet = GetFirstSelectedProceduralPlanet();
	if (procPlanet != nullptr)
	{
		procPlanet->UpdatePlanetMeshSections();
	}

	return FReply::Handled();
}

FReply FOP_ProceduralPlanetDetails::ClickedOnClearPlanet()
{
	AOP_ProceduralPlanet* procPlanet = GetFirstSelectedProceduralPlanet();
	if (procPlanet != nullptr)
	{
		procPlanet->ClearPlanet();
	}

	return FReply::Handled();
}

bool FOP_ProceduralPlanetDetails::GeneratePlanetEnabled() const
{
	return GetFirstSelectedProceduralPlanet() != nullptr;
}

bool FOP_ProceduralPlanetDetails::ClearPlanetEnabled() const
{
	return GetFirstSelectedProceduralPlanet() != nullptr;
}

AOP_ProceduralPlanet* FOP_ProceduralPlanetDetails::GetFirstSelectedProceduralPlanet() const
{
	// Find first selected procedural planet
	AOP_ProceduralPlanet* procPlanet = nullptr;
	for (const TWeakObjectPtr<UObject> &Object : SelectedObjectsList)
	{
		AOP_ProceduralPlanet* testProcPlanet = Cast<AOP_ProceduralPlanet>(Object.Get());
		
		if (testProcPlanet != nullptr && !testProcPlanet->IsTemplate())
		{
			procPlanet = testProcPlanet;
			break;
		}
	}

	return procPlanet;
}
