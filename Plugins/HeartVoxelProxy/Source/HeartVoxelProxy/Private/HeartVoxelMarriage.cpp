// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "HeartVoxelMarriage.h"

#include "Model/HeartNodeEdit.h"

#include "Proxy/VoxelProxyConverters.h"
#include "Proxy/VoxelProxyDummy.h"
#include "Proxy/VoxelProxyGraph.h"
#include "Proxy/VoxelProxyNode.h"

#include "VoxelActor.h"
#include "VoxelGraph.h"
#include "VoxelTerminalGraph.h"
#include "VoxelTerminalGraphRuntime.h"

void UHeartVoxelMarriage::GenerateProxyFromVoxelGraph(UVoxelGraph* Graph)
{
	RuntimeGraph = DuplicateObject(Graph, this);
	ProxyGraph = NewObject<UVoxelProxyGraph>(this);
	ProxyGraph->GetOnPinDefaultValueChanged().AddUObject(this, &ThisClass::RestartVoxelGraph);

	const UVoxelTerminalGraph& Terminal = RuntimeGraph->GetMainTerminalGraph();
	const UVoxelTerminalGraphRuntime& Runtime = Terminal.GetRuntime();
	const FVoxelSerializedGraph& SerializedGraph = Runtime.GetSerializedGraph();
	const FVoxelTerminalGraphRef TerminalGraphRef(RuntimeGraph, GVoxelMainTerminalGraphGuid);

	TMap<FName, FHeartNodeGuid> VoxelIDToHeartNodes;

	// Node creation scope
	{
		Heart::API::FNodeEdit NodeEdit(ProxyGraph);

		for (auto&& NameAndSerializedNode : SerializedGraph.NodeNameToNode)
		{
			const FVoxelSerializedNode& SerializedNode = NameAndSerializedNode.Value;

			NodeEdit.Create_Instanced(
				UVoxelProxyNode::StaticClass(),
				UVoxelProxyDummy::StaticClass(),
				FVector2D::ZeroVector);
			UVoxelProxyNode* NewProxy = Cast<UVoxelProxyNode>(NodeEdit.Get());
			check(NewProxy);

			VoxelIDToHeartNodes.Add(NameAndSerializedNode.Key, NewProxy->GetGuid());

			NewProxy->ProxiedNodeRef = FVoxelGraphNodeRef
			{
				TerminalGraphRef,
				SerializedNode.GetNodeId(),
				SerializedNode.EdGraphNodeTitle,
				SerializedNode.EdGraphNodeName
			};

			for (auto&& InputPin : SerializedNode.InputPins)
			{
				NewProxy->AddPin(Converters::VoxelPinToHeartPin(ProxyGraph, InputPin.Value, EHeartPinDirection::Input));
			}

			for (auto&& OutputPin : SerializedNode.OutputPins)
			{
				NewProxy->AddPin(Converters::VoxelPinToHeartPin(ProxyGraph, OutputPin.Value, EHeartPinDirection::Output));
			}

			NewProxy->CleanedName = FText::FromName(SerializedNode.EdGraphNodeTitle);

			// Parse the NodeID for the node color.
			const FString NodeIDStr = SerializedNode.GetNodeId().ToString();

			// Voxel Nodes
			if (NodeIDStr.StartsWith(TEXTVIEW("VoxelNode_")))
			{
				NewProxy->NodeColor = EVoxelProxyNodeColor::Blue;
			}
			// Voxel Template Nodes and Library functions
			else if (NodeIDStr.StartsWith(TEXTVIEW("VoxelTemplateNode_")) ||
					 NodeIDStr.Contains(TEXTVIEW("Library.")))
			{
				NewProxy->NodeColor = EVoxelProxyNodeColor::Green;
			}
			// Voxel Exec Nodes
			else if (NodeIDStr.StartsWith(TEXTVIEW("VoxelExecNode_")))
			{
				if (NodeIDStr.Contains(TEXTVIEW("CallGraph")))
				{
					NewProxy->NodeColor = EVoxelProxyNodeColor::Orange;
				}
				else
				{
					NewProxy->NodeColor = EVoxelProxyNodeColor::Red;
				}
			}
		}
	}

	// After nodes are created, scrape pin connections
	{
		Heart::Connections::FEdit PinEdit = ProxyGraph->EditConnections();

		for (auto&& NameAndSerializedNode : SerializedGraph.NodeNameToNode)
		{
			FHeartNodeGuid NodeGuid = VoxelIDToHeartNodes[NameAndSerializedNode.Key];
			UHeartGraphNode* GraphNode = ProxyGraph->GetNode(NodeGuid);

			for (auto&& OutputPin : NameAndSerializedNode.Value.OutputPins)
			{
				FHeartGraphPinReference FromPin;
				FromPin.NodeGuid = NodeGuid;
				FromPin.PinGuid = GraphNode->GetPinByName(OutputPin.Key);

				for (auto&& LinkedTo : OutputPin.Value.LinkedTo)
				{
					FHeartNodeGuid ToNodeGuid = VoxelIDToHeartNodes[LinkedTo.NodeName];
					UHeartGraphNode* ToGraphNode = ProxyGraph->GetNode(ToNodeGuid);

					FHeartGraphPinReference ToPin;
					ToPin.NodeGuid = ToNodeGuid;
					ToPin.PinGuid = ToGraphNode->GetPinByName(LinkedTo.PinName);

					PinEdit.Connect(FromPin, ToPin);
				}
			}
		}
	}

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