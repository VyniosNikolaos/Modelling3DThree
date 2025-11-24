// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Modelling3DThree : ModuleRules
{
	public Modelling3DThree(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"ProceduralMeshComponent"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"Modelling3DThree",
			"Modelling3DThree/Variant_Platforming",
			"Modelling3DThree/Variant_Platforming/Animation",
			"Modelling3DThree/Variant_Combat",
			"Modelling3DThree/Variant_Combat/AI",
			"Modelling3DThree/Variant_Combat/Animation",
			"Modelling3DThree/Variant_Combat/Gameplay",
			"Modelling3DThree/Variant_Combat/Interfaces",
			"Modelling3DThree/Variant_Combat/UI",
			"Modelling3DThree/Variant_SideScrolling",
			"Modelling3DThree/Variant_SideScrolling/AI",
			"Modelling3DThree/Variant_SideScrolling/Gameplay",
			"Modelling3DThree/Variant_SideScrolling/Interfaces",
			"Modelling3DThree/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
