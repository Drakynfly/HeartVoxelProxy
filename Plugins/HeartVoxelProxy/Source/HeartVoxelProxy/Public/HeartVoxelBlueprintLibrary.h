// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "VoxelExposedSeed.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HeartVoxelBlueprintLibrary.generated.h"

/**
 *
 */
UCLASS()
class HEARTVOXELPROXY_API UHeartVoxelBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Util to allow UMG to generate combo box.
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (WorldContext = "ContextObject"))
	static TArray<FName> GetAllVoxelChannels(UObject* ContextObject);

	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static TArray<FText> GetCollisionTypeOptions();

	UFUNCTION(BlueprintPure, Category = "Voxel", meta = (WorldContext = "ContextObject"))
	static FVoxelExposedSeed MakeRandomVoxelSeed();
};