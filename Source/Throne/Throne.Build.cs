// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class Throne : ModuleRules
{
	public Throne(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG", "Niagara" });

		PrivateDependencyModuleNames.AddRange(new string[] { "AnimGraph", "BlueprintGraph", "UnrealEd" });

		// Uncomment if you are using Slate UI
		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		PrivateDependencyModuleNames.AddRange(new string[] { "OnlineSubsystem", "OnlineSubsystemUtils" });
    }
}
