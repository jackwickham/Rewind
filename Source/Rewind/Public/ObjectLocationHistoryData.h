// Copyright (C) 2015 Jack Wickham - All Rights Reserved

#pragma once

//Forward declare ARewindableStaticMeshActor
class ARewindableStaticMeshActor;

#include "ObjectLocationHistoryData.generated.h"

/**
*
*/

USTRUCT()
struct FLocationHistory {
	GENERATED_USTRUCT_BODY()

		//The time that the location data was generated
		UPROPERTY(BlueprintReadWrite, Category = Rewind)
		float timestamp;

	//A pointer to the actor whos location is being logged
	UPROPERTY(BlueprintReadWrite, Category = Rewind)
		ARewindableStaticMeshActor* actor;

	//The remaining lifespan of the actor
	UPROPERTY(BlueprintReadWrite, Category = Rewind)
		float lifespan;

	//The transform (location and rotation) of the actor
	UPROPERTY(BlueprintReadWrite, Category = Rewind)
		FTransform actorTransform;

	//The physics velocity of the actor
	UPROPERTY(BlueprintReadWrite, Category = Rewind)
		FVector velocity;

	UPROPERTY(BlueprintReadWrite, Category = Rewind)
		float timelineProgress;

	//Allow for equality checking
	bool operator==(const FLocationHistory& right){
		return this->timestamp == right.timestamp &&
			this->actor == right.actor &&
			this->actorTransform.Equals(right.actorTransform, 0.0001f); //This might be throwing an exception? Not sure (and breakpoints don't work in here)
	}

	//Constructor
	/*FLocationHistory(float timestamp, AActor* actor, float lifespan, FTransform actorTransform, FVector velocity, float timelineProgress = NULL)
	: timestamp(timestamp),
	actor(actor),
	lifespan(lifespan),
	actorTransform(actorTransform),
	velocity(velocity),
	timelineProgress(timelineProgress) {}*/

};
