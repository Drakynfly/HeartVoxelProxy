// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HeartVoxelProxy : ModuleRules
{
	public HeartVoxelProxy(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Blood",
				"Core",
				"GameplayTags",
				"Heart",
				"VoxelCore",
				"VoxelGraphCore"
			});

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine"
			});

		PrivateIncludePaths.AddRange(
			new string[]
			{
				"HeartVoxelProxy/ThirdParty"
			});

	}
}