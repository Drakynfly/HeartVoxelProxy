// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "VoxelSerializedGraph.h"
#include "Model/HeartGraph.h"
#include "VoxelProxyGraph.generated.h"

class UHeartVoxelPinTypeWrapper;

using FHVPEvent = TMulticastDelegate<void()>;

/**
 * A Heart Graph that acts as an intermediate to display a voxel graph.
 */
UCLASS()
class HEARTVOXELPROXY_API UVoxelProxyGraph : public UHeartGraph
{
	GENERATED_BODY()

public:
	FHVPEvent::RegistrationType& GetOnPinDefaultValueChanged() { return OnPinDefaultValueChanged; }

	void NotifyPinDefaultValueChanged();

	UHeartVoxelPinTypeWrapper* GetTypeMetadata(const FVoxelPinType& Type);

protected:
	FHVPEvent OnPinDefaultValueChanged;

	UPROPERTY()
	TMap<FVoxelPinType, TObjectPtr<UHeartVoxelPinTypeWrapper>> TypeMetadata;
};