// Copyright Epic Games, Inc. All Rights Reserved.

#include "CH4_2WheelRear.h"
#include "UObject/ConstructorHelpers.h"

UCH4_2WheelRear::UCH4_2WheelRear()
{
	AxleType = EAxleType::Rear;
	bAffectedByHandbrake = true;
	bAffectedByEngine = true;
}