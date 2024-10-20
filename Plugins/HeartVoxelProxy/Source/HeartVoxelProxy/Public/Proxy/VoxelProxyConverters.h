// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "BloodValue.h"
#include "VoxelPinValue.h"
#include "Model/HeartGraphPinDesc.h"

struct FVoxelSerializedPin;
class UVoxelProxyGraph;

namespace Converters
{
	/* Convert a VoxelSerializedPin to a Heart Pin Desc. Uses the owning Proxy Graph to reuse Metadata to wrap the pin type. */
	FHeartGraphPinDesc VoxelPinToHeartPin(UVoxelProxyGraph* ProxyGraph, const FVoxelSerializedPin& InPin, const EHeartPinDirection PinDirection);

	FInstancedStruct VoxelInstancedStructDecay(const FVoxelInstancedStruct& Value);
	FVoxelInstancedStruct VoxelInstancedStructWrap(const FInstancedStruct& Value);

	/* Voxel -> Heart runtime data struct conversion */
	FBloodValue VoxelPinToBlood(const FVoxelPinValue& Value);

	/* Heart -> Voxel runtime data struct conversion */
	FVoxelPinValue BloodToVoxelPin(const FBloodValue& Value, const FVoxelPinType& ExpectedType);
}