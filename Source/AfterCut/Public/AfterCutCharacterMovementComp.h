#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AfterCut/Public/PlayerSettings.h"

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
	virtual float GetMaxBrakingDeceleration() const override;
	virtual float GetMaxSpeed() const override;
	virtual float GetGroundFriction() const;
	virtual float GetMaxAcceleration() const override;

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
	virtual void MoveAlongFloor(const FVector& InVelocity, float DeltaSeconds, FStepDownResult* OutStepDownResult = NULL) override;
	virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration) override;
	virtual bool ApplyRequestedMove(float DeltaTime, float MaxAccel, float MaxSpeed, float Friction, float BrakingDeceleration, FVector& OutAcceleration, float& OutRequestedSpeed) override;

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
	void PhysDash(float deltaTime, int32 Iterations);

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
