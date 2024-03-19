// Copyright Epic Games, Inc. All Rights Reserved.

#include "RevengersGameMode.h"
#include "Characters/RevengersCharacter.h"
#include "UObject/ConstructorHelpers.h"

ARevengersGameMode::ARevengersGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
