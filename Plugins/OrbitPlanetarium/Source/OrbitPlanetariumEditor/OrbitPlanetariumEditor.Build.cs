namespace UnrealBuildTool.Rules
{
    public class OrbitPlanetariumEditor : ModuleRules
    {
        public OrbitPlanetariumEditor(ReadOnlyTargetRules Target) : base(Target)
        {
            PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

            PrivateIncludePaths.Add("OrbitPlanetariumEditor/Private");
            PublicIncludePaths.Add("OrbitPlanetariumEditor/Public");

            PublicDependencyModuleNames.AddRange(
                new string[]
                {
                    "Core",
                    "CoreUObject",
                    "Slate",
                    "SlateCore",
                    "Engine",
                    "UnrealEd",
                    "PropertyEditor",
                    "AssetTools",
                    "AssetRegistry",
                    "OrbitPlanetarium"
                }
                );
        }
    }
}