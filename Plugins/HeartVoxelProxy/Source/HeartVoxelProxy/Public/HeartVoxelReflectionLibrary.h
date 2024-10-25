// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "HeartVoxelReflectionLibrary.generated.h"

/**
 *
 */
UCLASS()
class HEARTVOXELPROXY_API UHeartVoxelReflectionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Reflection|Enum")
	static const UEnum* CastToUEnum(const UObject* Object);

	UFUNCTION(BlueprintPure, Category = "Reflection|Enum")
	static int32 GetEnumIndexFromValue(const UEnum* Enum, const int64 Value);

	UFUNCTION(BlueprintPure, Category = "Reflection|Enum")
	static int64 GetEnumValueFromIndex(const UEnum* Enum, const int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Reflection|Enum")
	static TArray<FText> GetAllEnumNames(const UEnum* Enum);

	UFUNCTION(BlueprintPure, Category = "Reflection|Enum")
	static int64 NameToEnumValue(FName Value, const UEnum* Enum);

	UFUNCTION(BlueprintCallable, Category = "Reflection|Enum")
	static TArray<FText> GetCollisionTypeOptions();
};