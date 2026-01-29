// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TheSecond : ModuleRules
{
	public TheSecond(ReadOnlyTargetRules Target) : base(Target)
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
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"TheSecond",
			"TheSecond/Variant_Platforming",
			"TheSecond/Variant_Platforming/Animation",
			"TheSecond/Variant_Combat",
			"TheSecond/Variant_Combat/AI",
			"TheSecond/Variant_Combat/Animation",
			"TheSecond/Variant_Combat/Gameplay",
			"TheSecond/Variant_Combat/Interfaces",
			"TheSecond/Variant_Combat/UI",
			"TheSecond/Variant_SideScrolling",
			"TheSecond/Variant_SideScrolling/AI",
			"TheSecond/Variant_SideScrolling/Gameplay",
			"TheSecond/Variant_SideScrolling/Interfaces",
			"TheSecond/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
