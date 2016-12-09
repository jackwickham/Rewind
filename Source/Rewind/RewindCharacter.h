// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Character.h"

#include "TimerManager.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Public/ButtonActivationInterface.h"

#include "RewindCharacter.generated.h"

class UInputComponent;

UCLASS(config=Game)
class ARewindCharacter : public ACharacter, public IButtonActivationInterface
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	class USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;


	//Start a rewind
	UFUNCTION()
		void triggerRewind();
	//End rewind
	UFUNCTION()
		void triggerEndRewind();

	//Stores a timer handle which unpauses the game 0.5s after the rewind ends
	FTimerHandle timerHandle;
public:
	ARewindCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class ARewindProjectile> ProjectileClass;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	class USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class UAnimMontage* FireAnimation;

	//True if the game is currently rewinding, false if not
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = Rewind)
		bool bRewinding = false;

	//Run game resume
	void triggerGameResume();

protected:
	
	/** Fires a projectile. */
	void OnFire();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	struct TouchData
	{
		TouchData() { bIsPressed = false;Location=FVector::ZeroVector;}
		bool bIsPressed;
		ETouchIndex::Type FingerIndex;
		FVector Location;
		bool bMoved;
	};
	void BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location);
	TouchData	TouchItem;


	/*On jump*/
	void onJump();

	/*On Interaction start*/
	void startInteract();
	void endInteract();
	
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	/* 
	 * Configures input for touchscreen devices if there is a valid touch interface for doing so 
	 *
	 * @param	InputComponent	The input component pointer to bind controls to
	 * @returns true if touch controls were enabled.
	 */
	bool EnableTouchscreenMovement(UInputComponent* InputComponent);

public:
	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }


	/**
	* A port of the pickup mechanics that are present in the content examples demo (in blueprints)
	*/
public:
	//Whether we are currently holding an object
	bool holdingObject = false;
	//Whether we are currently interacting
	bool interacting = false;
	//Maximum interaction range
	float maxInteractionRange = 300;

	//The distance that there should be between the player and the object that they are carrying
	float objectHandleDistance = 200;

	//Drop the currently held object, but don't return anything
	UFUNCTION()
		void tryDropObject();

private:
	UPhysicsHandleComponent* physComponent;

	UPrimitiveComponent* pickedUpComponent;

	FRotator pickedUpObjectRotationOffset;

	//Pick up an object as a result of a hit scan
	void pickUpObjectFromTraceHit(FHitResult traceResult);

	//Update the location of the object that we've picked up
	void updatePickedUpObjectLocation();
	//Drop the currently held object
	void dropObject();

	//Whether we have set up the listener to the game mode's on pause event
	bool onPauseListenerInitialised = false;

	//On tick
	virtual void Tick(float deltaSeconds) override;

public:
	/*Reset Level*/
	void resetPressed();
	void resetReleased();

	UPROPERTY(EditDefaultsOnly)
	float targetResetPressedTime = 1.5f;

	bool resetIsPressed = false;
	float resetPressedTime = 0;

protected:
	//On destroy
	//virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Destroyed() override;

	/*void pausePressed();

private:
	// Reference UMG Asset in the Editor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
		TSubclassOf<class UUserWidget> wPauseMenu;
	
	// Variable to hold the widget After Creating it.
	UUserWidget* PauseMenu;*/

};

