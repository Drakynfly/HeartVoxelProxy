// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Proxy/VoxelProxyNode.h"
#include "Proxy/VoxelProxyGraph.h"
#include "Proxy/VoxelProxyConverters.h"

namespace NodeColors
{
	// Copied from FVoxelGraphVisuals::GetNodeColor
	static constexpr FLinearColor Blue(FLinearColor(0.f, 0.68359375f, 1.f));
	static constexpr FLinearColor Green(FLinearColor(0.039216f, 0.666667f, 0.f));
	static constexpr FLinearColor Red(FLinearColor(1.f, 0.f, 0.f));
	static constexpr FLinearColor Orange(FLinearColor(1.f, 0.546875f, 0.f));
	static constexpr FLinearColor White(FLinearColor(1.f, 1.f, 1.f, 1.f));
}

FText UVoxelProxyNode::GetNodeTitle_Implementation(const UObject* Node) const
{
	if (!CleanedName.IsEmpty())
	{
		return CleanedName;
	}
	return FText::FromName(ProxiedNodeRef.NodeId);
}

FLinearColor UVoxelProxyNode::GetNodeTitleColor_Implementation(const UObject* Node) const
{
	switch (NodeColor)
	{
	case EVoxelProxyNodeColor::Blue:	return NodeColors::Blue;
	case EVoxelProxyNodeColor::Green:	return NodeColors::Green;
	case EVoxelProxyNodeColor::Red:		return NodeColors::Red;
	case EVoxelProxyNodeColor::Orange:	return NodeColors::Orange;
	case EVoxelProxyNodeColor::White:
	default:
		return NodeColors::White;
	}
}

void UVoxelProxyNode::SetPinDefaultValue(const FName Pin, const FBloodValue& Value)
{
	GetOwningGraph<UVoxelProxyGraph>()->SetPinDefaultValue(Guid, Pin, Value);
}

FBloodValue UVoxelProxyNode::GetPinDefaultValue(const FName Pin) const
{
	return GetOwningGraph<UVoxelProxyGraph>()->GetPinDefaultValue(Guid, Pin);
}