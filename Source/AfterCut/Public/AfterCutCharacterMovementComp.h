#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "AfterCut.h"
#include "AfterCutCharacterMovementComp.generated.h"

UENUM(BlueprintType)
enum ECustomMovementMode {
	CMOVE_None		 UMETA(Hidden),
	CMOVE_Slide		 UMETA(DisplayName = "Slide"),
	CMOVE_Dash		 UMETA(DisplayName = "Dash"),
	CMOVE_Wallrun	 UMETA(DisplayName = "Wallrun"),
	CMOVE_MAX		 UMETA(Hidden)
};

UCLASS()
class AFTERCUT_API UAfterCutCharacterMovementComp : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	/// <summary>
	///  Blueprint callable or accessable variables
	/// </summary>
	/// Sprint
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AC|Sprint") bool ToggleSprint = false;
	UPROPERTY(EditDefaultsOnly, Category = "AC|Sprint") float SprintSpeed = 1000.0f;

	/// Crouch
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AC|Crouch") bool ToggleCrouch = false;

	/// Slide
	UPROPERTY(EditAnywhere, Category = "AC|Slide") float MinSpeedToSlide = 350.0f;
	UPROPERTY(EditAnywhere, Category = "AC|Slide") float SlideSpeed = 1200.0f;
	UPROPERTY(EditAnywhere, Category = "AC|Slide") float SlideEnterImpulse = 500.0f;
	UPROPERTY(EditAnywhere, Category = "AC|Slide") float SlideGravityForce = 5000.0f;
	UPROPERTY(EditAnywhere, Category = "AC|Slide") float SlideFriction = 0.1f;

	/// Dash
	UPROPERTY(EditAnywhere, Category = "AC|Dash") float DashImpulse = 1500.0f;

	bool bWantsToSprint;
	bool bPrevWantsToCrouch;
	bool bWantsToDash;

	// Transient
	UPROPERTY(Transient) AAfterCutCharacter* AfterCutCharacterOwner;

public:
	UAfterCutCharacterMovementComp();

protected:
	virtual void InitializeComponent() override;

public:

	virtual float GetMaxSpeed() const override;

public:
	virtual bool IsMovingOnGround() const override;
	virtual bool CanCrouchInCurrentState() const override;

protected:
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;

	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;

	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

/// <summary>
/// MOVEMENT
/// </summary>

private:
	//Jump

	//Slide
	bool CanSlide(FHitResult& Hit) const;
	void EnterSlide();
	void ExitSlide();
	void PhysSlide(float deltaTime, int32 Iterations);

	bool GetSlideSurface(FHitResult& Hit) const;

	//Dash
	bool CanDash();
	void EnterDash();
	void ExitDash();
	void PhysDash();

	//Wallrun
	bool CanWallrun();
	void EnterWallrun();
	void ExitWallrun();
	void PhysWallrun();

	//Parry Jump -- can be an attack or a movement so need super special code bullshit


	void ResetCallVeriables();

public:

	/// <summary>
	///  Blue print callable or accesable functions
	/// </summary>

	UFUNCTION(BlueprintCallable) void SprintPressed();
	UFUNCTION(BlueprintCallable) void SprintReleased();

	UFUNCTION(BlueprintCallable) void CrouchPressed();
	UFUNCTION(BlueprintCallable) void CrouchReleased();

	UFUNCTION(BlueprintCallable) void DashPressed();
	UFUNCTION(BlueprintCallable) void DashReleased();


	UFUNCTION(BlueprintPure) bool IsMovementMode(EMovementMode InMovementMode) const;

	UFUNCTION(BlueprintPure) bool IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const;
};
