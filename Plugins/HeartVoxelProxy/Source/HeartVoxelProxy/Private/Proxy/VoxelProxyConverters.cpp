﻿// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Proxy/VoxelProxyConverters.h"
#include "VoxelCompilationGraph.h"
#include "Proxy/VoxelProxyDummy.h"
#include "Proxy/VoxelProxyGraph.h"
#include "Proxy/VoxelProxyNode.h"
#include "Proxy/VoxelProxyPin.h"

#include "Model/HeartGraphNode.h"
#include "Model/HeartNodeEdit.h"

#include "VoxelExposedSeed.h"
#include "VoxelNode.h"
#include "VoxelSerializedGraph.h"
#include "VoxelPropertyType.h"
#include "VoxelTerminalGraph.h"
#include "VoxelTerminalGraphRuntime.h"
#include "Buffer/VoxelBaseBuffers.h"
#include "Channel/VoxelChannelName.h"


namespace Converters
{
	// Lifted from FVoxelGraphCompiler::LoadSerializedGraph
	bool LIFTED_LoadSerializedNode(UVoxelProxyGraph* ProxyGraph, const FVoxelSerializedNode& SerializedNode)
	{
		auto&& SerializedGraph = ProxyGraph->GetCompiledGraph();
		if (!SerializedGraph) return false;

		if (!SerializedNode.VoxelNode.IsValid())
		{
			for (const auto& It : SerializedNode.InputPins)
			{
				if (It.Value.LinkedTo.Num() > 0)
				{
					VOXEL_MESSAGE(Error, "Invalid struct on {0}", SerializedNode);
					return false;
				}
			}
			for (const auto& It : SerializedNode.OutputPins)
			{
				if (It.Value.LinkedTo.Num() > 0)
				{
					VOXEL_MESSAGE(Error, "Invalid struct on {0}", SerializedNode);
					return false;
				}
			}

			// This node isn't connected to anything, just skip it
			VOXEL_MESSAGE(Warning, "Invalid struct on {0}", SerializedNode);
			//SkippedNodes.Add(&SerializedNode);
			//continue;
			return false;
		}

		const FVoxelGraphNodeRef NodeRef
		{
			ProxyGraph->GetRef(),
			SerializedNode.GetNodeId(),
			SerializedNode.EdGraphNodeTitle,
			SerializedNode.EdGraphNodeName
		};

		Voxel::Graph::FNode& Node = SerializedGraph->NewNode(NodeRef);
		{
			VOXEL_SCOPE_COUNTER_FORMAT("Copy node %s", *SerializedNode.VoxelNode->GetStruct()->GetName());
			Node.SetVoxelNode(SerializedNode.VoxelNode->MakeSharedCopy());
		}

		for (const FString& Warning : SerializedNode.Warnings)
		{
			Node.AddError(Warning);
		}
		for (const FString& Error : SerializedNode.Errors)
		{
			Node.AddError(Error);
		}

		const FVoxelNode& VoxelNode = Node.GetVoxelNode();
		for (const FVoxelPin& Pin : VoxelNode.GetPins())
		{
			if (!(Pin.bIsInput ? SerializedNode.InputPins : SerializedNode.OutputPins).Contains(Pin.Name))
			{
				VOXEL_MESSAGE(Error, "Outdated node {0}: missing pin {1}", SerializedNode, Pin.Name);
				return false;
			}
		}
		for (const auto& It : SerializedNode.InputPins)
		{
			const FVoxelSerializedPin& SerializedPin = It.Value;
			if (!SerializedPin.ParentPinName.IsNone())
			{
				// Sub-pinS
				continue;
			}

			const TSharedPtr<const FVoxelPin> Pin = VoxelNode.FindPin(SerializedPin.PinName);
			if (!Pin ||
				!Pin->bIsInput)
			{
				VOXEL_MESSAGE(Error, "Outdated node {0}: unknown pin {1}", SerializedNode, SerializedPin.PinName);
				return false;
			}

			if (SerializedPin.Type != Pin->GetType())
			{
				VOXEL_MESSAGE(Error, "Outdated node {0}: type mismatch for pin {1}", SerializedNode, SerializedPin.PinName);
				return false;
			}
		}
		for (const auto& It : SerializedNode.OutputPins)
		{
			const FVoxelSerializedPin& SerializedPin = It.Value;
			if (!SerializedPin.ParentPinName.IsNone())
			{
				// Sub-pinS
				continue;
			}

			const TSharedPtr<const FVoxelPin> Pin = VoxelNode.FindPin(SerializedPin.PinName);
			if (!Pin ||
				Pin->bIsInput)
			{
				VOXEL_MESSAGE(Error, "Outdated node {0}: unknown pin {1}", SerializedNode, SerializedPin.PinName);
				return false;
			}

			if (SerializedPin.Type != Pin->GetType())
			{
				VOXEL_MESSAGE(Error, "Outdated node {0}: type mismatch for pin {1}", SerializedNode, SerializedPin.PinName);
				return false;
			}
		}

		for (const auto& It : SerializedNode.InputPins)
		{
			const FVoxelSerializedPin& SerializedPin = It.Value;
			if (!SerializedPin.Type.IsValid())
			{
				VOXEL_MESSAGE(Error, "Invalid pin {0}.{1}", SerializedNode, SerializedPin.PinName);
				return false;
			}

			FVoxelPinValue DefaultValue;
			if (SerializedPin.Type.HasPinDefaultValue())
			{
				DefaultValue = SerializedPin.DefaultValue;

				if (!DefaultValue.IsValid() ||
					!ensureVoxelSlow(DefaultValue.GetType().CanBeCastedTo(SerializedPin.Type.GetPinDefaultValueType())))
				{
					VOXEL_MESSAGE(Error, "{0}.{1}: Invalid default value", SerializedNode, SerializedPin.PinName);
					return false;
				}
			}
			else
			{
				ensureVoxelSlow(!SerializedPin.DefaultValue.IsValid());
			}

			Node.NewInputPin(SerializedPin.PinName, SerializedPin.Type, DefaultValue).SetParentName(SerializedPin.ParentPinName);
		}

		for (const auto& It : SerializedNode.OutputPins)
		{
			const FVoxelSerializedPin& SerializedPin = It.Value;
			if (!SerializedPin.Type.IsValid())
			{
				VOXEL_MESSAGE(Error, "Invalid pin {0}.{1}", SerializedNode, SerializedPin.PinName);
				return false;
			}

			Node.NewOutputPin(SerializedPin.PinName, SerializedPin.Type).SetParentName(SerializedPin.ParentPinName);
		}

		return true;
	}

	void CreateVoxelProxyNode(Heart::API::FNodeEdit& Edit, UVoxelProxyGraph* ProxyGraph, const FVoxelSerializedNode& Node)
	{
		Edit.Create_Reference(
			UVoxelProxyNode::StaticClass(),
			UVoxelProxyDummy::StaticClass(),
			FVector2D::ZeroVector);
				UVoxelProxyNode* NewProxy = Cast<UVoxelProxyNode>(Edit.Get());
				check(NewProxy);

		NewProxy->ProxiedNodeRef = FVoxelGraphNodeRef
		{
			ProxyGraph->GetRef(),
			Node.GetNodeId(),
			Node.EdGraphNodeTitle,
			Node.EdGraphNodeName
		};

		for (auto&& InputPin : Node.InputPins)
		{
			TSharedPtr<const FVoxelPin> Pin = Node.VoxelNode->FindPin(InputPin.Key);
			NewProxy->AddPin(VoxelPinToHeartPin(ProxyGraph,
				InputPin.Value, Pin ? Pin->Metadata : FVoxelPinMetadata(), EHeartPinDirection::Input));
		}

		for (auto&& OutputPin : Node.OutputPins)
		{
			TSharedPtr<const FVoxelPin> Pin = Node.VoxelNode->FindPin(OutputPin.Key);
			NewProxy->AddPin(VoxelPinToHeartPin(ProxyGraph,
				OutputPin.Value, Pin ? Pin->Metadata : FVoxelPinMetadata(), EHeartPinDirection::Output));
		}

		NewProxy->CleanedName = FText::FromName(Node.EdGraphNodeTitle);

		// Parse the NodeID for the node color.
		const FString NodeIDStr = Node.GetNodeId().ToString();

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

	UVoxelProxyGraph* CreateVoxelProxy(UObject* Outer, UVoxelGraph* GraphToProxy)
	{
		UVoxelProxyGraph* ProxyGraph = NewObject<UVoxelProxyGraph>(Outer);

		const UVoxelTerminalGraph& Terminal = GraphToProxy->GetMainTerminalGraph();
		const UVoxelTerminalGraphRuntime& Runtime = Terminal.GetRuntime();
		const FVoxelSerializedGraph& SerializedGraph = Runtime.GetSerializedGraph();
		const FVoxelTerminalGraphRef TerminalGraphRef(GraphToProxy, GVoxelMainTerminalGraphGuid);

		TMap<FName, FHeartNodeGuid> VoxelIDToHeartNodes;

		// Node creation scope
		{
			Heart::API::FNodeEdit NodeEdit(ProxyGraph);

			for (auto&& NameAndSerializedNode : SerializedGraph.NodeNameToNode)
			{
				CreateVoxelProxyNode(NodeEdit, ProxyGraph, NameAndSerializedNode.Value);

				VoxelIDToHeartNodes.Add(NameAndSerializedNode.Key, NodeEdit.Get()->GetGuid());
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

		ProxyGraph->SetInitialized(TerminalGraphRef);

		return ProxyGraph;
	}

	FHeartGraphPinDesc VoxelPinToHeartPin(UVoxelProxyGraph* ProxyGraph, const FVoxelSerializedPin& InPin, const FVoxelPinMetadata& PinMetadata, const EHeartPinDirection PinDirection)
	{
		FHeartGraphPinDesc NewPin;
		NewPin.Name = InPin.PinName;
		NewPin.FriendlyName = FText::FromString(FName::NameToDisplayString(InPin.PinName.ToString(), false));
		NewPin.Tag = FHeartGraphPinTag::ConvertChecked(VoxelPinTags::HeartPinTag_Voxel);
		NewPin.Direction = PinDirection;
		NewPin.Metadata.Add(ProxyGraph->GetTypeMetadata(InPin.Type));
		UHeartVoxelPerPinMetadata* PerPinMetadata = NewObject<UHeartVoxelPerPinMetadata>(ProxyGraph);
		PerPinMetadata->PinMetadata = PinMetadata;
		NewPin.Metadata.Add(PerPinMetadata);
		return NewPin;
	}

	FInstancedStruct VoxelInstancedStructDecay(const FVoxelInstancedStruct& Value)
	{
		FInstancedStruct Struct;
		Struct.InitializeAs(Value.GetScriptStruct(), static_cast<const uint8*>(Value.GetStructMemory()));
		return Struct;
	}

	FVoxelInstancedStruct VoxelInstancedStructWrap(const FInstancedStruct& Value)
	{
		FVoxelInstancedStruct Struct;
		// @todo remind Victor that InitializeAs should take a const pointer
		Struct.InitializeAs(const_cast<UScriptStruct*>(Value.GetScriptStruct()), Value.GetMemory());
		return Struct;
	}

	FBloodValue VoxelPinToBlood(const FVoxelPinValue& Value)
	{
		if (!Value.IsValid()) return FBloodValue();

		auto&& Type = Value.GetType();
		if (Value.IsArray())
		{
			const TArray<FVoxelTerminalPinValue>& ArrayData = Value.GetArray();

			auto ConvertArray = [&ArrayData]<typename T>()
			{
				TArray<T> Values;
				for (auto&& ArrayDatum : ArrayData)
				{
					Values.Add(ArrayDatum.Get<T>());
				}
				return FBloodValue(Values);
			};

			auto ConvertArray2 = [&ArrayData]<typename ProjectionType>(ProjectionType Proj)
			{
				TArray<decltype(Proj(FVoxelTerminalPinValue()))> Values;
				for (auto&& ArrayDatum : ArrayData)
				{
					Values.Add(Invoke(Proj, ArrayDatum));
				}
				return FBloodValue(Values);
			};

			// Manual converter for structs since blood cant convert from FInstancedStruct!!
			auto ConvertArray3 = [&Type, &ArrayData]()
			{
				TArray<const uint8*> Values;
				for (auto&& ArrayDatum : ArrayData)
				{
					Values.Add(static_cast<const uint8*>(ArrayDatum.GetStruct().GetStructMemory()));
				}
				return FBloodValue(Type.GetStruct(), Values);
			};

			if (Type.Is<bool>()) return ConvertArray.operator()<bool>();
			if (Type.Is<float>()) return ConvertArray.operator()<float>();
			if (Type.Is<double>()) return ConvertArray.operator()<double>();
			if (Type.Is<uint8>()) return ConvertArray.operator()<uint8>();
			if (Type.Is<int32>()) return ConvertArray.operator()<int32>();
			if (Type.Is<int64>()) return ConvertArray.operator()<int64>();
			if (Type.Is<FName>()) return ConvertArray.operator()<FName>();
			if (Type.IsClass()) return ConvertArray2.operator()([](const auto& Pin){ return Pin.GetClass(); });
			if (Type.IsObject()) return ConvertArray2.operator()([](const auto& Pin){ return Pin.GetObject(); });
			if (Type.IsStruct()) return ConvertArray3();
		}
		else
		{
			if (Type.Is<bool>()) return FBloodValue{Value.Get<bool>()};
			if (Type.Is<float>()) return FBloodValue{Value.Get<float>()};
			if (Type.Is<double>()) return FBloodValue{Value.Get<double>()};
			if (Type.Is<uint8>()) return FBloodValue{Value.Get<uint8>()};
			if (Type.Is<int32>()) return FBloodValue{Value.Get<int32>()};
			if (Type.Is<int64>()) return FBloodValue{Value.Get<int64>()};
			if (Type.Is<FName>()) return FBloodValue{Value.Get<FName>()};
			if (Type.IsClass()) return FBloodValue{Value.GetClass()};
			if (Type.IsObject()) return FBloodValue{Value.GetObject()};
			if (Type.IsStruct())
			{
				if (Type.GetStruct() == FVoxelChannelName::StaticStruct())
				{
					return FBloodValue(Value.Get<FVoxelChannelName>().Name);
				}
				if (Type.GetStruct() == FBodyInstance::StaticStruct())
				{
					// Body Instances are large as heck, all we need is the enum to generate a combo box.
					const uint8 ByteValue = Value.Get<FBodyInstance>().GetCollisionEnabled();
					return FBloodValue(ByteValue);
				}

				auto&& Struct = Value.GetStruct();
				return FBloodValue(Struct.GetScriptStruct(), static_cast<const uint8*>(Struct.GetStructMemory()));
			}
		}

		return FBloodValue();
	}

	bool IsNumeric(const FBloodValue& Value)
	{
		return Value.Is<float>() || Value.Is<double>() || Value.Is<uint8>() || Value.Is<int32>() || Value.Is<int64>();
	}

	FVoxelPinValue BloodToVoxelPin(const FBloodValue& Value, const FVoxelPinType& ExpectedType)
	{
		if (!Value.IsValid()) return FVoxelPinValue();

		if (Value.IsContainer1() || Value.IsContainer2())
		{
			unimplemented()
		}
		else
		{
			switch (ExpectedType.GetPropertyType().GetInternalType())
			{
			case EVoxelPropertyInternalType::Invalid:
				break;
			case EVoxelPropertyInternalType::Bool:
				if (Value.Is<bool>()) return FVoxelPinValue::Make(Value.GetValue<bool>());
				break;
			case EVoxelPropertyInternalType::Float:
				if (IsNumeric(Value)) return FVoxelPinValue::Make(Value.GetValue<float>());
				break;
			case EVoxelPropertyInternalType::Double:
				if (IsNumeric(Value)) return FVoxelPinValue::Make(Value.GetValue<double>());
				break;
			case EVoxelPropertyInternalType::Int32:
				if (IsNumeric(Value)) return FVoxelPinValue::Make(Value.GetValue<int32>());
				break;
			case EVoxelPropertyInternalType::Int64:
				if (IsNumeric(Value)) return FVoxelPinValue::Make(Value.GetValue<int64>());
				break;
			case EVoxelPropertyInternalType::Name:
				if (Value.Is<FName>()) return FVoxelPinValue::Make(Value.GetValue<FName>());
				if (Value.Is<FString>()) return FVoxelPinValue::Make(FName(Value.GetValue<FString>()));
				if (Value.Is<FText>()) return FVoxelPinValue::Make(FName(Value.GetValue<FText>().ToString()));
				break;
			case EVoxelPropertyInternalType::Byte:
				if (IsNumeric(Value)) return FVoxelPinValue::Make(Value.GetValue<uint8>());
				break;
			case EVoxelPropertyInternalType::Class:
				if (Value.Is<TSubclassOf<UObject>>()) return FVoxelPinValue::Make(Value.GetValue<TSubclassOf<UObject>>());
				break;
			case EVoxelPropertyInternalType::Object:
				if (Value.Is<TObjectPtr<UObject>>()) return FVoxelPinValue::Make(Value.GetValue<TObjectPtr<UObject>>());
				break;
			case EVoxelPropertyInternalType::Struct:
				if (ExpectedType.GetStruct() == FVoxelChannelName::StaticStruct())
				{
					if (Value.Is<FName>()) return FVoxelPinValue::Make(FVoxelChannelName(Value.GetValue<FName>()));
					if (Value.Is<FString>()) return FVoxelPinValue::Make(FVoxelChannelName(FName(Value.GetValue<FString>())));
				}
				else if (ExpectedType.GetStruct() == FVoxelExposedSeed::StaticStruct())
				{
					if (Value.Is<FName>()) return FVoxelPinValue::Make(FVoxelExposedSeed(Value.GetValue<FName>().ToString()));
					if (Value.Is<FString>()) return FVoxelPinValue::Make(FVoxelExposedSeed(Value.GetValue<FString>()));
				}
				else if (ExpectedType.GetStruct() == FVoxelSeed::StaticStruct())
				{
					if (Value.Is<FName>())
					{
						const FVoxelSeed Seed = FCrc::StrCrc32(*Value.GetValue<FName>().ToString());
						return FVoxelPinValue::Make(Seed);
					}
					if (Value.Is<FString>())
					{
						const FVoxelSeed Seed = FCrc::StrCrc32(*Value.GetValue<FString>());
						return FVoxelPinValue::Make(Seed);
					}
				}
				else if (ExpectedType.GetStruct() == FBodyInstance::StaticStruct())
				{
					if (IsNumeric(Value))
					{
						FBodyInstance BodyInstance;
						BodyInstance.SetCollisionEnabled(static_cast<ECollisionEnabled::Type>(Value.GetValue<uint8>()));
						return FVoxelPinValue::Make(BodyInstance);
					}
				}

				return FVoxelPinValue::MakeStruct(VoxelInstancedStructWrap(Value.GetStruct()));
			}
		}

		return FVoxelPinValue();
	}
}