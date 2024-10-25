// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "HeartVoxelBlueprintLibrary.generated.h"

struct FVoxelExposedSeed;

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

	UFUNCTION(BlueprintPure, Category = "Voxel")
	static FVoxelExposedSeed MakeRandomVoxelSeed();
};