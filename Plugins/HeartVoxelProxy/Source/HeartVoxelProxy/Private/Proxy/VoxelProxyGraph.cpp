// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Proxy/VoxelProxyGraph.h"
#include "Proxy/VoxelProxyConverters.h"
#include "Proxy/VoxelProxyNode.h"
#include "Proxy/VoxelProxyPin.h"

#include "Model/HeartGraphNode.h"

#include "VoxelGraph.h"
#include "VoxelGraphParametersView.h"
#include "VoxelParameterView.h"
#include "General/ObjectTree.h"


UVoxelProxyGraph::UVoxelProxyGraph()
{
	AssetPickerOptions = CreateDefaultSubobject<UHeartObjectTree>(TEXT("AssetPickerOptions"));
	ClassPickerOptions = CreateDefaultSubobject<UHeartObjectTree>(TEXT("ClassPickerOptions"));
}

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

void UVoxelProxyGraph::SetInitialized(const FVoxelTerminalGraphRef& GraphRef)
{
	check(!Initialized);
	TerminalGraphRef = GraphRef;
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