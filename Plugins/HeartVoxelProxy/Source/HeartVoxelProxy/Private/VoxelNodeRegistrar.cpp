// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "VoxelNodeRegistrar.h"
#include "VoxelNode.h"
#include "Proxy/VoxelProxyGraph.h"
#include "Proxy/VoxelProxyNode.h"

UVoxelNodeRegistrar::UVoxelNodeRegistrar()
{
	AutoRegisterWith.Add(UVoxelProxyGraph::StaticClass());

	FHeartNodeClassList& ClassList = Registration.GraphNodeLists.Add(UVoxelProxyNode::StaticClass());

	for (UScriptStruct* Struct : GetDerivedStructs<FVoxelNode>())
	{
		ClassList.Objects.Add(Struct);
	}
}