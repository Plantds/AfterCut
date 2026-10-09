#include "AfterCutCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"

AAfterCutCharacter::AAfterCutCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UAfterCutCharacterMovementComp>(ACharacter::CharacterMovementComponentName))
{
	AfterCutCharacterMovementComponent = Cast<UAfterCutCharacterMovementComp>(GetCharacterMovement());
	AfterCutCharacterMovementComponent->SetIsReplicated(true);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 0.0f;
	CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f,  PlayerSettings != nullptr ? PlayerSettings->CameraBoomHeight : 0.0f));
	CameraBoom->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = true;
}

void AAfterCutCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());

	Subsystem->ClearAllMappings();
	Subsystem->AddMappingContext(PlayerSettings->InputMapping, 0);

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	// Movement
	Input->BindAction(PlayerSettings->LookAction, ETriggerEvent::Triggered, this, &AAfterCutCharacter::ACLook);
	Input->BindAction(PlayerSettings->MoveAction, ETriggerEvent::Triggered, this, &AAfterCutCharacter::ACMove);
	Input->BindAction(PlayerSettings->MoveAction, ETriggerEvent::Completed, this, &AAfterCutCharacter::ACMoveEnded);
	
	Input->BindAction(PlayerSettings->JumpAction, ETriggerEvent::Started, this, &AAfterCutCharacter::ACJumpStarted);
	Input->BindAction(PlayerSettings->JumpAction, ETriggerEvent::Completed, this, &AAfterCutCharacter::ACJumpEnded);

	Input->BindAction(PlayerSettings->CrouchAction, ETriggerEvent::Started, this, &AAfterCutCharacter::ACCrouchStarted);
	Input->BindAction(PlayerSettings->CrouchAction, ETriggerEvent::Completed, this, &AAfterCutCharacter::ACCrouchEnded);

	Input->BindAction(PlayerSettings->SprintAction, ETriggerEvent::Started, this, &AAfterCutCharacter::ACSprintStarted);
	Input->BindAction(PlayerSettings->SprintAction, ETriggerEvent::Completed, this, &AAfterCutCharacter::ACSprintEnded);

	Input->BindAction(PlayerSettings->DashAction, ETriggerEvent::Triggered, this, &AAfterCutCharacter::ACDash);

}

void AAfterCutCharacter::ACLook(const FInputActionValue& Value)
{
	FVector2D LookVector2D = Value.Get<FVector2D>();
	AddControllerYawInput(LookVector2D.X * GetPlayerSettings()->Sensitivity);
	AddControllerPitchInput(LookVector2D.Y * GetPlayerSettings()->Sensitivity * -1.0f);
}

void AAfterCutCharacter::ACMove(const FInputActionValue& Value)
{
	FVector2D Move = Value.Get<FVector2D>();
	
	MoveVector2D = Move;
	
	AddMovementInput(GetCapsuleComponent()->GetRightVector(), Move.X);
	AddMovementInput(GetCapsuleComponent()->GetForwardVector(), Move.Y);
}

void AAfterCutCharacter::ACMoveEnded(const FInputActionValue& Value)
{
	FVector2D Move = Value.Get<FVector2D>();
	
	MoveVector2D = Move;
}

void AAfterCutCharacter::ACJumpStarted(const FInputActionValue& Value)
{
	Jump();
}

void AAfterCutCharacter::ACJumpEnded(const FInputActionValue& Value)
{
	StopJumping();
}

void AAfterCutCharacter::ACCrouchStarted(const FInputActionValue& Value)
{
	AfterCutCharacterMovementComponent->CrouchPressed();
}

void AAfterCutCharacter::ACCrouchEnded(const FInputActionValue& Value)
{
	AfterCutCharacterMovementComponent->CrouchReleased();
}

void AAfterCutCharacter::ACSprintStarted(const FInputActionValue& Value)
{
	AfterCutCharacterMovementComponent->SprintPressed();
}

void AAfterCutCharacter::ACSprintEnded(const FInputActionValue& Value)
{
	AfterCutCharacterMovementComponent->SprintReleased();
}

void AAfterCutCharacter::ACDash(const FInputActionValue& Value)
{
	AfterCutCharacterMovementComponent->DashPressed();
}

void AAfterCutCharacter::ACSlash(const FInputActionValue& Value)
{
}

void AAfterCutCharacter::ACSheath(const FInputActionValue& Value)
{
}

void AAfterCutCharacter::ACParry(const FInputActionValue& Value)
{
}

void AAfterCutCharacter::ACInteract(const FInputActionValue& Value)
{
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

