// Copyright Epic Games, Inc. All Rights Reserved.

#include "hackdGameMode.h"
#include "hackdCharacter.h"
#include "UObject/ConstructorHelpers.h"

AhackdGameMode::AhackdGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
