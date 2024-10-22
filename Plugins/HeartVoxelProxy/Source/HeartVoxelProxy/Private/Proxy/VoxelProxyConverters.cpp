// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Proxy/VoxelProxyConverters.h"
#include "VoxelPropertyType.h"
#include "Proxy/VoxelProxyGraph.h"
#include "Proxy/VoxelProxyPin.h"

#include "VoxelSerializedGraph.h"

namespace Converters
{
	FHeartGraphPinDesc VoxelPinToHeartPin(UVoxelProxyGraph* ProxyGraph, const FVoxelSerializedPin& InPin, const EHeartPinDirection PinDirection)
	{
		FHeartGraphPinDesc NewPin;
		NewPin.Name = InPin.PinName;
		NewPin.FriendlyName = FText::FromString(FName::NameToDisplayString(InPin.PinName.ToString(), false));
		NewPin.Tag = FHeartGraphPinTag::ConvertChecked(VoxelPinTags::HeartPinTag_Voxel);
		NewPin.Direction = PinDirection;
		NewPin.Metadata.Add(ProxyGraph->GetTypeMetadata(InPin.Type));
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

		if (Value.IsContainer1())
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
				return FVoxelPinValue::MakeStruct(VoxelInstancedStructWrap(Value.GetStruct()));
				break;
			}
		}

		return FVoxelPinValue();
	}
}