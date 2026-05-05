// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/Character.h"
#include "AfterCutCMC.h"

UAfterCutCMC::UAfterCutCMC()
{

}

bool UAfterCutCMC::FSavedMove_AfterCut::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	FSavedMove_AfterCut* NewAfterCutMove = static_cast<FSavedMove_AfterCut*>(NewMove.Get());

	if (Saved_bWantsToSprint != NewAfterCutMove->Saved_bWantsToSprint)
		return false;

	if (Saved_bWantsToDash != NewAfterCutMove->Saved_bWantsToDash)
		return false;

	return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void UAfterCutCMC::FSavedMove_AfterCut::Clear()
{
	Super::Clear();

	Saved_bWantsToSprint	= 0;
	Saved_bWantsToDash		= 0;
}

uint8 UAfterCutCMC::FSavedMove_AfterCut::GetCompressedFlags() const
{
	int Result = Super::GetCompressedFlags();

	if (Saved_bWantsToSprint) Result	|= FLAG_Custom_0;
	if (Saved_bWantsToDash) Result		|= FLAG_Custom_1;
	
	return Result;
}

void UAfterCutCMC::FSavedMove_AfterCut::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(C, InDeltaTime, NewAccel,ClientData);

	UAfterCutCMC* CharacterMovement = Cast<UAfterCutCMC>(C->GetCharacterMovement());

	Saved_bWantsToSprint	= CharacterMovement->Safe_bWantsToSprint;
	Saved_bWantsToDash		= CharacterMovement->Safe_bWantsToDash;
}

void UAfterCutCMC::FSavedMove_AfterCut::PrepMoveFor(ACharacter* C)
{
	Super::PrepMoveFor(C);

	UAfterCutCMC* CharacterMovement = Cast<UAfterCutCMC>(C->GetCharacterMovement());

	CharacterMovement->Safe_bWantsToSprint = Saved_bWantsToSprint;
	CharacterMovement->Safe_bWantsToDash	= Saved_bWantsToDash;

}

UAfterCutCMC::FNetworkPredictionData_Client_AfterCut::FNetworkPredictionData_Client_AfterCut(const UCharacterMovementComponent& ClientMovement)
	:	Super(ClientMovement)
{
}

FSavedMovePtr UAfterCutCMC::FNetworkPredictionData_Client_AfterCut::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_AfterCut());
}

FNetworkPredictionData_Client* UAfterCutCMC::GetPredictionData_Client() const
{
	check(PawnOwner != nullptr);

	if (ClientPredictionData == nullptr) {
		UAfterCutCMC* MutableTHis = const_cast<UAfterCutCMC*>(this);
		
		MutableTHis->ClientPredictionData = new FNetworkPredictionData_Client_AfterCut(*this);
		MutableTHis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.0f;
		MutableTHis->ClientPredictionData->NoSmoothNetUpdateDist	= 140.0f;
	}

	return ClientPredictionData;
}

void UAfterCutCMC::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	Safe_bWantsToSprint = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
}
