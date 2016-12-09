// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "Rewind.h"
#include "RewindCharacter.h"
#include "RewindProjectile.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/InputSettings.h"

#include "Public/CarryableStaticMeshActor.h"
#include "Public/InteractiveStaticMeshActor.h"

#include "RewindGameMode.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// ARewindCharacter

ARewindCharacter::ARewindCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->AttachParent = GetCapsuleComponent();
	FirstPersonCameraComponent->RelativeLocation = FVector(0, 0, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 30.0f, 10.0f);

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	Mesh1P->AttachParent = FirstPersonCameraComponent;
	Mesh1P->RelativeLocation = FVector(0.f, 0.f, -150.f);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P are set in the
	// derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	//Initialise the physics handle for grabbing other objects
	physComponent = CreateDefaultSubobject<UPhysicsHandleComponent>(FName(L"ActorCarryingHandle"));

	//Notify the engine that we can tick
	PrimaryActorTick.bCanEverTick = true;

}

//////////////////////////////////////////////////////////////////////////
// Input

void ARewindCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// set up gameplay key bindings
	check(InputComponent);

	InputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	InputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	
	//InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &ARewindCharacter::TouchStarted);
	if( EnableTouchscreenMovement(InputComponent) == false )
	{
		InputComponent->BindAction("Fire", IE_Pressed, this, &ARewindCharacter::OnFire);
	}
	
	InputComponent->BindAxis("MoveForward", this, &ARewindCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ARewindCharacter::MoveRight);
	
	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &ARewindCharacter::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &ARewindCharacter::LookUpAtRate);

	//Set up rewind
	InputComponent->BindAction("Rewind", IE_Pressed, this, &ARewindCharacter::triggerRewind);
	InputComponent->BindAction("Rewind", IE_Released, this, &ARewindCharacter::triggerEndRewind);

	InputComponent->BindAction("Interact", IE_Pressed, this, &ARewindCharacter::startInteract);
	InputComponent->BindAction("Interact", IE_Released, this, &ARewindCharacter::endInteract);

	InputComponent->BindAction("Reset", IE_Pressed, this, &ARewindCharacter::resetPressed);
	InputComponent->BindAction("Reset", IE_Released, this, &ARewindCharacter::resetReleased);

	//InputComponent->BindAction("Pause", IE_Pressed, this, &ARewindCharacter::pausePressed).bExecuteWhenPaused = true;
}

void ARewindCharacter::OnFire()
{
	//Resume physics simulation if paused
	triggerGameResume();
	//Drop any objects that we are holding
	tryDropObject();

	// try and fire a projectile
	if (ProjectileClass != NULL)
	{
		const FRotator SpawnRotation = GetControlRotation();
		// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
		const FVector SpawnLocation = GetActorLocation() + SpawnRotation.RotateVector(GunOffset);

		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			// spawn the projectile at the muzzle
			World->SpawnActor<ARewindProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
		}
	}

	// try and play the sound if specified
	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation(), 0.5f);
	}

	// try and play a firing animation if specified
	if(FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if(AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}

}

void ARewindCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if( TouchItem.bIsPressed == true )
	{
		return;
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void ARewindCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	if( ( FingerIndex == TouchItem.FingerIndex ) && (TouchItem.bMoved == false) )
	{
		OnFire();
	}
	TouchItem.bIsPressed = false;
}

void ARewindCharacter::TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if ((TouchItem.bIsPressed == true) && ( TouchItem.FingerIndex==FingerIndex))
	{
		if (TouchItem.bIsPressed)
		{
			if (GetWorld() != nullptr)
			{
				UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
				if (ViewportClient != nullptr)
				{
					FVector MoveDelta = Location - TouchItem.Location;
					FVector2D ScreenSize;
					ViewportClient->GetViewportSize(ScreenSize);
					FVector2D ScaledDelta = FVector2D( MoveDelta.X, MoveDelta.Y) / ScreenSize;									
					if (ScaledDelta.X != 0.0f)
					{
						TouchItem.bMoved = true;
						float Value = ScaledDelta.X * BaseTurnRate;
						AddControllerYawInput(Value);
					}
					if (ScaledDelta.Y != 0.0f)
					{
						TouchItem.bMoved = true;
						float Value = ScaledDelta.Y* BaseTurnRate;
						AddControllerPitchInput(Value);
					}
					TouchItem.Location = Location;
				}
				TouchItem.Location = Location;
			}
		}
	}
}

void ARewindCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		//Resume physics simulation if paused
		//triggerGameResume();

		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void ARewindCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		//Resume physics simulation if paused
		//triggerGameResume();

		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void ARewindCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ARewindCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

bool ARewindCharacter::EnableTouchscreenMovement(class UInputComponent* InputComponent)
{
	bool bResult = false;
	if(FPlatformMisc::GetUseVirtualJoysticks() || GetDefault<UInputSettings>()->bUseMouseForTouch )
	{
		bResult = true;
		InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &ARewindCharacter::BeginTouch);
		InputComponent->BindTouch(EInputEvent::IE_Released, this, &ARewindCharacter::EndTouch);
		InputComponent->BindTouch(EInputEvent::IE_Repeat, this, &ARewindCharacter::TouchUpdate);
	}
	return bResult;
}

//Rewind stuff
void ARewindCharacter::onJump() {
	//Resume physics simulation if paused
	//triggerGameResume();

	Jump();
}

//Call the rewind function in the GameMode
void ARewindCharacter::triggerRewind() {
	bRewinding = true;
	
	//If we're holding something that rewinds, stop
	if (holdingObject && Cast<ACarryableStaticMeshActor>(pickedUpComponent->GetOwner())->rewinds) {
		dropObject();
	}
}
void ARewindCharacter::triggerEndRewind() {
	if (bRewinding) {
		bRewinding = false;
		ARewindGameMode* gameMode = Cast<ARewindGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

		//Set a timer to unpause the game after 0.5s
		GetWorldTimerManager().SetTimer(timerHandle, this, &ARewindCharacter::triggerGameResume, gameMode->maxRewindPauseTime, false);
		int i = 0;
	}
}

void ARewindCharacter::triggerGameResume() {
	if (Cast<ARewindGameState>(UGameplayStatics::GetGameState(GetWorld()))->bGamePaused){
		bRewinding = false;

		//Stop the timer from executing and ending a different rewind
		GetWorldTimerManager().ClearTimer(timerHandle);

		Cast<ARewindGameMode>(UGameplayStatics::GetGameMode(GetWorld()))->resumeGame();
	}
}

void ARewindCharacter::startInteract() {
	triggerGameResume();

	/*If we're already holding something, drop it, and return if that was successful*/
	if (holdingObject) {
		dropObject();
		return;
	}

	/*Check whether there are any objects nearby to interact with*/
	APlayerCameraManager* playerCamera = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0); //Get the current player's camera manager
	FVector start = playerCamera->GetCameraLocation(); //Get the start point of the collision ray

	FVector actorDir = playerCamera->GetCameraRotation().Vector();
	actorDir.Normalize();
	FVector end = start + (actorDir * maxInteractionRange); //Get the end point of the collision ray
	//Init the trace - from https://answers.unrealengine.com/questions/3446/how-would-i-use-line-trace-in-c.html (and also referencing https://wiki.unrealengine.com/Trace_Functions )
	FCollisionQueryParams traceParams(FName(L"InteractTrace"), true, this);
	traceParams.bTraceComplex = true;
	//traceParams.bTraceAsyncScene = true;
	traceParams.bReturnPhysicalMaterial = false;
	//Init the hit data
	FHitResult traceResult(ForceInit);

	//Actually do the trace to find hits
	//DrawDebugLine(GetWorld(), start, end, FColor::Cyan, true, 10.0F, 0, 10.0F); //For debugging
	if (!GetWorld()->LineTraceSingleByChannel(
		traceResult,
		start,
		end,
		ECC_Pawn,
		traceParams
		)) return; //Look into changing this to a cone, or adding a cone check if it failed

	//We hit something!
	interacting = true;
	AActor* hit = traceResult.GetActor();

	if (!IsValid(hit)) return;

	//Find out what we need to do with the actor
	if (hit->IsA(AInteractiveStaticMeshActor::StaticClass())) {
		AInteractiveStaticMeshActor* iz = Cast<AInteractiveStaticMeshActor>(hit);
		iz->onKeyPress();
	}
	else if (hit->IsA(ACarryableStaticMeshActor::StaticClass())) {
		pickUpObjectFromTraceHit(traceResult);
	}
}
void ARewindCharacter::endInteract(){
	interacting = false;
}

void ARewindCharacter::pickUpObjectFromTraceHit(FHitResult traceResult) {
	ACarryableStaticMeshActor* obj = Cast<ACarryableStaticMeshActor>(traceResult.GetActor());
	UPrimitiveComponent* component = traceResult.GetComponent();

	physComponent->GrabComponent(component, traceResult.BoneName, traceResult.Location, true);

	obj->beingCarried = true;
	holdingObject = true;
	pickedUpComponent = component;

	/* Figure out how roated the actor that we've picked up, so we can rotate relative to that */
	FRotator objectRotation = component->GetOwner()->GetActorRotation(); //Get the picked up actor's rotation
	APlayerCameraManager* playerCamera = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0); //Get the current player's camera manager
	pickedUpObjectRotationOffset = objectRotation;
	pickedUpObjectRotationOffset.Yaw -= -playerCamera->GetCameraRotation().Yaw;

	if (!onPauseListenerInitialised) {
		//Listen for game pause notifications, and drop any objects when it happens
		AGameMode* gameMode = UGameplayStatics::GetGameMode(GetWorld());
		if (gameMode->IsA(ARewindGameMode::StaticClass())) {
			ARewindGameMode* myGameMode = Cast<ARewindGameMode>(gameMode);
			myGameMode->onPause.AddDynamic(this, &ARewindCharacter::tryDropObject);
		}
		onPauseListenerInitialised = true;
	}
}

void ARewindCharacter::tryDropObject(){
	if (holdingObject) {
		dropObject();
	}
}
void ARewindCharacter::dropObject() {
	physComponent->ReleaseComponent();
	holdingObject = false;
	Cast<ACarryableStaticMeshActor>(pickedUpComponent->GetOwner())->beingCarried = false;
	pickedUpComponent->WakeRigidBody();
	//pickedUpComponent = NULL;
}

void ARewindCharacter::updatePickedUpObjectLocation() {
	APlayerCameraManager* playerCamera = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0); //Get the current player's camera manager

	//Figure out their new rotation - only interested in the camera yaw (rotation about the vertical axis)
	FRotator cameraRotation = playerCamera->GetCameraRotation();
	FRotator newRotation = pickedUpObjectRotationOffset;
	newRotation.Yaw += cameraRotation.Yaw;

	//Figure out their new location
	FVector actorDir = cameraRotation.Vector(); //Get a vector in the direction that the player is currently facing
	actorDir.Normalize();
	FVector newLocation = playerCamera->GetCameraLocation() + (actorDir * objectHandleDistance); //Get the end point of the collision ray

	physComponent->SetTargetLocationAndRotation(newLocation, newRotation);
}

void ARewindCharacter::Tick(float deltaSeconds) {
	Super::Tick(deltaSeconds);

	if (holdingObject) {
		updatePickedUpObjectLocation();
	}

	if (resetIsPressed) {
		resetPressedTime += deltaSeconds;
		if (resetPressedTime >= targetResetPressedTime) {
			//Reset
			ARewindGameMode* gameMode = Cast<ARewindGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
			gameMode->reloadLevel();
		}
	}
}

//Restart the level
void ARewindCharacter::resetPressed(){
	resetIsPressed = true;
	resetPressedTime = 0;
}
void ARewindCharacter::resetReleased(){
	resetIsPressed = false;
}

void ARewindCharacter::Destroyed() {
	Cast<ARewindGameMode>(UGameplayStatics::GetGameMode(GetWorld()))->reloadLevel();
}