// Copyright Epic Games, Inc. All Rights Reserved.

#include "EightiesWheelRear.h"
#include "UObject/ConstructorHelpers.h"

UEightiesWheelRear::UEightiesWheelRear()
{
	AxleType = EAxleType::Rear;
	bAffectedByHandbrake = true;
	bAffectedByEngine = true;
}