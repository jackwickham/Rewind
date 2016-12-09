// Copyright (c) 2015 Jack Wickham - All Rights Reserved

#pragma once

#include "Engine/StaticMeshActor.h"
#include "InteractiveStaticMeshActor.generated.h"

/**
 * 
 */
UCLASS()
class REWIND_API AInteractiveStaticMeshActor : public AStaticMeshActor
{
	GENERATED_BODY()
	
public:
	//Whether it can be triggered multiple times, or just once
	UPROPERTY(EditAnywhere, Category = Interaction)
		bool triggerOnce = false;

	//Event implemented in blueprint to handle interaction start
	UFUNCTION(BlueprintImplementableEvent, Category = Interaction)
		void onInteract();
	//Event implemented in blueprint to handle interaction end
	UFUNCTION(BlueprintImplementableEvent, Category = Interaction)
		void onEndInteract();

	//Whether the object has interacted yet
	UPROPERTY(BlueprintReadWrite, Category = Interaction)
		bool enabled = true;

	//Called from the character controller when they initiate an interaction event with this actor
	void onKeyPress();
	//Called from the character controller when they end an interaction event with this actor
	void onKeyRelease();
	
	
};
