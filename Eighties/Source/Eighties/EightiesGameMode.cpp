// Copyright Epic Games, Inc. All Rights Reserved.

#include "EightiesGameMode.h"
#include "EightiesPlayerController.h"

AEightiesGameMode::AEightiesGameMode()
{
	PlayerControllerClass = AEightiesPlayerController::StaticClass();
}
