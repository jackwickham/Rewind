// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

//Forward declare FLocationHistory
class AObjectLocationHistory2;


#include "Engine/StaticMeshActor.h"

#include "ObjectLocationHistoryData.h"

#include "RewindableStaticMeshActor.generated.h"

/**
 * A static mesh actor which can, if rewinds is set to true, be rewound
 */
UCLASS()
class REWIND_API ARewindableStaticMeshActor : public AStaticMeshActor
{
	GENERATED_BODY()
	

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rewind)
		bool rewinds = true;

	//Create the location history object to save the current location data
	UFUNCTION(BlueprintNativeEvent, Category = Rewind)
		FLocationHistory getLocation();
	FLocationHistory getLocation_Implementation();

	//Reset the location of the actor to the new location
	UFUNCTION(BlueprintNativeEvent, Category = Rewind)
		void setLocation(FLocationHistory newState, int32 currentGameTime);
	void setLocation_Implementation(FLocationHistory newState, int32 currentGameTime);
	
	
};
