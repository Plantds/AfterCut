#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "EnhancedInput/Public/InputMappingContext.h"
#include "EnhancedInput/Public/EnhancedInputComponent.h"
#include "EnhancedInput/Public/InputAction.h"
#include "EnhancedInput/Public/EnhancedInputSubsystems.h"

#include  "AfterCut/Public/PlayerSettings.h"
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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Settings) UPlayerSettings* PlayerSettings = nullptr;
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true")) USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true")) UCameraComponent* Camera;

	FVector2D MoveVector2D = FVector2D(0.0f, 0.0f);
	
public:
	AAfterCutCharacter(const FObjectInitializer& ObjectInitializer);
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Movement
	UFUNCTION() void ACLook(const FInputActionValue& Value);
	UFUNCTION() void ACMove(const FInputActionValue& Value);
	UFUNCTION() void ACMoveEnded(const FInputActionValue& Value);
	UFUNCTION() void ACJumpStarted(const FInputActionValue& Value);
	UFUNCTION() void ACJumpEnded(const FInputActionValue& Value);
	UFUNCTION() void ACCrouchStarted(const FInputActionValue& Value);
	UFUNCTION() void ACCrouchEnded(const FInputActionValue& Value);
	UFUNCTION() void ACSprintStarted(const FInputActionValue& Value);
	UFUNCTION() void ACSprintEnded(const FInputActionValue& Value);
	UFUNCTION() void ACDash(const FInputActionValue& Value);

	// Combat
	UFUNCTION() void ACSlash(const FInputActionValue& Value);
	UFUNCTION() void ACSheath(const FInputActionValue& Value);
	UFUNCTION() void ACParry(const FInputActionValue& Value);

	// Interact
	UFUNCTION() void ACInteract(const FInputActionValue& Value);
	
	
public:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetCamera() const { return Camera; }
	UFUNCTION(BlueprintPure) FORCEINLINE UAfterCutCharacterMovementComp* GetAfterCutCharacterMovement() const { return AfterCutCharacterMovementComponent; }
	UFUNCTION(BlueprintPure) FORCEINLINE UPlayerSettings* GetPlayerSettings() const { return PlayerSettings; }
	UFUNCTION(BlueprintPure) FORCEINLINE FVector2D GetMovementVector2D() { return MoveVector2D; }
	FCollisionQueryParams GetIgnoreParams() const;
	TArray<AActor*> GetIgnoreActors() const;
};
