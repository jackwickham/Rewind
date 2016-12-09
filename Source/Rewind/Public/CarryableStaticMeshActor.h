// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "RewindableStaticMeshActor.h"

#include "ButtonActivationInterface.h"

#include "CarryableStaticMeshActor.generated.h"

/**
 * 
 */
UCLASS()
class REWIND_API ACarryableStaticMeshActor : public ARewindableStaticMeshActor, public IButtonActivationInterface
{
	GENERATED_BODY()

	ACarryableStaticMeshActor();
	
public:
	UPROPERTY(BlueprintReadOnly, Category = Interaction)
		bool beingCarried = false;
	
	
};
