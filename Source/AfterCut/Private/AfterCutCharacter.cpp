#include "AfterCutCharacter.h"

AAfterCutCharacter::AAfterCutCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UAfterCutCharacterMovementComp>(ACharacter::CharacterMovementComponentName))
{
	AfterCutCharacterMovementComponent = Cast<UAfterCutCharacterMovementComp>(GetCharacterMovement());
	AfterCutCharacterMovementComponent->SetIsReplicated(true);
}

FCollisionQueryParams AAfterCutCharacter::GetIgnoreParams() const
{
	FCollisionQueryParams Parmas;

	TArray<AActor*> CharacterChildern;
	GetAllChildActors(CharacterChildern);
	Parmas.AddIgnoredActors(CharacterChildern);
	Parmas.AddIgnoredActor(this);

	return Parmas;
}

TArray<AActor*> AAfterCutCharacter::GetIgnoreActors() const
{
	TArray<AActor*> actors;
	GetAllChildActors(actors);
	actors.Add(this->GetOwner());
	return actors;
}

