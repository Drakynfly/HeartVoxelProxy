// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "VoxelPinType.h"
#include "Model/HeartGraph.h"
#include "VoxelProxyGraph.generated.h"

class UHeartVoxelPinTypeWrapper;

enum EHVPEditType
{
	None = 0,
	SerializedGraph = 1 << 0,
	CompiledGraph = 1 << 1
};
ENUM_CLASS_FLAGS(EHVPEditType)

using FHVPEvent = TMulticastDelegate<void()>;

/**
 * A Heart Graph that acts as an intermediate to display a voxel graph.
 */
UCLASS()
class HEARTVOXELPROXY_API UVoxelProxyGraph : public UHeartGraph
{
	GENERATED_BODY()

protected:
	/* Voxel Graphs don't store their positions at runtime, so we don't have anything to handle here */
	//virtual void HandleNodeMoveEvent(const FHeartNodeMoveEvent& Event) override;

	/* Propagate Heart Connections to Voxel */
	virtual void HandleGraphConnectionEvent(const FHeartGraphConnectionEvent& Event) override;

public:
	void SetInitialized();

	FHVPEvent::RegistrationType& GetOnVoxelSerializedGraphEdited() { return OnVoxelSerializedGraphEdited; }
	FHVPEvent::RegistrationType& GetOnVoxelCompiledGraphEdited() { return OnVoxelCompiledGraphEdited; }

	void NotifyVoxelGraphEdited(EHVPEditType Type);

	UHeartVoxelPinTypeWrapper* GetTypeMetadata(const FVoxelPinType& Type);

protected:
	FHVPEvent OnVoxelSerializedGraphEdited;
	FHVPEvent OnVoxelCompiledGraphEdited;

	// Cached PinTypeWrappers to reuse across pins with the same type.
	UPROPERTY()
	TMap<FVoxelPinType, TObjectPtr<UHeartVoxelPinTypeWrapper>> TypeMetadata;

	// Has this proxy finished initial sync with Voxel Graph
	bool Initialized = false;
};