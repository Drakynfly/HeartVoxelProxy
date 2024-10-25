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

FVoxelExposedSeed UHeartVoxelBlueprintLibrary::MakeRandomVoxelSeed()
{
	FVoxelExposedSeed ExposedSeed;
	ExposedSeed.Randomize();
	return ExposedSeed;
}

#undef LOCTEXT_NAMESPACE