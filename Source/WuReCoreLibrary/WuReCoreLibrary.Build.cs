// Copyright Epic Games, Inc. All Rights Reserved.

using System;
using UnrealBuildTool;

public class WuReCoreLibrary : ModuleRules
{
	public WuReCoreLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);

		var publicDependencies = new string[]
		{
			"Core",
			"UMG",
			"InputCore", "CommonInput", "EnhancedInput",
			"Slate",
			"SlateCore",
			"OnlineSubsystem",
			"OnlineSubsystemSteam",
			"Json", "JsonUtilities",
		};

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			Array.Resize(ref publicDependencies, publicDependencies.Length + 3);
			publicDependencies[publicDependencies.Length - 3] = "DLSSBlueprint";
			publicDependencies[publicDependencies.Length - 2] = "NISBlueprint";
			publicDependencies[publicDependencies.Length - 1] = "StreamlineBlueprint";
		}
			
		PublicDependencyModuleNames.AddRange(publicDependencies);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
