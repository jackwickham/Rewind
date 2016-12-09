// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameState.h"

#include "RewindableStaticMeshActor.h"
#include "ObjectLocationHistoryData.h"
#include "Engine.h"

#include "RewindGameState.generated.h"

USTRUCT()
struct FActorLifespans {
	GENERATED_USTRUCT_BODY()

	//Reference to actor
	UPROPERTY()
		AActor* actor;

	//creation/destruction time
	UPROPERTY()
		float timestamp;
};

/**
 * 
 */
UCLASS()
class REWIND_API ARewindGameState : public AGameState
{
	GENERATED_BODY()
	
	//Constructor
	ARewindGameState(const FObjectInitializer& ObjectInitialiser);

	UPROPERTY()
		TArray<FLocationHistory> arrLocationHistory;

	virtual void BeginPlay() override;
	virtual void Tick(float deltaSeconds) override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite,/* Replicated, */Category = GameTime)
		float currentGameTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = GameTime)
		bool bGamePaused = false;

	UPROPERTY()
		//Array of actors whos phyics has been suspended because the game has been pseudo-paused
		TArray<UPrimitiveComponent*> pausedPhysicsActors;

	UPROPERTY()
		TArray<FLocationHistory> ActorCurrentLocations;

	//Array of the times when actors are created
	UPROPERTY()
		TArray<FActorLifespans> ActorCreationTimes;
	//Array of the times when actors are destroyed
	UPROPERTY()
		TArray<FActorLifespans> ActorDestructionTime;

	/*UFUNCTION()
	void appendLocationHistory(float timestamp, AActor* actor, float lifespan, FTransform actorTransform, FVector velocity);*/

	UFUNCTION(BlueprintCallable, Category = LocationHistory)
		void createLocationHistory(ARewindableStaticMeshActor* actor/*, bool checkIfChanged = true, bool updateLastChanged = true*/);

	UFUNCTION(BlueprintCallable, Category = LocationHistory)
		void revertLocationHistory(float timestamp);
};
	
