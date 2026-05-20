// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NEONDRIFT : ModuleRules
{
	public NEONDRIFT(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG", "Slate", "SlateCore" });

		PublicIncludePaths.AddRange(new string[] { 
			"NEONDRIFT/Public",
			"NEONDRIFT/Public/Core",
			"NEONDRIFT/Public/Actors",
			"NEONDRIFT/Public/Enemies",
			"NEONDRIFT/Public/Player",
			"NEONDRIFT/Public/Turrets",
			"NEONDRIFT/Public/Items",
			"NEONDRIFT/Public/UI"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
