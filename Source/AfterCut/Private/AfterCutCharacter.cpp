#include "AfterCutCharacter.h"

AAfterCutCharacter::AAfterCutCharacter(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer.SetDefaultSubobjectClass<UAfterCutCharacterMovementComp>(ACharacter::CharacterMovementComponentName))
{
	AfterCutCharacterMovementComponent = Cast<UAfterCutCharacterMovementComp>(GetCharacterMovement());
	AfterCutCharacterMovementComponent->SetIsReplicated(true);
}