// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "Rewind.h"
#include "RewindGameMode.h"
#include "RewindHUD.h"
#include "RewindCharacter.h"

#include "Public/RewindableStaticMeshActor.h"

ARewindGameMode::ARewindGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = ARewindHUD::StaticClass();

	//Set the game state
	GameStateClass = ARewindGameState::StaticClass();
}

//On each tick, rewind if paused or save new actor locations if not
void ARewindGameMode::Tick(float deltaSeconds) {
	Super::Tick(deltaSeconds);

	ARewindGameState* gameState = Cast<ARewindGameState>(GameState);

	if (gameState != NULL && !gameState->bGamePaused){
		//Update the actor location array
		for (TActorIterator<ARewindableStaticMeshActor> ActorItr(GetWorld()); ActorItr; ++ActorItr){
			//Check if the actor is movable
			/*if (ActorItr->IsRootComponentMovable()) {
			gameState->createLocationHistory(*ActorItr);
			}*/
			if (ActorItr->rewinds) {
				gameState->createLocationHistory(*ActorItr);
			}
		}
	}

	//Rewind if necessary

	//Get and store the current character reference
	ARewindCharacter* character = Cast<ARewindCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	if (IsValid(character) && character->bRewinding) {
		//Game is currently rewinding
		//Pause the game
		pauseGame();
		//Revert the object locations to where they were (number of seconds since last frame * 3) seconds ago
		gameState->revertLocationHistory(gameState->currentGameTime - (deltaSeconds * rewindDeltaTimeMultiplier));

		//Send the event to the blueprints
		onRewind.Broadcast(deltaSeconds * rewindDeltaTimeMultiplier);
	}
}

//Pseudo-pauses the game - disables physics on all actors to allow for rewinding
void ARewindGameMode::pauseGame() {
	ARewindGameState* gameState = Cast<ARewindGameState>(GameState);

	if (gameState->bGamePaused) return; //Don't need to do anything if already paused

	gameState->bGamePaused = true;

	GetWorld()->bShouldSimulatePhysics = false;

	onPause.Broadcast();
}

//Resumes the game
void ARewindGameMode::resumeGame() {
	ARewindGameState* gameState = Cast<ARewindGameState>(GameState);

	if (!gameState->bGamePaused) return; //Don't need to do anything if already unpaused

	gameState->bGamePaused = false;

	GetWorld()->bShouldSimulatePhysics = true;

	/*for (UPrimitiveComponent* primitive : gameState->pausedPhysicsActors){
	//Enable physics on actors that had it disabled initially
	primitive->SetSimulatePhysics(true);
	primitive->WakeAllRigidBodies();
	}
	gameState->pausedPhysicsActors.Empty();*/

	//this->onResume(gameState->currentGameTime);
	onResume.Broadcast(gameState->currentGameTime);
}

//Reloads the level (when you reset/die/etc)
void ARewindGameMode::reloadLevel() {
	if (!bIsResetting) {
		bIsResetting = true;
		FString levelName = GetWorld()->GetName();
		UGameplayStatics::OpenLevel(GetWorld(), FName(*levelName), false);
	}
}