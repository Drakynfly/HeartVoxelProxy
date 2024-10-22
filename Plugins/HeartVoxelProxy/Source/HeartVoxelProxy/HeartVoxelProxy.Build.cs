// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HeartVoxelProxy : ModuleRules
{
	public HeartVoxelProxy(ReadOnlyTargetRules Target) : base(Target)
	{
		HeartCore.ApplySharedModuleSetup(this, Target);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Blood",
				"Core",
				"GameplayTags",
				"HeartCore",
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