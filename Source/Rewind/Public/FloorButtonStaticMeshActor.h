// Copyright (c) 2015 Jack Wickham - All Rights Reserved

#pragma once

#include "Engine/StaticMeshActor.h"
#include "FloorButtonStaticMeshActor.generated.h"

/**
 * 
 */
UCLASS()
class REWIND_API AFloorButtonStaticMeshActor : public AActor//AStaticMeshActor
{
	GENERATED_BODY()

	//Constructor
	AFloorButtonStaticMeshActor(const FObjectInitializer& objectInitializer);

	//The number of objects currently touching the button. Press events are triggered when it stops being 0, and release when it changes to 0
	int32 touchingObjects = 0;
	
public:
	//Called when an object touches the button
	UFUNCTION(BlueprintCallable, Category = Interaction)
		void onRealOverlapStart(class AActor* otherActor, class UPrimitiveComponent* otherComponent, int32 otherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	//Called when an object stops touching the button
	UFUNCTION(BlueprintCallable, Category = Interaction)
		void onRealOverlapEnd(class AActor* otherActor, class UPrimitiveComponent* otherComponent, int32 otherBodyIndex);


	//Called when an object touches the button
	UFUNCTION(BlueprintCallable, Category = Interaction)
		void onCapsuleOverlapStart(class AActor* otherActor, class UPrimitiveComponent* otherComponent, int32 otherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	//Called when an object stops touching the button
	UFUNCTION(BlueprintCallable, Category = Interaction)
		void onCapsuleOverlapEnd(class AActor* otherActor, class UPrimitiveComponent* otherComponent, int32 otherBodyIndex);

	//Called when an object touches the button
	UFUNCTION(BlueprintCallable, Category = Interaction)
		void onBoxOverlapStart(class AActor* otherActor, class UPrimitiveComponent* otherComponent, int32 otherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	//Called when an object stops touching the button
	UFUNCTION(BlueprintCallable, Category = Interaction)
		void onBoxOverlapEnd(class AActor* otherActor, class UPrimitiveComponent* otherComponent, int32 otherBodyIndex);

private:
	TArray<AActor*> actorsInCapsule;
	TArray<AActor*> actorsInBox;

protected:
	//Event implemented in blueprint to handle button press
	UFUNCTION(BlueprintImplementableEvent, Category = Interaction)
		void onButtonPress();
	//Event implemented in blueprint to handle button release
	UFUNCTION(BlueprintImplementableEvent, Category = Interaction)
		void onButtonRelease();

public:
	//Store the visible part of the button
	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* buttonFrameComponent;
	//Store the interactive component
	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* buttonInnerComponent;

	UPROPERTY(EditAnywhere)
		UCapsuleComponent* hitboxComponent;
	UPROPERTY(EditAnywhere)
		UBoxComponent* hitboxBoxComponent;
};
