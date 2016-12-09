// Fill out your copyright notice in the Description page of Project Settings.

#include "Rewind.h"
#include "Public/CarryableStaticMeshActor.h"


ACarryableStaticMeshActor::ACarryableStaticMeshActor() : Super(){
	UStaticMeshComponent* sm = GetStaticMeshComponent();
	sm->bGenerateOverlapEvents = true;
	sm->SetSimulatePhysics(true);
	sm->SetMobility(EComponentMobility::Movable);
}