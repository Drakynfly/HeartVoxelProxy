// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "NativeGameplayTags.h"
#include "VoxelNode.h"
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

UENUM(BlueprintType)
enum class EVoxelPinProxyStructType : uint8
{
	// Exposed as an InstancedStruct
	Generic,

	// Exposed as an FName
	VoxelChannelName,

	// Exposed as an FString
	VoxelExposedSeed,

	// Exposed as a uint8
	BodyInstance
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
	EVoxelPinProxyStructType GetPinStructType() const;

	/* Get the field object used to filter values when in Enum/Struct/Object/Class modes. */
	UFUNCTION(BlueprintCallable, Category = "VoxelPinType")
	const UField* GetFieldFilter() const;

	UFUNCTION(BlueprintCallable, Category = "VoxelPinType")
	FLinearColor GetPinColor() const;

	UFUNCTION(BlueprintCallable, Category = "VoxelPinType")
	FString ToString() const { return PinType.ToString(); }

	UPROPERTY()
	FVoxelPinType PinType;
};

UCLASS()
class UHeartVoxelPerPinMetadata : public UHeartGraphPinMetadata
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "VoxelPinType")
	bool GetHidePin() const { return PinMetadata.bHidePin; }

	UFUNCTION(BlueprintCallable, Category = "VoxelPinType")
	bool GetArrayPin() const { return PinMetadata.bArrayPin; }

	UFUNCTION(BlueprintCallable, Category = "VoxelPinType")
	bool GetVirtualPin() const { return PinMetadata.bVirtualPin; }

	UFUNCTION(BlueprintCallable, Category = "VoxelPinType")
	bool GetConstantPin() const { return PinMetadata.bConstantPin; }

	UFUNCTION(BlueprintCallable, Category = "VoxelPinType")
	bool GetOptionalPin() const { return PinMetadata.bOptionalPin; }

	UFUNCTION(BlueprintCallable, Category = "VoxelPinType")
	bool GetDisplayLast() const { return PinMetadata.bDisplayLast; }

	UFUNCTION(BlueprintCallable, Category = "VoxelPinType")
	bool GetNoDefault() const { return PinMetadata.bNoDefault; }

	UFUNCTION(BlueprintCallable, Category = "VoxelPinType")
	bool GetShowInDetail() const { return PinMetadata.bShowInDetail; }

	FVoxelPinMetadata PinMetadata;
};