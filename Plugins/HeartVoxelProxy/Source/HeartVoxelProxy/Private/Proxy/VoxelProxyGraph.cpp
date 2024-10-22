// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Proxy/VoxelProxyGraph.h"
#include "Model/HeartGraphNode.h"
#include "Proxy/VoxelProxyNode.h"
#include "Proxy/VoxelProxyPin.h"

void UVoxelProxyGraph::HandleGraphConnectionEvent(const FHeartGraphConnectionEvent& Event)
{
	Super::HandleGraphConnectionEvent(Event);

	if (!Initialized) return;

	for (auto&& Node : Event.AffectedNodes)
	{
		UVoxelProxyNode* ProxyNode = Cast<UVoxelProxyNode>(Node);
		if (!IsValid(ProxyNode))
		{
			continue;
		}

		for (auto&& Pin : Event.AffectedPins)
		{
			if (ProxyNode->IsPinOnNode(Pin))
			{
				ProxyNode->SyncPinConnections(Pin);
			}
		}
	}
}

void UVoxelProxyGraph::SetInitialized()
{
	check(!Initialized);
	Initialized = true;
}

void UVoxelProxyGraph::NotifyVoxelGraphEdited(const EHVPEditType Type)
{
	if (EnumHasAnyFlags(Type, EHVPEditType::SerializedGraph))
	{
		OnVoxelSerializedGraphEdited.Broadcast();
	}
	if (EnumHasAnyFlags(Type, EHVPEditType::CompiledGraph))
	{
		OnVoxelCompiledGraphEdited.Broadcast();
	}
}

UHeartVoxelPinTypeWrapper* UVoxelProxyGraph::GetTypeMetadata(const FVoxelPinType& Type)
{
	if (auto Existing = TypeMetadata.Find(Type))
	{
		return *Existing;
	}

	UHeartVoxelPinTypeWrapper* NewMetadata = NewObject<UHeartVoxelPinTypeWrapper>(this);
	NewMetadata->PinType = Type;
	TypeMetadata.Add(Type, NewMetadata);
	return NewMetadata;
}