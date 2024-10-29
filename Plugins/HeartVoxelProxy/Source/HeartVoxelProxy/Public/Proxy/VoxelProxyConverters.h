// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "BloodValue.h"
#include "VoxelGraph.h"
#include "VoxelPinValue.h"
#include "Model/HeartGraphPinDesc.h"

namespace Heart::API
{
	class FNodeEdit;
}

struct FVoxelPinMetadata;
struct FVoxelSerializedNode;
struct FVoxelSerializedPin;
class UVoxelProxyGraph;

namespace Converters
{
	bool LIFTED_LoadSerializedNode(UVoxelProxyGraph* ProxyGraph, const FVoxelSerializedNode& SerializedNode);

	void CreateVoxelProxyNode(Heart::API::FNodeEdit& Edit, UVoxelProxyGraph* ProxyGraph, const FVoxelSerializedNode& Node);

	UVoxelProxyGraph* CreateVoxelProxy(UObject* Outer, UVoxelGraph* GraphToProxy);

	/* Convert a VoxelSerializedPin to a Heart Pin Desc. Uses the owning Proxy Graph to reuse Metadata to wrap the pin type. */
	FHeartGraphPinDesc VoxelPinToHeartPin(UVoxelProxyGraph* ProxyGraph, const FVoxelSerializedPin& InPin, const FVoxelPinMetadata& PinMetadata, const EHeartPinDirection PinDirection);

	FInstancedStruct VoxelInstancedStructDecay(const FVoxelInstancedStruct& Value);
	FVoxelInstancedStruct VoxelInstancedStructWrap(const FInstancedStruct& Value);

	/* Voxel -> Heart runtime data struct conversion */
	FBloodValue VoxelPinToBlood(const FVoxelPinValue& Value);

	/* Heart -> Voxel runtime data struct conversion */
	FVoxelPinValue BloodToVoxelPin(const FBloodValue& Value, const FVoxelPinType& ExpectedType);
}