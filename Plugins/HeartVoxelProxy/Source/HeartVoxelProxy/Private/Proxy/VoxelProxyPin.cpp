// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Proxy/VoxelProxyPin.h"

#include "VoxelExposedSeed.h"
#include "VoxelSurface.h"
#include "Channel/VoxelChannelName.h"
#include "Point/VoxelPointSet.h"

namespace PinColors
{
	// graph node pin type colors

	// Copied from UGraphEditorSettings
	static constexpr FLinearColor DefaultPinTypeColor = FLinearColor(0.750000f, 0.6f, 0.4f, 1.0f);			// light brown
	//static constexpr FLinearColor ExecutionPinTypeColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);				// white
	static constexpr FLinearColor BooleanPinTypeColor = FLinearColor(0.300000f, 0.0f, 0.0f, 1.0f);			// maroon
	static constexpr FLinearColor BytePinTypeColor = FLinearColor(0.0f, 0.160000f, 0.131270f, 1.0f);			// dark green
	static constexpr FLinearColor ClassPinTypeColor = FLinearColor(0.1f, 0.0f, 0.5f, 1.0f);					// deep purple (violet)
	static constexpr FLinearColor IntPinTypeColor = FLinearColor(0.013575f, 0.770000f, 0.429609f, 1.0f);		// green-blue
	static constexpr FLinearColor Int64PinTypeColor = FLinearColor(0.413575f, 0.770000f, 0.429609f, 1.0f);
	static constexpr FLinearColor FloatPinTypeColor = FLinearColor(0.357667f, 1.0f, 0.060000f, 1.0f);			// bright green
	static constexpr FLinearColor DoublePinTypeColor = FLinearColor(0.039216f, 0.666667f, 0.0f, 1.0f);		// darker green
	//static constexpr FLinearColor RealPinTypeColor = FLinearColor(0.039216f, 0.666667f, 0.0f, 1.0f);			// darker green
	//static constexpr FLinearColor NamePinTypeColor = FLinearColor(0.607717f, 0.224984f, 1.0f, 1.0f);			// lilac
	//static constexpr FLinearColor DelegatePinTypeColor = FLinearColor(1.0f, 0.04f, 0.04f, 1.0f);				// bright red
	static constexpr FLinearColor ObjectPinTypeColor = FLinearColor(0.0f, 0.4f, 0.910000f, 1.0f);				// sharp blue
	//static constexpr FLinearColor SoftObjectPinTypeColor = FLinearColor(0.3f, 1.0f, 1.0f, 1.0f);
	//static constexpr FLinearColor SoftClassPinTypeColor = FLinearColor(1.0f, 0.3f, 1.0f, 1.0f);
	//static constexpr FLinearColor InterfacePinTypeColor = FLinearColor(0.8784f, 1.0f, 0.4f, 1.0f);			// pale green
	static constexpr FLinearColor StringPinTypeColor = FLinearColor(1.0f, 0.0f, 0.660537f, 1.0f);				// bright pink
	//static constexpr FLinearColor TextPinTypeColor = FLinearColor(0.8f, 0.2f, 0.4f, 1.0f);					// salmon (light pink)
	static constexpr FLinearColor StructPinTypeColor = FLinearColor(0.0f, 0.1f, 0.6f, 1.0f);					// deep blue
	static constexpr FLinearColor WildcardPinTypeColor = FLinearColor(0.220000f, 0.195800f, 0.195800f, 1.0f);	// dark gray
	static constexpr FLinearColor VectorPinTypeColor = FLinearColor(1.0f, 0.591255f, 0.016512f, 1.0f);		// yellow
	static constexpr FLinearColor RotatorPinTypeColor = FLinearColor(0.353393f, 0.454175f, 1.0f, 1.0f);		// periwinkle
	static constexpr FLinearColor TransformPinTypeColor = FLinearColor(1.0f, 0.172585f, 0.0f, 1.0f);			// orange
	//static constexpr FLinearColor IndexPinTypeColor = FLinearColor(0.013575f, 0.770000f, 0.429609f, 1.0f);	// green-blue

	// Copied from UVoxelGraphEditorSettings
	static constexpr FLinearColor PointSetPinColor = FLinearColor(0.607717f, 0.224984f, 1.f, 1.f);
	static constexpr FLinearColor SurfacePinColor = FLinearColor(0.007499f, 0.64448f, 0.730461f, 1.f);
	static constexpr FLinearColor SeedPinColor = FLinearColor(0.607717f, 0.224984f, 1.f, 1.f);
	static constexpr FLinearColor ChannelPinColor = FLinearColor(0.607717f, 0.224984f, 1.f, 1.f);
}

namespace VoxelPinTags
{
	UE_DEFINE_GAMEPLAY_TAG(HeartPinTag_Voxel, "Heart.Pin.Voxel")
}

EVoxelPinProxyType UHeartVoxelPinTypeWrapper::GetPinType() const
{
	// Buffer has same color as inner
	const FVoxelPinType Type = PinType.GetInnerExposedType();

	if (Type.IsWildcard())
	{
		return EVoxelPinProxyType::Wildcard;
	}
	else if (Type.Is<bool>())
	{
		return EVoxelPinProxyType::Bool;
	}
	else if (Type.Is<float>())
	{
		return EVoxelPinProxyType::Float;
	}
	else if (Type.Is<double>())
	{
		return EVoxelPinProxyType::Double;
	}
	else if (Type.Is<int32>())
	{
		return EVoxelPinProxyType::Int32;
	}
	else if (Type.Is<int64>())
	{
		return EVoxelPinProxyType::Int64;
	}
	else if (Type.Is<FName>())
	{
		return EVoxelPinProxyType::Name;
	}
	else if (Type.Is<uint8>())
	{
		return EVoxelPinProxyType::Byte;
	}
	else if (Type.Is<FVoxelPointSet>())
	{
		return EVoxelPinProxyType::Struct;
	}
	else if (Type.Is<FVoxelSurface>())
	{
		return EVoxelPinProxyType::Struct;
	}
	else if (Type.Is<FVector>())
	{
		return EVoxelPinProxyType::Struct;
	}
	else if (
		Type.Is<FRotator>() ||
		Type.Is<FQuat>())
	{
		return EVoxelPinProxyType::Struct;
	}
	else if (Type.Is<FTransform>())
	{
		return EVoxelPinProxyType::Struct;
	}
	else if (Type.Is<FVoxelExposedSeed>())
	{
		return EVoxelPinProxyType::Struct;
	}
	else if (Type.Is<FVoxelChannelName>())
	{
		return EVoxelPinProxyType::Struct;
	}
	else if (Type.Is<FVoxelFloatRange>())
	{
		return EVoxelPinProxyType::Struct;
	}
	else if (Type.Is<FVoxelInt32Range>())
	{
		return EVoxelPinProxyType::Struct;
	}
	else if (Type.IsClass())
	{
		return EVoxelPinProxyType::Class;
	}
	else if (Type.IsObject())
	{
		return EVoxelPinProxyType::Object;
	}
	else if (Type.IsStruct())
	{
		return EVoxelPinProxyType::Struct;
	}
	else
	{
		return EVoxelPinProxyType::Invalid;
	}
}

EVoxelPinProxyStructType UHeartVoxelPinTypeWrapper::GetPinStructType() const
{
	const FVoxelPinType Type = PinType.GetInnerExposedType();

	if (Type.Is<FVoxelExposedSeed>())
	{
		return EVoxelPinProxyStructType::VoxelExposedSeed;
	}
	if (Type.Is<FVoxelChannelName>())
	{
		return EVoxelPinProxyStructType::VoxelChannelName;
	}
	if (Type.Is<FBodyInstance>())
	{
		return EVoxelPinProxyStructType::BodyInstance;
	}

	return EVoxelPinProxyStructType::Generic;
}

// Implementation based on FVoxelGraphVisuals::GetPinColor
FLinearColor UHeartVoxelPinTypeWrapper::GetPinColor() const
{
	// Buffer has same color as inner
	const FVoxelPinType Type = PinType.GetInnerExposedType();

	if (Type.IsWildcard())
	{
		return PinColors::WildcardPinTypeColor;
	}
	else if (Type.Is<bool>())
	{
		return PinColors::BooleanPinTypeColor;
	}
	else if (Type.Is<float>())
	{
		return PinColors::FloatPinTypeColor;
	}
	else if (Type.Is<double>())
	{
		return PinColors::DoublePinTypeColor;
	}
	else if (Type.Is<int32>())
	{
		return PinColors::IntPinTypeColor;
	}
	else if (Type.Is<int64>())
	{
		return PinColors::Int64PinTypeColor;
	}
	else if (Type.Is<FName>())
	{
		return PinColors::StringPinTypeColor;
	}
	else if (Type.Is<uint8>())
	{
		return PinColors::BytePinTypeColor;
	}
	else if (Type.Is<FVoxelPointSet>())
	{
		return PinColors::PointSetPinColor;
	}
	else if (Type.Is<FVoxelSurface>())
	{
		return PinColors::SurfacePinColor;
	}
	else if (Type.Is<FVector>())
	{
		return PinColors::VectorPinTypeColor;
	}
	else if (
		Type.Is<FRotator>() ||
		Type.Is<FQuat>())
	{
		return PinColors::RotatorPinTypeColor;
	}
	else if (Type.Is<FTransform>())
	{
		return PinColors::TransformPinTypeColor;
	}
	else if (Type.Is<FVoxelExposedSeed>())
	{
		return PinColors::SeedPinColor;
	}
	else if (Type.Is<FVoxelChannelName>())
	{
		return PinColors::ChannelPinColor;
	}
	else if (Type.Is<FVoxelFloatRange>())
	{
		return PinColors::FloatPinTypeColor;
	}
	else if (Type.Is<FVoxelInt32Range>())
	{
		return PinColors::IntPinTypeColor;
	}
	else if (Type.IsClass())
	{
		return PinColors::ClassPinTypeColor;
	}
	else if (Type.IsObject())
	{
		return PinColors::ObjectPinTypeColor;
	}
	else if (Type.IsStruct())
	{
		return PinColors::StructPinTypeColor;
	}
	else
	{
		return PinColors::DefaultPinTypeColor;
	}
}