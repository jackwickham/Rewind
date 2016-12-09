// Fill out your copyright notice in the Description page of Project Settings.

#include "Rewind.h"
#include "Public/RewindGameState.h"

#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::White,text)

//Constructor
ARewindGameState::ARewindGameState(const FObjectInitializer& ObjectInitialiser) : Super(ObjectInitialiser) {
	PrimaryActorTick.bCanEverTick = true;
}

void ARewindGameState::BeginPlay(){
	//Set time time to -0.01 to ensure that we can never overwrite the first snapshot, so we can always rewind all the way
	currentGameTime = -0.01f;

	Super::BeginPlay();

	for (TActorIterator<ARewindableStaticMeshActor> Itr(GetWorld()); Itr; ++Itr) {
		createLocationHistory((ARewindableStaticMeshActor*)&Itr);
	}
}
//Tick - Updates the game time every frame
void ARewindGameState::Tick(float deltaSeconds){
	if (!bGamePaused) {
		currentGameTime += deltaSeconds;
	}

	Super::Tick(deltaSeconds);
}

//Generates the properties for an objectLocationHistory, then has it appended to the array
void ARewindGameState::createLocationHistory(ARewindableStaticMeshActor* actor) {
	//make sure the actor is valid (I don't know *why* I need to perform this check, but it crashes otherwise (possibly a corrupt actor?))
	if (/*actor->GetStaticMeshComponent() == NULL || */!actor->IsValidLowLevel()) return;
	//Create and append the location history
	FLocationHistory history = actor->getLocation();
	if (history.actor == NULL) return;
	arrLocationHistory.Add(history);

}


//Reverts the actor locations back to how they were at the specified time
void ARewindGameState::revertLocationHistory(float timestamp) {
	if (timestamp < 0) timestamp = 0;
	TArray<FLocationHistory> bestNewLocation;

	//Iterate through the history to find the relevant locations
	for (int32 index = arrLocationHistory.Num() - 1; index >= 0; index--) {
		//The current location history entry
		FLocationHistory location = arrLocationHistory[index];
		//Prevent NULL pointer exceptions due to errors in the log
		if (location.actor == NULL){
			UE_LOG(LogTemp, Error, TEXT("When resetting location history, actor reference was NULL (caught in instance 1)"));
			print("Error: Actor reference was null (1)");
			arrLocationHistory.Remove(location); //Remove the broken entry from the array
			continue;
		}
		//We want the last update that was before the timestamp 
		if (location.timestamp < timestamp) {
			//Find any elements on the best new location array that match our current actor
			FLocationHistory* existing = bestNewLocation.FindByPredicate([&](FLocationHistory entry){
				return entry.actor == location.actor;
			});
			//If the existing version is older than the current best (we want newest)
			if (existing != NULL){
				if (existing->timestamp < location.timestamp){
					//replace element
					bestNewLocation.RemoveAll([&](FLocationHistory entry){
						return entry.actor == existing->actor &&
							entry.timestamp == existing->timestamp &&
							entry.actorTransform.Equals(existing->actorTransform, 0.001f);
					});
					bestNewLocation.Add(location);
				}
			}
			else {
				bestNewLocation.Add(location);
			}
		}
		else {
			//Remove redundant entries (we can't go forwards in time at the moment)
			arrLocationHistory.RemoveAt(index);
		}
	}
	//Iterate through the best new location states and set the new actor locations
	for (FLocationHistory newState : bestNewLocation) {
		ARewindableStaticMeshActor* targetActor = newState.actor;
		if (targetActor == NULL){
			//THIS MUST GO FIRST
			//Sometimes the target actor seems to be a null pointer for some reason (usually associated with performance dips)
			//This will catch the error
			UE_LOG(LogTemp, Error, TEXT("When resetting location history, actor reference was NULL"));
			print("Error: Actor reference was null");
			continue;
		}
		if (!IsValid(targetActor)) {
			//Actor isn't usable - it's probably been destroyed
			continue;
		}
		if (targetActor->IsA(APlayerController::StaticClass()) ||
			targetActor->IsA(ACharacter::StaticClass())) {
			//We don't want to move the player
			continue;
		}

		targetActor->setLocation(newState, currentGameTime);
	}

	//Iterate through recently created actors and destroy them if created after the current game time
	for (FActorLifespans creationTime : ActorCreationTimes) {
		//Check whether the actor needs deleting and whether it has already been deleted
		if (creationTime.timestamp > timestamp && IsValid(creationTime.actor)) {
			creationTime.actor->Destroy();
		}
	}

	currentGameTime = timestamp;

	//print("Reset actor locations");
}