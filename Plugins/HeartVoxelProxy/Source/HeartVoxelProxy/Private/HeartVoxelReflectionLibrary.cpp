// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "HeartVoxelReflectionLibrary.h"

#define LOCTEXT_NAMESPACE "HeartVoxelReflectionLibrary"

const UEnum* UHeartVoxelReflectionLibrary::CastToUEnum(const UObject* Object)
{
	return Cast<UEnum>(Object);
}

int32 UHeartVoxelReflectionLibrary::GetEnumIndexFromValue(const UEnum* Enum, const int64 Value)
{
	if (!IsValid(Enum))
	{
		return 0;
	}

	return Enum->GetIndexByValue(Value);
}

int64 UHeartVoxelReflectionLibrary::GetEnumValueFromIndex(const UEnum* Enum, const int32 Index)
{
	if (!IsValid(Enum))
	{
		return 0;
	}

	return Enum->GetValueByIndex(Index);
}

TArray<FText> UHeartVoxelReflectionLibrary::GetAllEnumNames(const UEnum* Enum)
{
	if (!IsValid(Enum))
	{
		return {};
	}

	TArray<FText> Out;
	Out.Reserve(Enum->NumEnums());

	// Iterate until the last index - 1 to skip the _MAX entry, which shouldn't be user facing.
	for (int32 i = 0; i < Enum->NumEnums() - 1; ++i)
	{
		Out.Add(Enum->GetDisplayNameTextByIndex(i));
	}
	return Out;
}

int64 UHeartVoxelReflectionLibrary::NameToEnumValue(const FName Value, const UEnum* Enum)
{
	if (!IsValid(Enum))
	{
		return 0;
	}

	return Enum->GetValueByName(Value);
}

TArray<FText> UHeartVoxelReflectionLibrary::GetCollisionTypeOptions()
{
	static const UEnum* Enum = StaticEnum<ECollisionEnabled::Type>();
	return GetAllEnumNames(Enum);
}

#undef LOCTEXT_NAMESPACE