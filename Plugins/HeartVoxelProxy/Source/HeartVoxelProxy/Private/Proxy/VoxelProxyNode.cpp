// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Proxy/VoxelProxyNode.h"
#include "Proxy/VoxelProxyGraph.h"
#include "Proxy/VoxelProxyConverters.h"

#include "VoxelCompilationGraph.h"
#include "VoxelGraph.h"
#include "VoxelTerminalGraph.h"
#include "VoxelTerminalGraphRuntime.h"

#include "access/access.h"

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

using FCachedCompiledGraphType = TOptional<TSharedPtr<const Voxel::Graph::FGraph>>;
ACCESS_CREATE_TAG(TheCompiledGraph, UVoxelTerminalGraphRuntime, CachedCompiledGraph);

void UVoxelProxyNode::SetPinDefaultValue(const FName Pin, const FBloodValue& Value)
{
	const UVoxelTerminalGraph& Terminal = ProxiedNodeRef.TerminalGraphRef.Graph->GetMainTerminalGraph();
	UVoxelTerminalGraphRuntime& Runtime = Terminal.GetRuntime();

	// Set value on Serialized Graph
	FVoxelSerializedGraph& SerializedGraph = const_cast<FVoxelSerializedGraph&>(Runtime.GetSerializedGraph());
	if (auto&& SerializedPin = SerializedGraph.NodeNameToNode[ProxiedNodeRef.EdGraphNodeName].InputPins.Find(Pin))
	{
		SerializedPin->DefaultValue = Converters::BloodToVoxelPin(Value, SerializedPin->Type);
	}

	// Set value on Compiled Graph
	FCachedCompiledGraphType CompiledGraph = access::get<TheCompiledGraph>(Runtime);
	if (CompiledGraph.IsSet() && CompiledGraph.GetValue().IsValid())
	{
		const Voxel::Graph::FGraph* GraphPtr = CompiledGraph.GetValue().Get();
		for (const Voxel::Graph::FNode& NodeAddr : GraphPtr->GetNodes())
		{
			if (NodeAddr.NodeRef != ProxiedNodeRef) continue;

			Voxel::Graph::FPin* PinPtr = const_cast<Voxel::Graph::FNode&>(NodeAddr).FindPin(Pin);
			if (!PinPtr) return;

			PinPtr->SetDefaultValue(Converters::BloodToVoxelPin(Value, PinPtr->Type));
		}
	}

	GetOwningGraph<UVoxelProxyGraph>()->NotifyPinDefaultValueChanged();
}

FBloodValue UVoxelProxyNode::GetPinDefaultValue(const FName Pin) const
{
	const UVoxelTerminalGraph& Terminal = ProxiedNodeRef.TerminalGraphRef.Graph->GetMainTerminalGraph();
	const UVoxelTerminalGraphRuntime& Runtime = Terminal.GetRuntime();
	const FVoxelSerializedGraph& SerializedGraph = Runtime.GetSerializedGraph();
	if (auto&& SerializedPin = SerializedGraph.NodeNameToNode[ProxiedNodeRef.EdGraphNodeName].InputPins.Find(Pin))
	{
		return Converters::VoxelPinToBlood(SerializedPin->DefaultValue);
	}

	return FBloodValue();
}