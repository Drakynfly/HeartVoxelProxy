// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "VoxelGraphNodeRef.h"
#include "Model/HeartGraphNode.h"
#include "VoxelProxyNode.generated.h"

UENUM()
enum class EVoxelProxyNodeColor : uint8
{
	Blue,
	Green,
	Red,
	Orange,
	White
 };

/**
 *
 */
UCLASS()
class HEARTVOXELPROXY_API UVoxelProxyNode : public UHeartGraphNode
{
	GENERATED_BODY()

public:
	virtual FText GetNodeTitle_Implementation(const UObject* Node) const override;
	virtual FLinearColor GetNodeTitleColor_Implementation(const UObject* Node) const override;

	UFUNCTION(BlueprintCallable, Category = "HeartVoxel|Editing")
	void SetPinDefaultValue_Float(FName Pin, double Value);

	UFUNCTION(BlueprintCallable, Category = "VoxelProxy|PinValue")
	double GetPinDefaultValue_Float(FName Pin) const;

	UPROPERTY()
	FVoxelGraphNodeRef ProxiedNodeRef;

	UPROPERTY()
	FText CleanedName;

	UPROPERTY()
	EVoxelProxyNodeColor NodeColor = EVoxelProxyNodeColor::White;
};