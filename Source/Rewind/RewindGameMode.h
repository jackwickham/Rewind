// Copyright 2015 Jack Wickham, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/GameMode.h"

#include "Public/RewindGameState.h"

#include "RewindGameMode.generated.h"

//Declare the events that can be triggered
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGamePauseEvent); //When game is paused
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameResumeEvent, float, time); //When game resumes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameRewindEvent, float, deltaTime); //Called each frame that the game is being rewound

UCLASS(minimalapi)
class ARewindGameMode : public AGameMode
{
	GENERATED_BODY()
	
	virtual void Tick(float deltaSeconds) override;

	UPROPERTY()
		float lastTime = 0;

	//How much faster the game should run when being rewound
	UPROPERTY(EditDefaultsOnly)
		float rewindDeltaTimeMultiplier = 3;

	bool bIsResetting = false;

public:
	ARewindGameMode();

	UFUNCTION(BlueprintCallable, category = Pause)
		void pauseGame();
	UFUNCTION(BlueprintCallable, category = Pause)
		void resumeGame();

	//The maximum amount of time that the game will remain paused after a rewind
	UPROPERTY(EditDefaultsOnly)
		float maxRewindPauseTime = 0.001f; //0.5f;

	UPROPERTY(BlueprintAssignable, Category = Rewind)
		FOnGamePauseEvent onPause;
	UPROPERTY(BlueprintAssignable, Category = Rewind)
		FOnGameResumeEvent onResume;
	UPROPERTY(BlueprintAssignable, Category = Rewind)
		FOnGameRewindEvent onRewind;

	UFUNCTION()
		void reloadLevel();
};



