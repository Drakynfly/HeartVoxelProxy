// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "HeartVoxelMarriage.generated.h"

class AVoxelActor;
class UVoxelGraph;
class UVoxelProxyGraph;

/**
 * A sample implementation of binding a VoxelGraph to a HeartGraph.
 *
 * To use:
 * 1. Add to any actor. The player controller works fine.
 * 2. Call any of the three "SetupWith..." functions.
 * 3. Call SetVoxelActor
 */
UCLASS(ClassGroup = ("Heart/Voxel"), meta = (BlueprintSpawnableComponent))
class HEARTVOXELPROXY_API UHeartVoxelMarriage : public UActorComponent
{
	GENERATED_BODY()

public:
	// Init with a new blank graph
	UFUNCTION(BlueprintCallable, Category = "HeartVoxel|Setup")
	void SetupWithBlank();

	// Init with a reference to a graph to edit (do not call this with assets)
	UFUNCTION(BlueprintCallable, Category = "HeartVoxel|Setup")
	void SetupWithExisting(UVoxelGraph* Graph);

	// Init by copying an existing graph
	UFUNCTION(BlueprintCallable, Category = "HeartVoxel|Setup")
	void SetupWithCopy(const UVoxelGraph* Graph);

	UFUNCTION(BlueprintCallable, Category = "HeartVoxel|Setup")
	void SetVoxelActor(AVoxelActor* Actor);

protected:
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