// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "NativeGameplayTags.h"
#include "VoxelPinType.h"
#include "Model/HeartGraphPinMetadata.h"

#include "VoxelProxyPin.generated.h"

struct FVoxelSerializedPin;
class UVoxelProxyGraph;

namespace VoxelPinTags
{
	// Tags Voxel pins for Heart Registrars
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(HeartPinTag_Voxel)
}

UENUM(BlueprintType)
enum class EVoxelPinProxyType : uint8
{
	Invalid,
	Wildcard,
	Bool,
	Float,
	Double,
	Int32,
	Int64,
	Name,
	Byte,
	Class,
	Object,
	Struct
};

UCLASS()
class UHeartVoxelPinTypeWrapper : public UHeartGraphPinMetadata
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "VoxelPinType")
	bool IsBuffer() const { return PinType.IsBuffer(); }

	UFUNCTION(BlueprintCallable, Category = "VoxelPinType")
	bool IsBufferArray() const { return PinType.IsBufferArray(); }

	UFUNCTION(BlueprintCallable, Category = "VoxelPinType")
	EVoxelPinProxyType GetPinType() const;

	UFUNCTION(BlueprintCallable, Category = "VoxelPinType")
	FLinearColor GetPinColor() const;

	UFUNCTION(BlueprintCallable, Category = "VoxelPinType")
	FString ToString() const { return PinType.ToString(); }

	UPROPERTY()
	FVoxelPinType PinType;
};