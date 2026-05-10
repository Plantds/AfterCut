#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "AfterCut.h"
#include "AfterCutCharacterMovementComp.generated.h"

UCLASS()
class AFTERCUT_API UAfterCutCharacterMovementComp : public UCharacterMovementComponent
{
	GENERATED_BODY()

	class FSavedMove_AC : public FSavedMove_Character
	{
	public:
		typedef FSavedMove_Character Super;

		// Flags
		uint8 Saved_bWantsToSprint : 1;

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

	UPROPERTY(EditDefaultsOnly) float SprintSpeed = 1000.0f;

	bool Safe_bWantsToSprint;

public:
	UAfterCutCharacterMovementComp();

	virtual float GetMaxSpeed() const override;

public:
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

protected:
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;

	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;

public:
	UFUNCTION(BlueprintCallable) void SprintPressed();
	UFUNCTION(BlueprintCallable) void SprintReleased();



	UFUNCTION(BlueprintPure) bool IsMovementMode(EMovementMode InMovementMode) const;
};
