#include "AfterCutCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

AAfterCutCharacter::AAfterCutCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UAfterCutCharacterMovementComp>(ACharacter::CharacterMovementComponentName))
{
	AfterCutCharacterMovementComponent = Cast<UAfterCutCharacterMovementComp>(GetCharacterMovement());
	AfterCutCharacterMovementComponent->SetIsReplicated(true);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 0.0f;
	CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
	CameraBoom->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = true;
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

