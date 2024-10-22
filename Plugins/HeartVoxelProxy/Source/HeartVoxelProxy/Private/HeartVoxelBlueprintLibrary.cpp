// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "HeartVoxelBlueprintLibrary.h"
#include "VoxelExposedSeed.h"
#include "Channel/VoxelWorldChannel.h"

#define LOCTEXT_NAMESPACE "HeartVoxelBlueprintLibrary"

TArray<FName> UHeartVoxelBlueprintLibrary::GetAllVoxelChannels(UObject* ContextObject)
{
	if (IsValid(ContextObject))
	{
		return FVoxelWorldChannelManager::Get(ContextObject->GetWorld())->GetValidChannelNames().ToConstArray_Unsafe();
	}
	return {};
}

TArray<FText> UHeartVoxelBlueprintLibrary::GetCollisionTypeOptions()
{
	static const TArray<FText> Options{
		LOCTEXT("CollisionType0", "No Collision"),
		LOCTEXT("CollisionType1", "Query Only (No Physics Collision)"),
		LOCTEXT("CollisionType2", "Physics Only (No Query Collision)"),
		LOCTEXT("CollisionType3", "Collision Enabled (Query and Physics)"),
		LOCTEXT("CollisionType4", "Probe Only (Contact Data, No Query or Physics Collision)"),
		LOCTEXT("CollisionType5", "Query and Probe (Query Collision and Contact Data, No Physics Collision)")
	};
	return Options;
}

FVoxelExposedSeed UHeartVoxelBlueprintLibrary::MakeRandomVoxelSeed()
{
	FVoxelExposedSeed ExposedSeed;
	ExposedSeed.Randomize();
	return ExposedSeed;
}

#undef LOCTEXT_NAMESPACE