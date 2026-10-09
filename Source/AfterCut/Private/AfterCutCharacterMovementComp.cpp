#include "AfterCutCharacterMovementComp.h"

#include "AfterCutCharacter.h"
#include "NetworkPredictionCVars.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/TextRenderActor.h"
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
#define DASHLOG(x) GEngine->AddOnScreenDebugMessage(30, MacroDuration ? MacroDuration : -1.f, FColor::Red, x);
#else
#define SLOG(x)
#define POINT(x, c)
#define LINE(x1, x2, c)
#define CAPSULE(x, c)
#define SLIDELOG(x)
#define DASHLOG(x)
#endif

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

float UAfterCutCharacterMovementComp::GetMaxBrakingDeceleration() const
{
	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		return AfterCutCharacterOwner->GetPlayerSettings()->SlideBrakingDeceleration;
	case CMOVE_Dash:
		return AfterCutCharacterOwner->GetPlayerSettings()->DashBrakingDeceleration;
	default:
		break;
	}

	return Super::GetMaxBrakingDeceleration();
}

float UAfterCutCharacterMovementComp::GetMaxSpeed() const
{
	if (IsMovementMode(MOVE_Walking) && bWantsToSprint && !IsCrouching()) return AfterCutCharacterOwner->GetPlayerSettings()->SprintSpeed;

	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		return AfterCutCharacterOwner->GetPlayerSettings()->SlideSpeed;
	case CMOVE_Dash:
		return AfterCutCharacterOwner->GetPlayerSettings()->DashImpulse;
	default:
		break;
	}

	return Super::GetMaxSpeed();
}

float UAfterCutCharacterMovementComp::GetGroundFriction() const
{
	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		return AfterCutCharacterOwner->GetPlayerSettings()->SlideFriction;
	case CMOVE_Dash:
		return AfterCutCharacterOwner->GetPlayerSettings()->DashFriction;
	default:
		return GroundFriction;
		break;
	}
}

float UAfterCutCharacterMovementComp::GetMaxAcceleration() const
{

	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		return AfterCutCharacterOwner->GetPlayerSettings()->SlideMaxAcceleration;
	case CMOVE_Dash:
		return AfterCutCharacterOwner->GetPlayerSettings()->DashMaxAcceleration;
	default:
		break;
	}

	return Super::GetMaxAcceleration();
}

void UAfterCutCharacterMovementComp::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
	
	bPrevWantsToCrouch = bWantsToCrouch;
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
	GEngine->AddOnScreenDebugMessage(1, 1.0f, FColor::Yellow, FString::Printf(TEXT("Allowed Floor Angle Is; %f"),GetWalkableFloorAngle()));
	
	// Enter Movementmodes
	if (bWantsToDash && CanDash())
		EnterDash();
	
	if (bWantsToCrouch) { // sliding can be done in air to be able to slide down high angels that you cant walk on
		FHitResult PotentialSlideSurface;
		if (CanSlide(PotentialSlideSurface) && !IsCustomMovementMode(CMOVE_Slide)) {
			EnterSlide();
		}
	}

	// ~ Enter Movementmodes

	// Exit Movement mode
	if (IsCustomMovementMode(CMOVE_Slide) && !bWantsToCrouch)
		ExitSlide();

	if (IsCustomMovementMode(CMOVE_Dash))
		ExitDash();

	// ~ Exit Movement mode

	ResetCallVeriables();

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
	case CMOVE_Dash:
		PhysDash(deltaTime, Iterations);
	default:	
		UE_LOG(LogTemp, Fatal, TEXT("Invalid Movement Mode"));
		break;
	}
}

/// <summary>
/// MOVEMENT
/// </summary>

void UAfterCutCharacterMovementComp::MoveAlongFloor(const FVector& InVelocity, float DeltaSeconds, FStepDownResult* OutStepDownResult)
{
	if (!CurrentFloor.IsWalkableFloor())
		return;
	
	// Move along the current floor
	const FVector Delta = ProjectToGravityFloor(InVelocity) * DeltaSeconds;
	FHitResult Hit(1.f);
	FVector RampVector = ComputeGroundMovementDelta(Delta, CurrentFloor.HitResult, CurrentFloor.bLineTrace);
	SafeMoveUpdatedComponent(RampVector, UpdatedComponent->GetComponentQuat(), true, Hit);
	float LastMoveTimeSlice = DeltaSeconds;
	
	if (Hit.bStartPenetrating)
	{
		// Allow this hit to be used as an impact we can deflect off, otherwise we do nothing the rest of the update and appear to hitch.
		HandleImpact(Hit);
		SlideAlongSurface(Delta, 1.f, Hit.Normal, Hit, true);

		if (Hit.bStartPenetrating)
		{
			OnCharacterStuckInGeometry(&Hit);
		}
	}
	else if (Hit.IsValidBlockingHit())
	{
		// We impacted something (most likely another ramp, but possibly a barrier).
		float PercentTimeApplied = Hit.Time;
		if ((Hit.Time > 0.f) && (GetGravitySpaceZ(Hit.Normal) > UE_KINDA_SMALL_NUMBER) && IsWalkable(Hit))
		{
			// Another walkable ramp.
			const float InitialPercentRemaining = 1.f - PercentTimeApplied;
			RampVector = ComputeGroundMovementDelta(Delta * InitialPercentRemaining, Hit, false);
			LastMoveTimeSlice = InitialPercentRemaining * LastMoveTimeSlice;
			SafeMoveUpdatedComponent(RampVector, UpdatedComponent->GetComponentQuat(), true, Hit);

			const float SecondHitPercent = Hit.Time * InitialPercentRemaining;
			PercentTimeApplied = FMath::Clamp(PercentTimeApplied + SecondHitPercent, 0.f, 1.f);
		}

		if (Hit.IsValidBlockingHit())
		{
			if (CanStepUp(Hit) || (CharacterOwner->GetMovementBase() != nullptr && Hit.HitObjectHandle == CharacterOwner->GetMovementBase()->GetOwner()))
			{
				// hit a barrier, try to step up
				const FVector PreStepUpLocation = UpdatedComponent->GetComponentLocation();
				if (!StepUp(GetGravityDirection(), Delta * (1.f - PercentTimeApplied), Hit, OutStepDownResult))
				{
					HandleImpact(Hit, LastMoveTimeSlice, RampVector);
					SlideAlongSurface(Delta, 1.f - PercentTimeApplied, Hit.Normal, Hit, true);
				}
				else
				{
					if (!bMaintainHorizontalGroundVelocity)
					{
						// Don't recalculate velocity based on this height adjustment, if considering vertical adjustments. Only consider horizontal movement.
						bJustTeleported = true;
						const float StepUpTimeSlice = (1.f - PercentTimeApplied) * DeltaSeconds;
						if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && StepUpTimeSlice >= UE_KINDA_SMALL_NUMBER)
						{
							Velocity = (UpdatedComponent->GetComponentLocation() - PreStepUpLocation) / StepUpTimeSlice;
							Velocity = ProjectToGravityFloor(Velocity);
						}
					}
				}
			}
			else if ( Hit.Component.IsValid() && !Hit.Component.Get()->CanCharacterStepUp(CharacterOwner) )
			{
				HandleImpact(Hit, LastMoveTimeSlice, RampVector);
				SlideAlongSurface(Delta, 1.f - PercentTimeApplied, Hit.Normal, Hit, true);
			}
		}
	}
}

void UAfterCutCharacterMovementComp::CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration)
{
	// Do not update velocity when using root motion or when SimulatedProxy and not simulating root motion - SimulatedProxy are repped their Velocity
	if (!HasValidData() || HasAnimRootMotion() || DeltaTime < MIN_TICK_TIME || (CharacterOwner && CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy && !bWasSimulatingRootMotion))
	{
		return;
	}

	Friction = FMath::Max(0.f, Friction);
	const float MaxAccel = GetMaxAcceleration();
	float MaxSpeed = GetMaxSpeed();
	
	// Check if path following requested movement
	bool bZeroRequestedAcceleration = true;
	FVector RequestedAcceleration = FVector::ZeroVector;
	float RequestedSpeed = 0.0f;
	if (ApplyRequestedMove(DeltaTime, MaxAccel, MaxSpeed, Friction, BrakingDeceleration, RequestedAcceleration, RequestedSpeed))
	{
		bZeroRequestedAcceleration = false;
	}

	if (bForceMaxAccel)
	{
		// Force acceleration at full speed.
		// In consideration order for direction: Acceleration, then Velocity, then Pawn's rotation.
		if (Acceleration.SizeSquared() > UE_SMALL_NUMBER)
		{
			Acceleration = Acceleration.GetSafeNormal() * MaxAccel;
		}
		else 
		{
			Acceleration = MaxAccel * (Velocity.SizeSquared() < UE_SMALL_NUMBER ? UpdatedComponent->GetForwardVector() : Velocity.GetSafeNormal());
		}

		AnalogInputModifier = 1.f;
	}

	// Path following above didn't care about the analog modifier, but we do for everything else below, so get the fully modified value.
	// Use max of requested speed and max speed if we modified the speed in ApplyRequestedMove above.
	const float MaxInputSpeed = FMath::Max(MaxSpeed * AnalogInputModifier, GetMinAnalogSpeed());
	MaxSpeed = FMath::Max(RequestedSpeed, MaxInputSpeed);

	// Apply braking or deceleration
	const bool bZeroAcceleration = Acceleration.IsZero();
	const bool bVelocityOverMax = IsExceedingMaxSpeed(MaxSpeed);
	
	// Only apply braking if there is no acceleration, or we are over our max speed and need to slow down to it.
	if ((bZeroAcceleration && bZeroRequestedAcceleration) || bVelocityOverMax)
	{
		const FVector OldVelocity = Velocity;

		const float ActualBrakingFriction = (bUseSeparateBrakingFriction ? BrakingFriction : Friction);
		ApplyVelocityBraking(DeltaTime, ActualBrakingFriction, BrakingDeceleration);
	
		// Don't allow braking to lower us below max speed if we started above it.
		if (bVelocityOverMax && Velocity.SizeSquared() < FMath::Square(MaxSpeed) && FVector::DotProduct(Acceleration, OldVelocity) > 0.0f)
		{
			Velocity = OldVelocity.GetSafeNormal() * MaxSpeed;
		}
	}
	else if (!bZeroAcceleration)
	{
		// Friction affects our ability to change direction. This is only done for input acceleration, not path following.
		const FVector AccelDir = Acceleration.GetSafeNormal();
		const float VelSize = Velocity.Size();
		Velocity = Velocity - (Velocity - AccelDir * VelSize) * FMath::Min(DeltaTime * Friction, 1.f);
	}

	// Apply fluid friction
	if (bFluid)
	{
		Velocity = Velocity * (1.f - FMath::Min(Friction * DeltaTime, 1.f));
	}

	// Apply input acceleration
	if (!bZeroAcceleration)
	{
		const float NewMaxInputSpeed = IsExceedingMaxSpeed(MaxInputSpeed) ? Velocity.Size() : MaxInputSpeed;
		Velocity += Acceleration * DeltaTime;
		Velocity = Velocity.GetClampedToMaxSize(NewMaxInputSpeed);
	}

	// Apply additional requested acceleration
	if (!bZeroRequestedAcceleration)
	{
		const float NewMaxRequestedSpeed = IsExceedingMaxSpeed(RequestedSpeed) ? Velocity.Size() : RequestedSpeed;
		Velocity += RequestedAcceleration * DeltaTime;
		Velocity = Velocity.GetClampedToMaxSize(NewMaxRequestedSpeed);
	}

	if (bUseRVOAvoidance)
	{
		CalcAvoidanceVelocity(DeltaTime);
	}
}

bool UAfterCutCharacterMovementComp::ApplyRequestedMove(float DeltaTime, float MaxAccel, float MaxSpeed, float Friction, float BrakingDeceleration, FVector& OutAcceleration, float& OutRequestedSpeed)
{
	if (bHasRequestedVelocity)
	{
		const float RequestedSpeedSquared = RequestedVelocity.SizeSquared();
		if (RequestedSpeedSquared < UE_KINDA_SMALL_NUMBER)
		{
			return false;
		}

		// Compute requested speed from path following
		float RequestedSpeed = FMath::Sqrt(RequestedSpeedSquared);
		const FVector RequestedMoveDir = RequestedVelocity / RequestedSpeed;
		RequestedSpeed = (bRequestedMoveWithMaxSpeed ? MaxSpeed : FMath::Min(MaxSpeed, RequestedSpeed));
		
		// Compute actual requested velocity
		const FVector MoveVelocity = RequestedMoveDir * RequestedSpeed;
		
		// Compute acceleration. Use MaxAccel to limit speed increase, 1% buffer.
		FVector NewAcceleration = FVector::ZeroVector;
		const float CurrentSpeedSq = Velocity.SizeSquared();
		if (ShouldComputeAccelerationToReachRequestedVelocity(RequestedSpeed))
		{
			// Turn in the same manner as with input acceleration.
			const float VelSize = FMath::Sqrt(CurrentSpeedSq);
			Velocity = Velocity - (Velocity - RequestedMoveDir * VelSize) * FMath::Min(DeltaTime * Friction, 1.f);

			// How much do we need to accelerate to get to the new velocity?
			NewAcceleration = ((MoveVelocity - Velocity) / DeltaTime);
			NewAcceleration = NewAcceleration.GetClampedToMaxSize(MaxAccel);
		}
		else
		{
			// Just set velocity directly.
			// If decelerating we do so instantly, so we don't slide through the destination if we can't brake fast enough.
			Velocity = MoveVelocity;
		}

		// Copy to out params
		OutRequestedSpeed = RequestedSpeed;
		OutAcceleration = NewAcceleration;
		return true;
	}

	return false;
}

// SLIDE
bool UAfterCutCharacterMovementComp::CanSlide(FHitResult& Hit) const
{
	return GetSlideSurface(Hit) && Velocity.Size() > AfterCutCharacterOwner->GetPlayerSettings()->MinSpeedToSlide; // there is a surface bellow player and their vel is high enought
}

void UAfterCutCharacterMovementComp::EnterSlide()
{
	SLIDELOG("-- ENTER_SLIDE --");
	
	SetWalkableFloorAngle(AfterCutCharacterOwner->GetPlayerSettings()->MaxFloorAngleForSlide);
	FStepDownResult StepDownResult;
	FindFloor(UpdatedComponent->GetComponentLocation(), StepDownResult.FloorResult, false, NULL);
	CurrentFloor = StepDownResult.FloorResult;
	
	if (Velocity.Size() < AfterCutCharacterOwner->GetPlayerSettings()->SlideSpeed) {
		FVector NewV = Velocity.GetSafeNormal() * AfterCutCharacterOwner->GetPlayerSettings()->SlideSpeed;
		NewV = UKismetMathLibrary::ProjectVectorOnToPlane(NewV,CurrentFloor.HitResult.ImpactNormal);
		MoveAlongFloor(NewV,0.1f, NULL);
		UKismetSystemLibrary::DrawDebugArrow(GetWorld(), UpdatedComponent->GetComponentLocation(), UpdatedComponent->GetComponentLocation() + (Velocity.GetSafeNormal() * 100.0f), 3.0f , FColor::Red, 30.0f, 3.0f);
	}

	SetMovementMode(MOVE_Custom, CMOVE_Slide);
}

void UAfterCutCharacterMovementComp::ExitSlide()
{
	SetWalkableFloorAngle(45.0f);

	SLIDELOG("-- EXIT_SLIDE --")
}

void UAfterCutCharacterMovementComp::PhysSlide(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME) {
		ExitSlide();
		return;
	}

	FHitResult SlideHit;
	if (!UpdatedComponent->IsQueryCollisionEnabled() || !CanSlide(SlideHit))
	{
		SetMovementMode(MOVE_Walking);
		ExitSlide();
		return;
	}

	bJustTeleported = false;
	bool bCheckedFall = false;
	bool bTriedLedgeMove = false;
	float remainingTime = deltaTime;

	const EMovementMode StartingMovementMode = MovementMode;
	const uint8 StartingCustomMovementMode = CustomMovementMode;

	// Perform the move
	while (remainingTime >= MIN_TICK_TIME && Iterations < MaxSimulationIterations)
	{
		Iterations++;
		bJustTeleported = false;
		const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;

		const FVector OldLocation = UpdatedComponent->GetComponentLocation();

		CalcVelocity(timeTick, GetGroundFriction(), true, GetMaxBrakingDeceleration());
		const FVector MoveVelocity = Velocity;
		const FVector Delta = timeTick * MoveVelocity;
		const bool bZeroDelta = Delta.IsNearlyZero();
		FStepDownResult StepDownResult;

		MoveAlongFloor(MoveVelocity, timeTick, &StepDownResult);

		if (StepDownResult.bComputedFloor)
		{
			CurrentFloor = StepDownResult.FloorResult;
		}
		else
		{
			FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, bZeroDelta, NULL);
		}

		// If we didn't move at all this iteration then abort (since future iterations will also be stuck).
		if (UpdatedComponent->GetComponentLocation() == OldLocation)
		{
			remainingTime = 0.f;
			break;
		}

		MaintainHorizontalGroundVelocity();
	}

	if (IsMovingOnGround())
	{
		MaintainHorizontalGroundVelocity();
	}
}

bool UAfterCutCharacterMovementComp::GetSlideSurface(FHitResult& Hit) const
{
	FHitResult BestHit;
	BestHit.ImpactPoint = FVector(99999999999999983222784.0f);

	UWorld* World = UpdatedComponent->GetWorld();
	FVector Location = UpdatedComponent->GetComponentLocation() + (UpdatedComponent->GetUpVector() * -1) * (AfterCutCharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()*0.5f);
	float Radius = AfterCutCharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius() + 0.25f;
	TArray<AActor*> IgnoreActors = AfterCutCharacterOwner->GetIgnoreActors();
	TArray<FHitResult> Hits;
	if (!UKismetSystemLibrary::SphereTraceMultiByProfile(World, Location, Location, Radius, "SlideProfile", true, IgnoreActors, EDrawDebugTrace::None, Hits, true))
		return false;
	
	bool gotReplaced = false;
	for (FHitResult hit : Hits) { // check for the best hit possible
		// check if the floor you are standing on isnt to steep to slide on, ie not a wall
		float FloorDot = FVector::DotProduct(hit.ImpactNormal, UpdatedComponent->GetUpVector());
		float FloorAngel = UKismetMathLibrary::DegAcos(FloorDot);
		if (FloorAngel >= AfterCutCharacterOwner->GetPlayerSettings()->MaxFloorAngleForSlide) {
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
	return gotReplaced; // if not the orginal point then we have a valid hit so return true else return false
}

// DASH
bool UAfterCutCharacterMovementComp::CanDash()
{
	return true;
}


void UAfterCutCharacterMovementComp::EnterDash()
{
	DASHLOG("-- ENTER_DASH --");
	SetMovementMode(MOVE_Custom, CMOVE_Dash);
}

void UAfterCutCharacterMovementComp::ExitDash()
{
	DASHLOG("-- EXIT_DASH --");
	SetMovementMode(MOVE_Walking);

}

void UAfterCutCharacterMovementComp::PhysDash(float deltaTime, int32 Iterations)
{

}

void UAfterCutCharacterMovementComp::ResetCallVeriables()
{
	bWantsToDash = false;
}

/// <summary>
/// INPUTS
/// </summary>

void UAfterCutCharacterMovementComp::SprintPressed()
{
	if (AfterCutCharacterOwner->GetPlayerSettings()->ToggleSprint)
		bWantsToSprint = !bWantsToSprint;
	else
		bWantsToSprint = true;
}

void UAfterCutCharacterMovementComp::SprintReleased()
{
	if(!AfterCutCharacterOwner->GetPlayerSettings()->ToggleSprint)
		bWantsToSprint = false;
}

void UAfterCutCharacterMovementComp::CrouchPressed()
{
	if (AfterCutCharacterOwner->GetPlayerSettings()->ToggleCrouch)
		bWantsToCrouch = !bWantsToCrouch;
	else
		bWantsToCrouch = true;
}

void UAfterCutCharacterMovementComp::CrouchReleased()
{
	if(!AfterCutCharacterOwner->GetPlayerSettings()->ToggleCrouch)
		bWantsToCrouch = false;
}

void UAfterCutCharacterMovementComp::DashPressed()
{
	bWantsToDash = true;
}

void UAfterCutCharacterMovementComp::DashReleased()
{

}

bool UAfterCutCharacterMovementComp::IsMovementMode(EMovementMode InMovementMode) const
{
	return InMovementMode == MovementMode;
}

bool UAfterCutCharacterMovementComp::IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == InCustomMovementMode;
}
