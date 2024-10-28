// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Proxy/VoxelProxyGraph.h"
#include "VoxelCompilationGraph.h"
#include "Proxy/VoxelProxyConverters.h"
#include "Proxy/VoxelProxyNode.h"
#include "Proxy/VoxelProxyPin.h"

#include "Model/HeartGraphNode.h"

#include "VoxelGraph.h"
#include "VoxelGraphParametersView.h"
#include "VoxelParameterView.h"
#include "VoxelTerminalGraph.h"
#include "VoxelTerminalGraphRuntime.h"
#include "General/ObjectTree.h"

#include "access/access.h"

using FCachedCompiledGraphType = TOptional<TSharedPtr<const Voxel::Graph::FGraph>>;

ACCESS_CREATE_TAG(FCachedCompiledGraph, UVoxelTerminalGraphRuntime, CachedCompiledGraph);

namespace Voxel::Graph
{
	ACCESS_CREATE_TAG(FLinkedTo, FPin, LinkedTo);
}

UVoxelProxyGraph::UVoxelProxyGraph()
{
	AssetPickerOptions = CreateDefaultSubobject<UHeartObjectTree>(TEXT("AssetPickerOptions"));
	ClassPickerOptions = CreateDefaultSubobject<UHeartObjectTree>(TEXT("ClassPickerOptions"));
}

void UVoxelProxyGraph::HandleNodeRemoveEvent(const FHeartNodeRemoveEvent& Event)
{
	Super::HandleNodeRemoveEvent(Event);

	for (auto&& AffectedNode : Event.AffectedNodes)
	{
		if (const UVoxelProxyNode* ProxyNode = Cast<UVoxelProxyNode>(AffectedNode))
		{
			SyncNodeRemoval(ProxyNode);
		}
	}
}

void UVoxelProxyGraph::HandleGraphConnectionEvent(const FHeartGraphConnectionEvent& Event)
{
	Super::HandleGraphConnectionEvent(Event);

	if (!Initialized) return;

	for (auto&& Node : Event.AffectedNodes)
	{
		const UVoxelProxyNode* ProxyNode = Cast<UVoxelProxyNode>(Node);
		if (!IsValid(ProxyNode))
		{
			continue;
		}

		for (auto&& Pin : Event.AffectedPins)
		{
			if (ProxyNode->IsPinOnNode(Pin))
			{
				SyncPinConnections(ProxyNode, Pin);
			}
		}
	}
}

void UVoxelProxyGraph::SyncNodeRemoval(const UVoxelProxyNode* Node)
{
	const UVoxelTerminalGraph& Terminal = TerminalGraphRef.Graph->GetMainTerminalGraph();
	const UVoxelTerminalGraphRuntime& Runtime = Terminal.GetRuntime();

	EHVPEditType Type = EHVPEditType::None;

	// Sync the Serialized Graph
	FVoxelSerializedGraph& SerializedGraph = ConstCast(Runtime.GetSerializedGraph());
	FVoxelSerializedNode& SerializedNode = SerializedGraph.NodeNameToNode[Node->ProxiedNodeRef.EdGraphNodeName];

	for (auto&& InputPin : SerializedNode.InputPins)
	{
		for (auto&& LinkedRef : InputPin.Value.LinkedTo)
		{
			FVoxelSerializedNode& LinkedNode = SerializedGraph.NodeNameToNode[LinkedRef.NodeName];
			auto& OtherLinks = LinkedNode.OutputPins[LinkedRef.PinName].LinkedTo;
			for (int32 i = 0; i < OtherLinks.Num(); ++i)
			{
				if (OtherLinks[i].PinName == InputPin.Key)
				{
					OtherLinks.RemoveAt(i);
					break;
				}
			}
		}
	}

	for (auto&& OutputPin : SerializedNode.OutputPins)
	{
		for (auto&& LinkedRef : OutputPin.Value.LinkedTo)
		{
			FVoxelSerializedNode& LinkedNode = SerializedGraph.NodeNameToNode[LinkedRef.NodeName];
			auto& OtherLinks = LinkedNode.InputPins[LinkedRef.PinName].LinkedTo;
			for (int32 i = 0; i < OtherLinks.Num(); ++i)
			{
				if (OtherLinks[i].PinName == OutputPin.Key)
				{
					OtherLinks.RemoveAt(i);
					break;
				}
			}
		}
	}

	SerializedGraph.NodeNameToNode.Remove(Node->ProxiedNodeRef.EdGraphNodeName);
	Type |= EHVPEditType::SerializedGraph;

	// Sync the Compiled Graph
	FCachedCompiledGraphType CompiledGraph = access::get<FCachedCompiledGraph>(Runtime);
	if (CompiledGraph.IsSet() && CompiledGraph.GetValue().IsValid())
	{
		Voxel::Graph::FGraph* GraphPtr = ConstCast(CompiledGraph.GetValue().Get());
		for (Voxel::Graph::FNode* NodePtr : GraphPtr->GetNodesArray())
		{
			if (NodePtr->NodeRef.NodeId != Node->ProxiedNodeRef.NodeId) continue;

			GraphPtr->RemoveNode(*NodePtr);
			Type |= EHVPEditType::CompiledGraph;
		}
	}

	if (Type != None)
	{
		NotifyVoxelGraphEdited(Type);
	}
}

enum ESyncType
{
	NoLinks,
	HeartHasLinks = 1,
	VoxelHasLinks = 2,
	BothHaveLinks = HeartHasLinks | VoxelHasLinks
};
ENUM_CLASS_FLAGS(ESyncType)

void SetDisconnectedPinDefaultValue(Voxel::Graph::FPin& Pin)
{
	if (!Pin.Type.HasPinDefaultValue() ||
		Pin.Type.IsBuffer() ||
		Pin.Type.IsBufferArray())
	{
		return;
	}

	// @todo can we get the default value from the SerializedGraph instead?
	const FVoxelPinValue DefaultValue = FVoxelPinValue(Pin.Type);
	Pin.SetDefaultValue(DefaultValue);
}

void UVoxelProxyGraph::SyncPinConnections(const UVoxelProxyNode* Node, const FHeartPinGuid& Pin)
{
	const UVoxelTerminalGraph& Terminal = TerminalGraphRef.Graph->GetMainTerminalGraph();
	const UVoxelTerminalGraphRuntime& Runtime = Terminal.GetRuntime();

	EHVPEditType Type = EHVPEditType::None;

	// Get pin name
	const auto SelfDesc = Node->GetPinDesc(Pin);

	ESyncType SyncType = NoLinks;
	SyncType |= Node->HasConnections(Pin) ? HeartHasLinks : NoLinks;

	// Sync the Serialized Graph
	FVoxelSerializedGraph& SerializedGraph = ConstCast(Runtime.GetSerializedGraph());
	FVoxelSerializedNode& SerializedNode = SerializedGraph.NodeNameToNode[Node->ProxiedNodeRef.EdGraphNodeName];
	if (FVoxelSerializedPin* SerializedPin = SerializedNode.InputPins.Find(SelfDesc->Name))
	{
		SyncType |= !SerializedPin->LinkedTo.IsEmpty() ? VoxelHasLinks : NoLinks;

		switch (SyncType)
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
				const FHeartGraphPinConnections HeartConnections = Node->GetConnections(Pin).GetValue();
				for (const FHeartGraphPinReference& Connection : HeartConnections)
				{
					const UVoxelProxyNode* ProxyNode = GetNode<UVoxelProxyNode>(Connection.NodeGuid);

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
	FCachedCompiledGraphType CompiledGraph = access::get<FCachedCompiledGraph>(Runtime);
	if (CompiledGraph.IsSet() && CompiledGraph.GetValue().IsValid())
	{
		Voxel::Graph::FGraph* GraphPtr = ConstCast(CompiledGraph.GetValue().Get());
		for (Voxel::Graph::FNode* NodePtr : GraphPtr->GetNodesArray())
		{
			if (NodePtr->NodeRef.NodeId != Node->ProxiedNodeRef.NodeId) continue;

			Voxel::Graph::FPin* PinPtr = NodePtr->FindPin(SelfDesc->Name);
			if (!PinPtr) return;
			Voxel::Graph::FPin& VoxelPin = *PinPtr;

			// Clear this flag from when we set it previously, and reset with new value
			EnumRemoveFlags(SyncType, VoxelHasLinks);
			SyncType |= !access::get<Voxel::Graph::FLinkedTo>(VoxelPin).IsEmpty() ? VoxelHasLinks : NoLinks;

			switch (SyncType)
			{
			case NoLinks:
				// All synced!
				break;
			case VoxelHasLinks:
				// There are no connections on the heart side, remove all voxel links.
				{
					bool PinIsInput = VoxelPin.Direction == Voxel::Graph::EPinDirection::Input;

					// When removing all pin connections from an output we have to fill in a default value for links that will have no connections.
					if (!PinIsInput)
					{
						auto&& Links = VoxelPin.GetLinkedTo();
						for (auto&& Link : Links)
						{
							if (Link.GetLinkedTo().Num() == 1)
							{
								SetDisconnectedPinDefaultValue(Link);
							}
						}
					}

					VoxelPin.BreakAllLinks();

					// When removing all pin connections from an input we have to fill in a default value.
					if (PinIsInput)
					{
						SetDisconnectedPinDefaultValue(VoxelPin);
					}

					Type |= EHVPEditType::CompiledGraph;
				}
				break;
			case BothHaveLinks:
				// Reset Voxel links, then fall into the HeartHasLinks path...
				VoxelPin.BreakAllLinks();
			case HeartHasLinks:
				// There are no connections on the voxel side, add all heart links.
				{
					const FHeartGraphPinConnections HeartConnections = Node->GetConnections(Pin).GetValue();
					for (const FHeartGraphPinReference& Connection : HeartConnections)
					{
						const UVoxelProxyNode* ProxyNode = GetNode<UVoxelProxyNode>(Connection.NodeGuid);
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
		NotifyVoxelGraphEdited(Type);
	}
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

void UVoxelProxyGraph::SetInitialized(const FVoxelTerminalGraphRef& GraphRef)
{
	check(!Initialized);
	TerminalGraphRef = GraphRef;
	Initialized = true;
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

TArray<FName> UVoxelProxyGraph::GetParameterNames() const
{
	const UVoxelGraph* Graph = TerminalGraphRef.Graph.Get();
	const TSharedPtr<FVoxelGraphParametersView> ParametersView = Graph->GetParametersView();
	if (!ParametersView)
	{
		return {};
	}

	TArray<FName> Names;
	Names.Reserve(ParametersView->GetChildren().Num());
	for (const FVoxelParameterView* ParameterView : ParametersView->GetChildren())
	{
		Names.Add(ParameterView->GetName());
	}
	return Names;
}

UHeartVoxelPinTypeWrapper* UVoxelProxyGraph::GetParameterPinType(const FName Name)
{
	const UVoxelGraph* Graph = TerminalGraphRef.Graph.Get();
	if (!Graph)
	{
		return nullptr;
	}
	const TSharedPtr<FVoxelGraphParametersView> ParametersView = Graph->GetParametersView();
	if (!ParametersView)
	{
		return nullptr;
	}

	if (const FVoxelParameterView* ParameterView = ParametersView->FindByName(Name))
	{
		return GetTypeMetadata(ParameterView->GetType());
	}
	return nullptr;
}

FBloodValue UVoxelProxyGraph::GetParameterValue(const FName Name) const
{
	const UVoxelGraph* Graph = TerminalGraphRef.Graph.Get();
	if (!Graph)
	{
		return FBloodValue();
	}
	return Converters::VoxelPinToBlood(Graph->GetParameter(Name));
}

void UVoxelProxyGraph::SetParameterValue(const FName Name, const FBloodValue& Value)
{
	UVoxelGraph* Graph = ConstCast(TerminalGraphRef.Graph.Get());
	if (!Graph)
	{
		return;
	}
	auto PinTypeWrapper = GetParameterPinType(Name);
	if (!PinTypeWrapper)
	{
		return;
	}
	Graph->SetParameter(Name, Converters::BloodToVoxelPin(Value, PinTypeWrapper->PinType));
}

FBloodValue UVoxelProxyGraph::GetPinDefaultValue(const FHeartNodeGuid NodeGuid, const FName Pin) const
{
	const UVoxelTerminalGraph& Terminal = TerminalGraphRef.Graph->GetMainTerminalGraph();
	const UVoxelTerminalGraphRuntime& Runtime = Terminal.GetRuntime();
	const FVoxelSerializedGraph& SerializedGraph = Runtime.GetSerializedGraph();
	if (const UVoxelProxyNode* Node = GetNode<UVoxelProxyNode>(NodeGuid))
	{
		if (auto&& SerializedPin = SerializedGraph.NodeNameToNode[Node->ProxiedNodeRef.EdGraphNodeName].InputPins.Find(Pin))
		{
			return Converters::VoxelPinToBlood(SerializedPin->DefaultValue);
		}
	}

	return FBloodValue();
}

void UVoxelProxyGraph::SetPinDefaultValue(FHeartNodeGuid NodeGuid, FName Pin, const FBloodValue& Value)
{
	const UVoxelTerminalGraph& Terminal = TerminalGraphRef.Graph->GetMainTerminalGraph();
	UVoxelTerminalGraphRuntime& Runtime = Terminal.GetRuntime();

	EHVPEditType Type = EHVPEditType::None;

	const UVoxelProxyNode* Node = GetNode<UVoxelProxyNode>(NodeGuid);

	// Set value on Serialized Graph
	FVoxelSerializedGraph& SerializedGraph = ConstCast(Runtime.GetSerializedGraph());
	FVoxelSerializedNode& SerializedNode = SerializedGraph.NodeNameToNode[Node->ProxiedNodeRef.EdGraphNodeName];
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
	FCachedCompiledGraphType CompiledGraph = access::get<FCachedCompiledGraph>(Runtime);
	if (CompiledGraph.IsSet() && CompiledGraph.GetValue().IsValid())
	{
		Voxel::Graph::FGraph* GraphPtr = ConstCast(CompiledGraph.GetValue().Get());
		for (Voxel::Graph::FNode* NodePtr : GraphPtr->GetNodesArray())
		{
			if (NodePtr->NodeRef.NodeId != Node->ProxiedNodeRef.NodeId) continue;

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
		NotifyVoxelGraphEdited(Type);
	}
}