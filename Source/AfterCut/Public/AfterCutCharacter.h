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
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true")) USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true")) UCameraComponent* Camera;

public:
	AAfterCutCharacter(const FObjectInitializer& ObjectInitializer);

public:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetCamera() const { return Camera; }
	UFUNCTION(BlueprintPure) FORCEINLINE UAfterCutCharacterMovementComp* GetAfterCutCharacterMovement() const { return AfterCutCharacterMovementComponent; }

	FCollisionQueryParams GetIgnoreParams() const;
	TArray<AActor*> GetIgnoreActors() const;
};
