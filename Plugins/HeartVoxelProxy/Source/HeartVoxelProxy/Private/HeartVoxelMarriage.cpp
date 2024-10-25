// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "HeartVoxelMarriage.h"

#include "Proxy/VoxelProxyConverters.h"
#include "Proxy/VoxelProxyGraph.h"

#include "VoxelActor.h"
#include "VoxelGraph.h"

void UHeartVoxelMarriage::SetupWithBlank()
{
	RuntimeGraph = NewObject<UVoxelGraph>(this);
	ProxyGraph = NewObject<UVoxelProxyGraph>(this);
	const FVoxelTerminalGraphRef TerminalGraphRef(RuntimeGraph, GVoxelMainTerminalGraphGuid);
	ProxyGraph->SetInitialized(TerminalGraphRef);
	ProxyGraph->GetOnVoxelCompiledGraphEdited().AddUObject(this, &ThisClass::RestartVoxelGraph);

	SyncWithVoxelActor();
}

void UHeartVoxelMarriage::SetupWithExisting(UVoxelGraph* Graph)
{
	if (!ensure(IsValid(Graph)))
	{
		return;
	}

	// We do not want to edit existing voxel graphs, this messes up in PIE.
	if (!ensure(!Graph->IsAsset()))
	{
		return;
	}

	RuntimeGraph = Graph;
	ProxyGraph = Converters::CreateVoxelProxy(this, RuntimeGraph);
	ProxyGraph->GetOnVoxelCompiledGraphEdited().AddUObject(this, &ThisClass::RestartVoxelGraph);

	SyncWithVoxelActor();
}

void UHeartVoxelMarriage::SetupWithCopy(const UVoxelGraph* Graph)
{
	if (!ensure(IsValid(Graph)))
	{
		return;
	}

	RuntimeGraph = DuplicateObject(Graph, this);
	ProxyGraph = Converters::CreateVoxelProxy(this, RuntimeGraph);
	ProxyGraph->GetOnVoxelCompiledGraphEdited().AddUObject(this, &ThisClass::RestartVoxelGraph);

	SyncWithVoxelActor();
}

void UHeartVoxelMarriage::SetVoxelActor(AVoxelActor* Actor)
{
	VoxelActor = Actor;
	SyncWithVoxelActor();
}

void UHeartVoxelMarriage::SyncWithVoxelActor()
{
	if (IsValid(VoxelActor) && IsValid(RuntimeGraph))
	{
		VoxelActor->SetGraph(RuntimeGraph);
	}
}

void UHeartVoxelMarriage::RestartVoxelGraph()
{
	if (VoxelActor)
	{
		VoxelActor->QueueRecreate();
	}
}