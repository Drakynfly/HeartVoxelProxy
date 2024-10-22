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

using FCachedCompiledGraphType = TOptional<TSharedPtr<const Voxel::Graph::FGraph>>;

ACCESS_CREATE_TAG(TheCompiledGraph, UVoxelTerminalGraphRuntime, CachedCompiledGraph);

namespace Voxel::Graph
{
	ACCESS_CREATE_TAG(FPin_LinkedTo, FPin, LinkedTo);
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

enum ELinkCombo
{
	NoLinks,
	HeartHasLinks = 1,
	VoxelHasLinks = 2,
	BothHaveLinks = HeartHasLinks | VoxelHasLinks
};
ENUM_CLASS_FLAGS(ELinkCombo)

void UVoxelProxyNode::SyncPinConnections(const FHeartPinGuid& Pin)
{
	const UVoxelTerminalGraph& Terminal = ProxiedNodeRef.TerminalGraphRef.Graph->GetMainTerminalGraph();
	const UVoxelTerminalGraphRuntime& Runtime = Terminal.GetRuntime();

	EHVPEditType Type = EHVPEditType::None;

	// Get pin name
	const auto SelfDesc = GetPinDesc(Pin);

	ELinkCombo Links = NoLinks;
	Links |= HasConnections(Pin) ? HeartHasLinks : NoLinks;

	// Sync the Serialized Graph
	FVoxelSerializedGraph& SerializedGraph = ConstCast(Runtime.GetSerializedGraph());
	FVoxelSerializedNode& SerializedNode = SerializedGraph.NodeNameToNode[ProxiedNodeRef.EdGraphNodeName];
	if (FVoxelSerializedPin* SerializedPin = SerializedNode.InputPins.Find(SelfDesc->Name))
	{
		Links |= !SerializedPin->LinkedTo.IsEmpty() ? VoxelHasLinks : NoLinks;

		switch (Links)
		{
		case NoLinks:
			// All synced!
			break;
		case VoxelHasLinks:
			// There are no connections on the heart side, remove all voxel links.
			SerializedPin->LinkedTo.Reset();
			Type |= EHVPEditType::SerializedGraph;
			break;
		case BothHaveLinks:
			// Reset Voxel links, then fall into the HeartHasLinks path...
			SerializedPin->LinkedTo.Reset();
		case HeartHasLinks:
			// There are no connections on the voxel side, add all heart links.
			{
				const FHeartGraphPinConnections HeartConnections = GetConnections(Pin).GetValue();
				for (const FHeartGraphPinReference& Connection : HeartConnections)
				{
					const UVoxelProxyNode* ProxyNode = GetGraph()->GetNode<UVoxelProxyNode>(Connection.NodeGuid);

					FVoxelSerializedPinRef VoxelPinRef;

					VoxelPinRef.NodeName = ProxyNode->ProxiedNodeRef.EdGraphNodeName;
					VoxelPinRef.PinName = ProxyNode->GetPinDesc(Connection.PinGuid)->Name;

					// Link is an input if we are an output.
					VoxelPinRef.bIsInput = SelfDesc->Direction == EHeartPinDirection::Output;

					SerializedPin->LinkedTo.Add(VoxelPinRef);
				}
			}
			Type |= EHVPEditType::SerializedGraph;
			break;
		}
	}

	// Sync the Compiled Graph
	FCachedCompiledGraphType CompiledGraph = access::get<TheCompiledGraph>(Runtime);
	if (CompiledGraph.IsSet() && CompiledGraph.GetValue().IsValid())
	{
		Voxel::Graph::FGraph* GraphPtr = ConstCast(CompiledGraph.GetValue().Get());
		for (Voxel::Graph::FNode* NodePtr : GraphPtr->GetNodesArray())
		{
			if (NodePtr->NodeRef.NodeId != ProxiedNodeRef.NodeId) continue;

			Voxel::Graph::FPin* PinPtr = NodePtr->FindPin(SelfDesc->Name);
			if (!PinPtr) return;
			Voxel::Graph::FPin& VoxelPin = *PinPtr;

			// Clear this flag from when we set it previously, and reset with new value
			EnumRemoveFlags(Links, VoxelHasLinks);
			Links |= !access::get<Voxel::Graph::FPin_LinkedTo>(VoxelPin).IsEmpty() ? VoxelHasLinks : NoLinks;

			switch (Links)
			{
			case NoLinks:
				// All synced!
				break;
			case VoxelHasLinks:
				// There are no connections on the heart side, remove all voxel links.
				PinPtr->BreakAllLinks();
				Type |= EHVPEditType::CompiledGraph;
				break;
			case BothHaveLinks:
				// Reset Voxel links, then fall into the HeartHasLinks path...
				PinPtr->BreakAllLinks();
			case HeartHasLinks:
				// There are no connections on the voxel side, add all heart links.
				{
					const FHeartGraphPinConnections HeartConnections = GetConnections(Pin).GetValue();
					for (const FHeartGraphPinReference& Connection : HeartConnections)
					{
						const UVoxelProxyNode* ProxyNode = GetGraph()->GetNode<UVoxelProxyNode>(Connection.NodeGuid);
						const FName ProxyPin = ProxyNode->GetPinDesc(Connection.PinGuid)->Name;

						for (Voxel::Graph::FNode* NodePtrB : GraphPtr->GetNodesArray())
						{
							if (NodePtrB->NodeRef.NodeId != ProxyNode->ProxiedNodeRef.NodeId) continue;
							if (Voxel::Graph::FPin* PinPtrB = NodePtrB->FindPin(ProxyPin))
							{
								VoxelPin.MakeLinkTo(*PinPtrB);
							}
							break;
						}
					}
				}
				Type |= EHVPEditType::CompiledGraph;
				break;
			}
			break;
		}
	}

	if (Type != None)
	{
		GetOwningGraph<UVoxelProxyGraph>()->NotifyVoxelGraphEdited(Type);
	}
}

void UVoxelProxyNode::SetPinDefaultValue(const FName Pin, const FBloodValue& Value)
{
	const UVoxelTerminalGraph& Terminal = ProxiedNodeRef.TerminalGraphRef.Graph->GetMainTerminalGraph();
	UVoxelTerminalGraphRuntime& Runtime = Terminal.GetRuntime();

	EHVPEditType Type = EHVPEditType::None;

	// Set value on Serialized Graph
	FVoxelSerializedGraph& SerializedGraph = ConstCast(Runtime.GetSerializedGraph());
	FVoxelSerializedNode& SerializedNode = SerializedGraph.NodeNameToNode[ProxiedNodeRef.EdGraphNodeName];
	if (auto&& SerializedPin = SerializedNode.InputPins.Find(Pin))
	{
		// DefaultValue and PinType can differ (ExposedSeed vs. VoxelSeed)
		const FVoxelPinType ExpectedType = SerializedPin->DefaultValue.IsValid() ? SerializedPin->DefaultValue.GetType() : SerializedPin->Type;

		if (const FVoxelPinValue NewValue = Converters::BloodToVoxelPin(Value, ExpectedType);
			NewValue != SerializedPin->DefaultValue)
		{
			SerializedPin->DefaultValue = NewValue;
			Type |= EHVPEditType::SerializedGraph;
		}
	}

	// Set value on Compiled Graph
	FCachedCompiledGraphType CompiledGraph = access::get<TheCompiledGraph>(Runtime);
	if (CompiledGraph.IsSet() && CompiledGraph.GetValue().IsValid())
	{
		Voxel::Graph::FGraph* GraphPtr = ConstCast(CompiledGraph.GetValue().Get());
		for (Voxel::Graph::FNode* NodePtr : GraphPtr->GetNodesArray())
		{
			if (NodePtr->NodeRef.NodeId != ProxiedNodeRef.NodeId) continue;

			Voxel::Graph::FPin* PinPtr = NodePtr->FindPin(Pin);
			if (!PinPtr) return;

			// DefaultValue and PinType can differ (ExposedSeed vs. VoxelSeed)
			const FVoxelPinType ExpectedType = PinPtr->GetDefaultValue().IsValid() ? PinPtr->GetDefaultValue().GetType() : PinPtr->Type;

			if (const FVoxelPinValue NewValue = Converters::BloodToVoxelPin(Value, ExpectedType);
				NewValue != PinPtr->GetDefaultValue())
			{
				PinPtr->SetDefaultValue(NewValue);
				Type |= EHVPEditType::CompiledGraph;
			}
			break;
		}
	}

	if (Type != None)
	{
		GetOwningGraph<UVoxelProxyGraph>()->NotifyVoxelGraphEdited(Type);
	}
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