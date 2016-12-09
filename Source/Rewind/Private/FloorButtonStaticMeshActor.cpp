// Copyright (c) 2015 Jack Wickham - All Rights Reserved

#include "Rewind.h"
#include "Public/FloorButtonStaticMeshActor.h"

#include "Public/ButtonActivationInterface.h"


AFloorButtonStaticMeshActor::AFloorButtonStaticMeshActor(const FObjectInitializer& objectInitializer) : Super(objectInitializer) {
	buttonFrameComponent = objectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, FName(L"ButtonFrameComponent1"));
	buttonFrameComponent->bVisible = true;
	RootComponent = buttonFrameComponent;

	buttonInnerComponent = objectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, FName(L"ButtonInnerComponent1"));
	FTransform innerComponentTransform = FTransform(FVector(0, 0, 20));
	innerComponentTransform.SetScale3D(FVector(0.6f, 0.6f, 1.0f));
	buttonInnerComponent->AddLocalTransform(innerComponentTransform);
	buttonInnerComponent->AttachParent = RootComponent;

	hitboxComponent = objectInitializer.CreateDefaultSubobject<UCapsuleComponent>(this, FName("HitboxComponent"));
	hitboxComponent->InitCapsuleSize(30.0f, 260.0f);
	FTransform hitboxComponentTransform = FTransform(FVector(0, 0, 60.0f));
	hitboxComponent->AddLocalTransform(hitboxComponentTransform);
	hitboxComponent->AttachParent = RootComponent;

	hitboxBoxComponent = objectInitializer.CreateDefaultSubobject<UBoxComponent>(this, FName("HitboxBoxComponent"));
	hitboxBoxComponent->SetBoxExtent(FVector(30.0f, 30.0f, 20.0f));
	hitboxBoxComponent->AddLocalOffset(FVector(0, 0, 60.0f));
	hitboxBoxComponent->AttachParent = RootComponent;

	/*buttonInnerComponent->OnComponentBeginOverlap.AddDynamic(this, &AFloorButtonStaticMeshActor::onOverlapStart);
	buttonInnerComponent->OnComponentEndOverlap.AddDynamic(this, &AFloorButtonStaticMeshActor::onOverlapEnd);
	hitboxComponent->OnComponentBeginOverlap.AddDynamic(this, &AFloorButtonStaticMeshActor::onOverlapStart);
	hitboxComponent->OnComponentEndOverlap.AddDynamic(this, &AFloorButtonStaticMeshActor::onOverlapEnd);*/
	hitboxComponent->OnComponentBeginOverlap.AddDynamic(this, &AFloorButtonStaticMeshActor::onCapsuleOverlapStart);
	hitboxComponent->OnComponentEndOverlap.AddDynamic(this, &AFloorButtonStaticMeshActor::onCapsuleOverlapEnd);
	hitboxBoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AFloorButtonStaticMeshActor::onBoxOverlapStart);
	hitboxBoxComponent->OnComponentEndOverlap.AddDynamic(this, &AFloorButtonStaticMeshActor::onBoxOverlapEnd);

	buttonInnerComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
}

//Actual overlap/end overlap events
void AFloorButtonStaticMeshActor::onRealOverlapStart(class AActor* otherActor, class UPrimitiveComponent* otherComponent, int32 otherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	if (otherActor == nullptr || otherActor == this || otherComponent == nullptr) {
		return;
	}
	//Check whether it implements the ButtonActivationInterface
	IButtonActivationInterface* Interface = Cast<IButtonActivationInterface>(otherActor);
	if (Interface) {
		if (touchingObjects++ == 0) {
			//buttonInnerComponent->AddLocalOffset(FVector(0, 0, -18));
			onButtonPress();
		}
	}
}

void AFloorButtonStaticMeshActor::onRealOverlapEnd(class AActor* otherActor, class UPrimitiveComponent* otherComponent, int32 otherBodyIndex) {
	if (otherActor == nullptr || otherActor == this || otherComponent == nullptr) {
		return;
	}
	//Check whether it implements the ButtonActivationInterface
	IButtonActivationInterface* Interface = Cast<IButtonActivationInterface>(otherActor);
	if (Interface) {
		if (--touchingObjects == 0) {
			//buttonInnerComponent->AddLocalOffset(FVector(0, 0, 18));
			onButtonRelease();
		}
	}

	
}

//Capsule component events
void AFloorButtonStaticMeshActor::onCapsuleOverlapStart(class AActor* otherActor, class UPrimitiveComponent* otherComponent, int32 otherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	if (otherActor == nullptr || otherActor == this || otherComponent == nullptr) {
		return;
	}
	//Add to list of actors in the capsule
	actorsInCapsule.Add(otherActor);
	//Check whether it's already in the box
	if (actorsInBox.Contains(otherActor)) {
		//And send the overlap event if so
		onRealOverlapStart(otherActor, otherComponent, otherBodyIndex, bFromSweep, SweepResult);
	}
}

void AFloorButtonStaticMeshActor::onCapsuleOverlapEnd(class AActor* otherActor, class UPrimitiveComponent* otherComponent, int32 otherBodyIndex) {
	if (otherActor == nullptr || otherActor == this || otherComponent == nullptr) {
		return;
	}
	//Remove from list of actors in the capsule
	actorsInCapsule.Remove(otherActor);
	//Check whether it was in the box
	if (actorsInBox.Contains(otherActor)) {
		//And send the overlap event if so
		onRealOverlapEnd(otherActor, otherComponent, otherBodyIndex);
	}
}

//Box component events
void AFloorButtonStaticMeshActor::onBoxOverlapStart(class AActor* otherActor, class UPrimitiveComponent* otherComponent, int32 otherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	if (otherActor == nullptr || otherActor == this || otherComponent == nullptr) {
		return;
	}
	//Add to list of actors in the capsule
	actorsInBox.Add(otherActor);
	//Check whether it's already in the box
	if (actorsInCapsule.Contains(otherActor)) {
		//And send the overlap event if so
		onRealOverlapStart(otherActor, otherComponent, otherBodyIndex, bFromSweep, SweepResult);
	}
}

void AFloorButtonStaticMeshActor::onBoxOverlapEnd(class AActor* otherActor, class UPrimitiveComponent* otherComponent, int32 otherBodyIndex) {
	if (otherActor == nullptr || otherActor == this || otherComponent == nullptr) {
		return;
	}
	//Remove from list of actors in the capsule
	actorsInBox.Remove(otherActor);
	//Check whether it was in the box
	if (actorsInCapsule.Contains(otherActor)) {
		//And send the overlap event if so
		onRealOverlapEnd(otherActor, otherComponent, otherBodyIndex);
	}
}
