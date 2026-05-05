// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AfterCutCMC.generated.h"

/**
 * 
 */
UCLASS()
class AFTERCUT_API UAfterCutCMC : public UCharacterMovementComponent
{
	GENERATED_BODY()
	

	class FSavedMove_AfterCut : public FSavedMove_Character {
		typedef FSavedMove_Character Super; // used to call the base function from within the overide functions
		
		uint8 Saved_bWantsToSprint : 1;
		uint8 Saved_bWantsToDash : 1;

		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;

		virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData) override;
		virtual void PrepMoveFor(ACharacter* C) override;
	};

	class FNetworkPredictionData_Client_AfterCut : public FNetworkPredictionData_Client_Character {
	public:
		typedef FNetworkPredictionData_Client_Character Super;
		
		FNetworkPredictionData_Client_AfterCut(const UCharacterMovementComponent& ClientMovement);

		virtual FSavedMovePtr AllocateNewMove() override;
	};

	bool Safe_bWantsToSprint;
	bool Safe_bWantsToDash;

public:
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
protected:
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;

public:
	UAfterCutCMC();

};
