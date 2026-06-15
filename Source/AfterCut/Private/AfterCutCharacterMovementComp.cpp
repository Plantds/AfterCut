#include "AfterCutCharacterMovementComp.h"

#include "AfterCutCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

// Helper Macros
#if 1
float MacroDuration = 2.f;
#define SLOG(x) GEngine->AddOnScreenDebugMessage(-1, MacroDuration ? MacroDuration : -1.f, FColor::Yellow, x);
#define POINT(x, c) DrawDebugPoint(GetWorld(), x, 10, c, !MacroDuration, MacroDuration);
#define LINE(x1, x2, c) DrawDebugLine(GetWorld(), x1, x2, c, !MacroDuration, MacroDuration);
#define CAPSULE(x, c) DrawDebugCapsule(GetWorld(), x, CapHH(), CapR(), FQuat::Identity, c, !MacroDuration, MacroDuration);
#define SLIDELOG(x) GEngine->AddOnScreenDebugMessage(30, MacroDuration ? MacroDuration : -1.f, FColor::Green, x);
#else
#define SLOG(x)
#define POINT(x, c)
#define LINE(x1, x2, c)
#define CAPSULE(x, c)
#define SLIDELOG(x)
#endif

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
	Saved_bPreWantsToCrouch = CharacterMovement->Safe_bPrevWantsToCrouch;
}

void UAfterCutCharacterMovementComp::FSavedMove_AC::PrepMoveFor(ACharacter* C)
{
	Super::PrepMoveFor(C);

	UAfterCutCharacterMovementComp* CharacterMovement = Cast<UAfterCutCharacterMovementComp>(C->GetCharacterMovement());

	CharacterMovement->Safe_bWantsToSprint = Saved_bWantsToSprint;
	CharacterMovement->Safe_bPrevWantsToCrouch = Saved_bPreWantsToCrouch;
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
	NavAgentProps.bCanCrouch = true;
}

void UAfterCutCharacterMovementComp::InitializeComponent()
{
	Super::InitializeComponent();

	AfterCutCharacterOwner = Cast<AAfterCutCharacter>(GetOwner());
}

float UAfterCutCharacterMovementComp::GetMaxSpeed() const
{
	if (IsMovementMode(MOVE_Walking) && Safe_bWantsToSprint && !IsCrouching()) return SprintSpeed;

	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		return SlideSpeed;
	default:
		break;
	}


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

	Safe_bPrevWantsToCrouch = bWantsToCrouch;
}

bool UAfterCutCharacterMovementComp::IsMovingOnGround() const
{
	return Super::IsMovingOnGround() || IsCustomMovementMode(CMOVE_Slide);
}

bool UAfterCutCharacterMovementComp::CanCrouchInCurrentState() const
{
	return Super::CanCrouchInCurrentState() /*&& IsMovingOnGround()*/; // might want to remove the is moving on ground, crouching in air shold be allowed. IF I want this I need to rewrite the crouch as it works on basis that you are not in air
}

void UAfterCutCharacterMovementComp::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	if (bWantsToCrouch && Safe_bPrevWantsToCrouch) { // sliding can be done in air to be able to slide down high angels that you cant walk on
		FHitResult PotentialSlideSurface;
		if (CanSlide(PotentialSlideSurface)) {
			EnterSlide(PotentialSlideSurface);
		}
	}

	if (IsCustomMovementMode(CMOVE_Slide) && !bWantsToCrouch)
		ExitSlide();

	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}

void UAfterCutCharacterMovementComp::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);

	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		PhysSlide(deltaTime, Iterations);
		break;
	default:
		UE_LOG(LogTemp, Fatal, TEXT("Invalid Movement Mode"));
		break;
	}
}

/// <summary>
/// MOVEMENT
/// </summary>

// SLIDE

void UAfterCutCharacterMovementComp::EnterSlide(FHitResult& Floor)
{
	(void)Floor;
	SLIDELOG("-- ENTER_SLIDE --");

	//Velocity = Velocity.ProjectOnToNormal(Floor.ImpactNormal);
	Velocity += Velocity.GetSafeNormal2D() * SlideEnterImpulse;
	SetMovementMode(MOVE_Custom, CMOVE_Slide);
}

void UAfterCutCharacterMovementComp::ExitSlide()
{
	FQuat NewRoation = FRotationMatrix::MakeFromXZ(UpdatedComponent->GetForwardVector().GetSafeNormal2D(), -AfterCutCharacterOwner->GetGravityDirection().GetSafeNormal()).ToQuat();
	FHitResult Hit;
	SafeMoveUpdatedComponent(FVector::ZeroVector, NewRoation, true, Hit);
	SetMovementMode(MOVE_Walking);

	SLIDELOG("-- EXIT_SLIDE --")
}

void UAfterCutCharacterMovementComp::PhysSlide(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
		return;

	FHitResult SurfaceHit;
	if (!CanSlide(SurfaceHit)) { // If not on valid surface or to slow stop sliding
		ExitSlide();
		StartNewPhysics(deltaTime, Iterations);
		return;
	}

	bJustTeleported = false;
	bool bCheckedFall = false;
	bool bTriedLedgeMove = false;
	float remainingTime = deltaTime;

	while ((remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && CharacterOwner && 
		(CharacterOwner->Controller || bRunPhysicsWithNoController || (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy))) { // prefrom the move 
		Iterations++;
		bJustTeleported = false;
		const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;

		FStepDownResult StepDown;

		MoveAlongFloor(Velocity, timeTick, &StepDown);
	 }


	 FHitResult Hit;
	 FQuat NewRotation = FRotationMatrix::MakeFromXZ(Velocity.GetSafeNormal2D(), FVector::UpVector).ToQuat();
	 SafeMoveUpdatedComponent(FVector::ZeroVector, NewRotation, false, Hit);
}

bool UAfterCutCharacterMovementComp::GetSlideSurface(FHitResult& Hit) const
{
	FHitResult BestHit;
	BestHit.ImpactPoint = FVector(99999999999999983222784.0f);

	UWorld* World = UpdatedComponent->GetWorld();
	FVector Location = UpdatedComponent->GetComponentLocation() + (UpdatedComponent->GetUpVector()*-1) * (AfterCutCharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()*0.5f);
	float Radius = AfterCutCharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius() + 0.25f;
	TArray<AActor*> IgnoreActors = AfterCutCharacterOwner->GetIgnoreActors();
	TArray<FHitResult> Hits;
	if (!UKismetSystemLibrary::SphereTraceMultiByProfile(World, Location, Location, Radius, "SlideProfile", true, IgnoreActors, EDrawDebugTrace::ForDuration, Hits, true))
		return false;
	
	bool gotReplaced = false;
	for (FHitResult hit : Hits) { // check for the best hit possible
		// check if the floor you are standing on isnt to steep to slide on, ie not a wall
		float FloorDot = FVector::DotProduct(hit.ImpactNormal, UpdatedComponent->GetUpVector());
		float FloorAngel = UKismetMathLibrary::DegAcos(FloorDot);
		if (FloorAngel >= 70.5f /* replace with floor angel veriable */) {
			UE_LOG(LogTemp, Error, TEXT("Floor angel failed check"));
			continue;
		}

		// check if the Z value is less then the best hit. If true replace the best hit
		FTransform PlayerTransfrom = UpdatedComponent->GetComponentTransform();
		FVector bestVector = UKismetMathLibrary::TransformLocation(PlayerTransfrom, BestHit.ImpactPoint);
		FVector hitVector = UKismetMathLibrary::TransformLocation(PlayerTransfrom, hit.ImpactPoint);

		if (bestVector.Z > hitVector.Z) {
			gotReplaced = true;
			BestHit = hit;
		}
	}

	Hit = BestHit;
	return gotReplaced;

	return true;
}

bool UAfterCutCharacterMovementComp::CanSlide(FHitResult& Hit) const
{
	return GetSlideSurface(Hit) && Velocity.SizeSquared() < pow(MinSpeedToSlide, 2);
}

/// <summary>
/// INPUTS
/// </summary>

void UAfterCutCharacterMovementComp::SprintPressed()
{
	if (ToggleSprint)
		Safe_bWantsToSprint = !Safe_bWantsToSprint;
	else
		Safe_bWantsToSprint = true;
}

void UAfterCutCharacterMovementComp::SprintReleased()
{
	if(!ToggleSprint)
		Safe_bWantsToSprint = false;
}

void UAfterCutCharacterMovementComp::CrouchPressed()
{
	if (ToggleCrouch)
		bWantsToCrouch = !bWantsToCrouch;
	else
		bWantsToCrouch = true;
}

void UAfterCutCharacterMovementComp::CrouchReleased()
{
	if(!ToggleCrouch)
		bWantsToCrouch = false;
}

bool UAfterCutCharacterMovementComp::IsMovementMode(EMovementMode InMovementMode) const
{
	return InMovementMode == MovementMode;
}

bool UAfterCutCharacterMovementComp::IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == InCustomMovementMode;
}
