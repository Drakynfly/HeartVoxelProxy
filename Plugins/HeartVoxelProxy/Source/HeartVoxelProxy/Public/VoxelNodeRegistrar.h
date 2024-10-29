// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "GraphRegistry/GraphNodeRegistrar.h"
#include "VoxelNodeRegistrar.generated.h"

/**
 *
 */
UCLASS()
class HEARTVOXELPROXY_API UVoxelNodeRegistrar : public UGraphNodeRegistrar
{
	GENERATED_BODY()

public:
	UVoxelNodeRegistrar();

	virtual bool ShouldRegister() const override;

	virtual void PreSave(FObjectPreSaveContext SaveContext) override;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "Reset")
	bool RegenerateOnSave = false;
#endif
};