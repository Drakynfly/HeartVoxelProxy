// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class HeartRuntimeVoxelEditorTarget : TargetRules
{
	public HeartRuntimeVoxelEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.AddRange( new string[] { "HeartRuntimeVoxel" } );
	}
}