// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Proxy/VoxelProxyGraph.h"
#include "Proxy/VoxelProxyPin.h"

void UVoxelProxyGraph::NotifyPinDefaultValueChanged()
{
	OnPinDefaultValueChanged.Broadcast();
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