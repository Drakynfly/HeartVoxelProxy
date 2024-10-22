// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "HeartVoxelMarriage.generated.h"

class AVoxelActor;
class UVoxelGraph;
class UVoxelProxyGraph;

UCLASS(ClassGroup = ("Heart/Voxel"), meta = (BlueprintSpawnableComponent))
class HEARTVOXELPROXY_API UHeartVoxelMarriage : public UActorComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "HeartVoxel|Setup")
	void GenerateProxyFromVoxelGraph(UVoxelGraph* Graph);

	UFUNCTION(BlueprintCallable, Category = "HeartVoxel|Setup")
	void SetVoxelActor(AVoxelActor* Actor);

	void SyncWithVoxelActor();

	void RestartVoxelGraph();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Graphs")
	TObjectPtr<AVoxelActor> VoxelActor;

	// The live graph.
	UPROPERTY(BlueprintReadOnly, Category = "Graphs")
	TObjectPtr<UVoxelGraph> RuntimeGraph;

	// The simulated graph for viewing/editing.
	UPROPERTY(BlueprintReadOnly, Category = "Graphs")
	TObjectPtr<UVoxelProxyGraph> ProxyGraph;
};