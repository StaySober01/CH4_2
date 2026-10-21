// Copyright Epic Games, Inc. All Rights Reserved.

#include "CH4_2WheelFront.h"
#include "UObject/ConstructorHelpers.h"

UCH4_2WheelFront::UCH4_2WheelFront()
{
	AxleType = EAxleType::Front;
	bAffectedBySteering = true;
	MaxSteerAngle = 40.f;
}