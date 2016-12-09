// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "Rewind.h"
#include "RewindHUD.h"
#include "Engine/Canvas.h"
#include "TextureResource.h"
#include "CanvasItem.h"

#include "Public/RewindGameState.h"
#include "RewindCharacter.h"

ARewindHUD::ARewindHUD()
{
	// Set the crosshair texture
	static ConstructorHelpers::FObjectFinder<UTexture2D> CrosshiarTexObj(TEXT("/Game/FirstPerson/Textures/FirstPersonCrosshair"));
	CrosshairTex = CrosshiarTexObj.Object;

	//Create the hud font
	static ConstructorHelpers::FObjectFinder<UFont>HUDFontObj(TEXT("/Engine/EngineFonts/RobotoDistanceField"));
	HUDFont = HUDFontObj.Object;
}


void ARewindHUD::DrawHUD()
{
	Super::DrawHUD();

	//Get and store the size of the screen
	const FVector2D HUDSize(Canvas->SizeX, Canvas->SizeY);

	// Draw very simple crosshair

	// find center of the Canvas
	const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);

	// offset by half the texture's dimensions so that the center of the texture aligns with the center of the Canvas
	const FVector2D CrosshairDrawPosition( (Center.X - (CrosshairTex->GetSurfaceWidth() * 0.5)),
										   (Center.Y - (CrosshairTex->GetSurfaceHeight() * 0.5f)) );

	// draw the crosshair
	FCanvasTileItem TileItem( CrosshairDrawPosition, CrosshairTex->Resource, FLinearColor::White);
	TileItem.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem( TileItem );


	//Draw the current in game time in the top right
	//Get the current game state
	ARewindGameState* gameState = Cast<ARewindGameState>(UGameplayStatics::GetGameState(GetWorld()));
	//Format the current game time
	FString gameTimeString = FString::Printf(TEXT("Current Game Time: %.1f"), gameState->currentGameTime);
	//Calculate the size of the text
	FVector2D timeStringSize;
	GetTextSize(gameTimeString, timeStringSize.X, timeStringSize.Y, HUDFont);
	//Draw the text
	DrawText(gameTimeString, FColor::Green, HUDSize.X - timeStringSize.X - 10, 10, HUDFont);

	//Draw watermark
	DrawText(FString::Printf(L"Rewind Tech Demo - Preview Build %03d - Copyright (C) 2016 Jack Wickham\nUnauthorised redistribution is prohibited", BUILDNUMBER), FColor::Magenta, 10, 10, HUDFont, 0.5f);

	//If the player is holding reset, tell them that they are
	//Get the player character
	ARewindCharacter* playerCharacter = Cast<ARewindCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (playerCharacter != NULL && playerCharacter->IsValidLowLevel() && playerCharacter->resetIsPressed) {
		APlayerController* playerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		TArray<FInputActionKeyMapping> resetKeyBindings = playerController->PlayerInput->GetKeysForAction(FName(L"Reset"));
		FString keyMessage;
		int16 numKeyBindings = resetKeyBindings.Num();
		if (numKeyBindings < 1) return;
		else if (numKeyBindings == 1) {
			keyMessage = resetKeyBindings[0].Key.GetDisplayName().ToString();
		}
		else if (numKeyBindings == 2){
			keyMessage = FString::Printf(L"%s or %s", *resetKeyBindings[0].Key.GetDisplayName().ToString(), *resetKeyBindings[1].Key.GetDisplayName().ToString());
		}
		else {
			keyMessage = resetKeyBindings[0].Key.GetDisplayName().ToString();
			for (int16 i = 1; i < numKeyBindings - 1; i++) {
				keyMessage = FString::Printf(L"%s, %s", *keyMessage, *resetKeyBindings[i].Key.GetDisplayName().ToString());
			}
			keyMessage = FString::Printf(L"%s or %s", *keyMessage, *resetKeyBindings[numKeyBindings - 1].Key.GetDisplayName().ToString());
		}
		FString resetMessage = FString::Printf(L"Hold %s to reset", *keyMessage);
		FVector2D resetMsgSize;
		GetTextSize(resetMessage, resetMsgSize.X, resetMsgSize.Y, HUDFont);
		DrawText(resetMessage, FColor::Red, (HUDSize.X - resetMsgSize.X) * 0.5f, HUDSize.Y - 50, HUDFont);
	}
}

