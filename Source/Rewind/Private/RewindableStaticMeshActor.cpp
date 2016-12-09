// Fill out your copyright notice in the Description page of Project Settings.

#include "Rewind.h"
#include "Public/RewindableStaticMeshActor.h"

#include "Public/RewindGameState.h"


FLocationHistory ARewindableStaticMeshActor::getLocation_Implementation(){
	//Get the game state
	ARewindGameState* gameState = Cast<ARewindGameState>(UGameplayStatics::GetGameState(GetWorld()));

	/*Get and store the actor's location*/
	FTransform transform = GetTransform();
	/*Get the current velocity*/
	FVector velocity = GetVelocity();

	//Check if the actor has changed since the last frame
	if (gameState->ActorCurrentLocations.FindByPredicate([&](FLocationHistory entry){
		return entry.actor == this &&
			entry.actorTransform.Equals(transform, 0.001f) &&
			entry.velocity.Equals(velocity, 0.001f);

	}) != NULL) { //Nothing we need to do
		FLocationHistory ret = {};
		return ret;
	}
	//not changed (or doesn't need to)
	FLocationHistory* currentLocation = gameState->ActorCurrentLocations.FindByPredicate([&](FLocationHistory entry){
		return entry.actor == this;
	});
	if (currentLocation == NULL){
		//Not already in array - create a new entry
		gameState->ActorCurrentLocations.Add({
			0,
			this,
			0,
			transform,
			velocity
		});
		//Save the fact that the actor has just been created
		gameState->ActorCreationTimes.Add({
			this,
			gameState->currentGameTime
		});
	}
	else {
		currentLocation->actorTransform = transform;
		currentLocation->velocity = velocity;
	}

	float currentLifespan = GetLifeSpan();

	FLocationHistory container = {
		gameState->currentGameTime,
		this,
		currentLifespan,
		transform,
		velocity
	};

	return container;
}

void ARewindableStaticMeshActor::setLocation_Implementation(FLocationHistory newState, int32 currentGameTime) {
	//Set tranformation
	SetActorTransform(newState.actorTransform);
	//Set physics velocity
	UPrimitiveComponent* component = Cast<UPrimitiveComponent>(GetRootComponent());
	if (component != NULL) {
		//Set the actor's velocity
		component->SetPhysicsLinearVelocity(newState.velocity);
	}
	//Set lifespan
	//This isn't perfect because when the game is "paused" the decay-thing doesn't work
	if (newState.lifespan != 0) {
		//calculate the difference between the time that the snapshot was taken and the current time
		float deltaTime = currentGameTime - newState.timestamp;
		float newLifespan = newState.lifespan - deltaTime;
		newState.actor->SetLifeSpan(newLifespan);
	}
}