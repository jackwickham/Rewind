// Copyright (c) 2015 Jack Wickham - All Rights Reserved

#include "Rewind.h"
#include "Public/InteractiveStaticMeshActor.h"


void AInteractiveStaticMeshActor::onKeyPress() {
	if (enabled) { //TODO check interface
		onInteract();
		if (triggerOnce) enabled = false;
	}
}

void AInteractiveStaticMeshActor::onKeyRelease() {
	onEndInteract();
}


