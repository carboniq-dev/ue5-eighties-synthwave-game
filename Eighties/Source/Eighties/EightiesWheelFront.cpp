// Copyright Epic Games, Inc. All Rights Reserved.

#include "EightiesWheelFront.h"
#include "UObject/ConstructorHelpers.h"

UEightiesWheelFront::UEightiesWheelFront()
{
	AxleType = EAxleType::Front;
	bAffectedBySteering = true;
	MaxSteerAngle = 40.f;
}