// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "VoxelGraphNodeRef.h"
#include "VoxelPinType.h"
#include "General/ObjectTree.h"
#include "Model/HeartGraph.h"
#include "VoxelProxyGraph.generated.h"

namespace Voxel::Graph
{
	class FGraph;
	class FNode;
}

struct FVoxelSerializedGraph;
class UVoxelProxyNode;
struct FBloodValue;
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

public:
	UVoxelProxyGraph();

protected:
	/* Propagate Heart Events to Voxel */
	virtual void HandleNodeRemoveEvent(const FHeartNodeRemoveEvent& Event) override;

	/* Voxel Graphs don't store their positions at runtime, so we don't have anything to handle here */
	//virtual void HandleNodeMoveEvent(const FHeartNodeMoveEvent& Event) override;

	virtual void HandleGraphConnectionEvent(const FHeartGraphConnectionEvent& Event) override;

	void SyncNodeRemoval(const UVoxelProxyNode* Node);

	/* Push Heart Pin connections to Voxel Graph */
	void SyncPinConnections(const UVoxelProxyNode* Node, const FHeartPinGuid& Pin);

	void NotifyVoxelGraphEdited(EHVPEditType Type);

public:
	const FVoxelSerializedGraph* GetSerializedGraph() const;
	FVoxelSerializedGraph* GetSerializedGraph();

	const Voxel::Graph::FGraph* GetCompiledGraph() const;
	Voxel::Graph::FGraph* GetCompiledGraph();

	Voxel::Graph::FNode* GetCompiledGraphNode(FName NodeID);

	FVoxelTerminalGraphRef GetRef() const { return TerminalGraphRef; }

	void SetInitialized(const FVoxelTerminalGraphRef& GraphRef);

	FHVPEvent::RegistrationType& GetOnVoxelSerializedGraphEdited() { return OnVoxelSerializedGraphEdited; }
	FHVPEvent::RegistrationType& GetOnVoxelCompiledGraphEdited() { return OnVoxelCompiledGraphEdited; }

	UHeartVoxelPinTypeWrapper* GetTypeMetadata(const FVoxelPinType& Type);

	UFUNCTION(BlueprintCallable, Category = "VoxelProxyGraph")
	TArray<FName> GetParameterNames() const;

	UFUNCTION(BlueprintCallable, Category = "VoxelProxyGraph")
	UHeartVoxelPinTypeWrapper* GetParameterPinType(FName Name);

	UFUNCTION(BlueprintCallable, Category = "VoxelProxyGraph")
	FBloodValue GetParameterValue(FName Name) const;

	UFUNCTION(BlueprintCallable, Category = "VoxelProxyGraph")
	void SetParameterValue(FName Name, const FBloodValue& Value);

	UFUNCTION(BlueprintCallable, Category = "VoxelProxyGraph")
	FHeartNodeGuid AddNodeToGraph(UObject* VoxelNodeSource);

	UFUNCTION(BlueprintCallable, Category = "VoxelProxy|PinValue")
	FBloodValue GetPinDefaultValue(FHeartNodeGuid NodeGuid, FName Pin) const;

	UFUNCTION(BlueprintCallable, Category = "HeartVoxel|Editing")
	void SetPinDefaultValue(FHeartNodeGuid NodeGuid, FName Pin, const FBloodValue& Value);

	UFUNCTION(BlueprintCallable, Category = "VoxelProxyGraph")
	UHeartObjectTree* GetAssetPicker() const { return { AssetPickerOptions }; }

	UFUNCTION(BlueprintCallable, Category = "VoxelProxyGraph")
	UHeartObjectTree* GetClassPicker() const { return { ClassPickerOptions }; }


protected:
	FHVPEvent OnVoxelSerializedGraphEdited;
	FHVPEvent OnVoxelCompiledGraphEdited;

	UPROPERTY()
	FVoxelTerminalGraphRef TerminalGraphRef;

	// Cached PinTypeWrappers to reuse across pins with the same type.
	UPROPERTY()
	TMap<FVoxelPinType, TObjectPtr<UHeartVoxelPinTypeWrapper>> TypeMetadata;

	UPROPERTY()
	TObjectPtr<UHeartObjectTree> AssetPickerOptions;

	UPROPERTY()
	TObjectPtr<UHeartObjectTree> ClassPickerOptions;

	// Has this proxy finished initial sync with Voxel Graph
	bool Initialized = false;
};