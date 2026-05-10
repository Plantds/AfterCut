#include "AfterCutCharacterMovementComp.h"

#include "AfterCutCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

/// FSavedMove_Character

UAfterCutCharacterMovementComp::FSavedMove_AC::FSavedMove_AC()
{
}

bool UAfterCutCharacterMovementComp::FSavedMove_AC::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	const FSavedMove_AC* NewACMove = static_cast<FSavedMove_AC*>(NewMove.Get());

	if (Saved_bWantsToSprint != NewACMove->Saved_bWantsToSprint)
		return false;

	return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void UAfterCutCharacterMovementComp::FSavedMove_AC::Clear()
{
	Super::Clear();

	Saved_bWantsToSprint = 0;
}

uint8 UAfterCutCharacterMovementComp::FSavedMove_AC::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (Saved_bWantsToSprint) Result |= FLAG_Custom_0;

	return Result;
}

void UAfterCutCharacterMovementComp::FSavedMove_AC::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	UAfterCutCharacterMovementComp* CharacterMovement = Cast<UAfterCutCharacterMovementComp>(C->GetCharacterMovement());

	Saved_bWantsToSprint = CharacterMovement->Safe_bWantsToSprint;
}

void UAfterCutCharacterMovementComp::FSavedMove_AC::PrepMoveFor(ACharacter* C)
{
	Super::PrepMoveFor(C);

	UAfterCutCharacterMovementComp* CharacterMovement = Cast<UAfterCutCharacterMovementComp>(C->GetCharacterMovement());

	CharacterMovement->Safe_bWantsToSprint = Saved_bWantsToSprint;
}

///  FNetworkPredictionData_Client_AC

UAfterCutCharacterMovementComp::FNetworkPredictionData_Client_AC::FNetworkPredictionData_Client_AC(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr UAfterCutCharacterMovementComp::FNetworkPredictionData_Client_AC::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_AC());
}

/// UAfterCutCharacterMovementComp
UAfterCutCharacterMovementComp::UAfterCutCharacterMovementComp()
{

}

float UAfterCutCharacterMovementComp::GetMaxSpeed() const
{
	if (IsMovementMode(MOVE_Walking) && Safe_bWantsToSprint && !IsCrouching()) return SprintSpeed;

	return Super::GetMaxSpeed();
}

FNetworkPredictionData_Client* UAfterCutCharacterMovementComp::GetPredictionData_Client() const
{
	check(PawnOwner != nullptr)

		if (ClientPredictionData == nullptr)
		{
			UAfterCutCharacterMovementComp* MutableThis = const_cast<UAfterCutCharacterMovementComp*>(this);

			MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_AC(*this);
			MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
			MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
		}

	return ClientPredictionData;
}

void UAfterCutCharacterMovementComp::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	Safe_bWantsToSprint = (Flags & FSavedMove_AC::FLAG_Custom_0) != 0;
}

void UAfterCutCharacterMovementComp::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
}

void UAfterCutCharacterMovementComp::SprintPressed()
{
	Safe_bWantsToSprint = true;
}

void UAfterCutCharacterMovementComp::SprintReleased()
{
	Safe_bWantsToSprint = false;
}

bool UAfterCutCharacterMovementComp::IsMovementMode(EMovementMode InMovementMode) const
{
	return InMovementMode == MovementMode;
}