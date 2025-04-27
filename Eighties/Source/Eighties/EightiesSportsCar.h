// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EightiesPawn.h"
#include "EightiesSportsCar.generated.h"

/**
 *  Sports car wheeled vehicle implementation
 */
UCLASS(abstract)
class EIGHTIES_API AEightiesSportsCar : public AEightiesPawn
{
	GENERATED_BODY()
	
public:

	AEightiesSportsCar();
};
