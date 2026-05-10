#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "AfterCutCharacterMovementComp.h"
#include "AfterCut.h"
#include "AfterCutCharacter.generated.h"

UCLASS(config = Game)
class AFTERCUT_API AAfterCutCharacter : public ACharacter
{
	GENERATED_BODY()

	typedef ACharacter Super;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement) UAfterCutCharacterMovementComp* AfterCutCharacterMovementComponent;

public:
	AAfterCutCharacter(const FObjectInitializer& ObjectInitializer);
};
