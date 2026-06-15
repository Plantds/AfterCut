#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "AfterCut.h"
#include "AfterCutCharacterMovementComp.generated.h"

UENUM(BlueprintType)
enum ECustomMovementMode {
	CMOVE_None		 UMETA(Hidden),
	CMOVE_Slide		 UMETA(DisplayName = "Slide"),
	CMOVE_MAX		 UMETA(Hidden)
};

UCLASS()
class AFTERCUT_API UAfterCutCharacterMovementComp : public UCharacterMovementComponent
{
	GENERATED_BODY()

	class FSavedMove_AC : public FSavedMove_Character
	{
		typedef FSavedMove_Character Super;

		// Flags
		uint8 Saved_bWantsToSprint : 1;
		uint8 Saved_bPreWantsToCrouch : 1;

	public:
		FSavedMove_AC();

		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;
		virtual void PrepMoveFor(ACharacter* C) override;
	};

	class FNetworkPredictionData_Client_AC : public FNetworkPredictionData_Client_Character {
	public:
		typedef FNetworkPredictionData_Client_Character Super;

		FNetworkPredictionData_Client_AC(const UCharacterMovementComponent& ClientMovement);

		virtual FSavedMovePtr AllocateNewMove() override;
	};

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
	UPROPERTY(EditAnywhere, Category = "AC|Slide") float SlideFriction = 1.3f;


	bool Safe_bWantsToSprint;
	bool Safe_bPrevWantsToCrouch;

	// Transient
	UPROPERTY(Transient) AAfterCutCharacter* AfterCutCharacterOwner;

public:
	UAfterCutCharacterMovementComp();

protected:
	virtual void InitializeComponent() override;

public:

	virtual float GetMaxSpeed() const override;

public:
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	virtual bool IsMovingOnGround() const override;
	virtual bool CanCrouchInCurrentState() const override;

protected:
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;

	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;

	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;

	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

/// <summary>
/// MOVEMENT
/// </summary>

	//Slide
private:
	void EnterSlide(FHitResult& Floor);
	void ExitSlide();
	void PhysSlide(float deltaTime, int32 Iterations);
	bool GetSlideSurface(FHitResult& Hit) const;
	bool CanSlide(FHitResult& Hit) const;

public:

	/// <summary>
	///  Blue print callable or accesable functions
	/// </summary>

	UFUNCTION(BlueprintCallable) void SprintPressed();
	UFUNCTION(BlueprintCallable) void SprintReleased();

	UFUNCTION(BlueprintCallable) void CrouchPressed();
	UFUNCTION(BlueprintCallable) void CrouchReleased();

	UFUNCTION(BlueprintPure) bool IsMovementMode(EMovementMode InMovementMode) const;

	UFUNCTION(BlueprintPure) bool IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const;
};
