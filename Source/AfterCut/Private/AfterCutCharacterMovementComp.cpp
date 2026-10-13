#include "AfterCutCharacterMovementComp.h"

#include "AfterCutCharacter.h"
#include "NetworkPredictionCVars.h"

#include "Components/ArrowComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/TextRenderActor.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Storage/Nodes/FileEntry.h"

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

void UAfterCutCharacterMovementComp::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UAfterCutCharacterMovementComp::InitializeComponent()
{
	Super::InitializeComponent();

	AfterCutCharacterOwner = Cast<AAfterCutCharacter>(GetOwner());
	Settings = AfterCutCharacterOwner->GetPlayerSettings();
}

float UAfterCutCharacterMovementComp::GetMaxBrakingDeceleration() const
{
	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		return Settings->SlideBrakingDeceleration;
	case CMOVE_Dash:
		return Settings->DashBrakingDeceleration;
	default:
		break;
	}

	return Super::GetMaxBrakingDeceleration();
}

float UAfterCutCharacterMovementComp::GetMaxSpeed() const
{
	if (IsMovementMode(MOVE_Walking) && bWantsToSprint && !IsCrouching()) return Settings->SprintSpeed;

	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		return Settings->SlideSpeed;
	case CMOVE_Dash:
		return Settings->DashImpulse;
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
		return Settings->SlideFriction;
	case CMOVE_Dash:
		return Settings->DashFriction;
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
		return Settings->SlideMaxAcceleration;
	case CMOVE_Dash:
		return Settings->DashMaxAcceleration;
	default:
		break;
	}

	return Super::GetMaxAcceleration();
}

bool UAfterCutCharacterMovementComp::CanAttemptJump() const
{
	return Super::CanAttemptJump();
}

bool UAfterCutCharacterMovementComp::DoJump(bool bReplayingMoves, float _deltatime)
{
	return Super::DoJump(bReplayingMoves, _deltatime);
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
	// Enter Movementmodes
	if (bWantsToDash && CanDash())
		EnterDash();
	
	if (bWantsToCrouch) { // sliding can be done in air to be able to slide down high angels that you cant walk on
		FHitResult PotentialSlideSurface;
		if (CanSlide(PotentialSlideSurface) && !IsCustomMovementMode(CMOVE_Slide)) {
			SetMovementMode(MOVE_Custom, CMOVE_Slide);
		}
	}

	// ~ Enter Movementmodes

	// Exit Movement mode
	//if (IsCustomMovementMode(CMOVE_Slide) && !bWantsToCrouch)
	//	SetMovementMode(MOVE_Walking);

	if (IsCustomMovementMode(CMOVE_Dash))
		ExitDash();

	// ~ Exit Movement mode

	ResetCallVeriables();

	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}

void UAfterCutCharacterMovementComp::UpdateCharacterStateAfterMovement(float DeltaSeconds)
{
	Super::UpdateCharacterStateAfterMovement(DeltaSeconds);
}

void UAfterCutCharacterMovementComp::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
	
	bPrevWantsToCrouch = bWantsToCrouch;
}

void UAfterCutCharacterMovementComp::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
	
	//if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == CMOVE_Slide) ExitSlide();
	if (IsCustomMovementMode(CMOVE_Slide)) EnterSlide();
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

	if (bForceMaxAccel && !IsCustomMovementMode(CMOVE_Slide))
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
	GEngine->AddOnScreenDebugMessage(88, 3.0f, FColor::Red, 
			FString::Printf(TEXT("Vel.S > MinSpeed: %s and/or Surface is: %s"), 
				Velocity.Size() > Settings->MinSpeedToSlide ? TEXT("true") : TEXT("false"), 
				GetSlideSurface(Hit) ? TEXT("true") : TEXT("false")));
	return GetSlideSurface(Hit) && Velocity.Size() > Settings->MinSpeedToSlide; // there is a surface bellow player and their vel is high enought
}

UArrowComponent* VelArrow;
void UAfterCutCharacterMovementComp::EnterSlide()
{
	SLIDELOG("-- ENTER_SLIDE --");
	
	//SetWalkableFloorAngle(Settings->MaxFloorAngleForSlide);
	FStepDownResult StepDownResult;
	if (Velocity.Size() < Settings->SlideEnterImpulse)
		Velocity = Velocity.GetSafeNormal2D() * Settings->SlideEnterImpulse;
	
	FindFloor(UpdatedComponent->GetComponentLocation(), StepDownResult.FloorResult, false, NULL);
}

void UAfterCutCharacterMovementComp::ExitSlide()
{
	//SetWalkableFloorAngle(45.0f);
	
	SLIDELOG("-- EXIT_SLIDE --")
}

void UAfterCutCharacterMovementComp::PhysSlide(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME) {
		GEngine->AddOnScreenDebugMessage(1588, 3.0f, FColor::Red, FString("deltaTime was less then MIN_TICK_TIME" ));
		ExitSlide();
		return;
	}

	FHitResult SlideHit;
	if (!UpdatedComponent->IsQueryCollisionEnabled() || !CanSlide(SlideHit))
	{
		GEngine->AddOnScreenDebugMessage(1588, 3.0f, FColor::Red, 
			FString::Printf(TEXT("IsQueryCollisionEnabled() is %s and CanSlide() is %s"), 
				!UpdatedComponent->IsQueryCollisionEnabled() ? TEXT("true") :  TEXT("false"), !CanSlide(SlideHit) ? TEXT("true") : TEXT("false") ) );
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
		
		// Save current values
		UPrimitiveComponent * const OldBase = GetMovementBase();
		const FVector PreviousBaseLocation = (OldBase != NULL) ? OldBase->GetComponentLocation() : FVector::ZeroVector;
		const FVector OldLocation = UpdatedComponent->GetComponentLocation();
		const FFindFloorResult OldFloor = CurrentFloor;
		
		// Ensure velocity is horizontal.
		MaintainHorizontalGroundVelocity();
		const FVector OldVelocity = Velocity;
		Acceleration = FVector::VectorPlaneProject(Acceleration, -GetGravityDirection());

		// CUSTOM VEL AND ACCEL CALCUALTIONS
		//Velocity = Velocity.GetSafeNormal() * (Velocity.Size() - 1.0f);
		//Velocity = Velocity + ((-Settings->SlideFriction) * Velocity) * remainingTime; 
		
		FVector NewV = UKismetMathLibrary::ProjectVectorOnToPlane(Velocity, OldFloor.HitResult.Normal);
		float D = NewV.GetSafeNormal() | GetGravityDirection();

		FVector before = Velocity;
		
		Velocity = D > 0.0f ? Velocity.GetSafeNormal() * (Velocity.Size() + ((1.0f + Settings->SlideSlopeGain) + (D * Settings->SlideSlopeAngleMulti) ) * deltaTime) 
					: Velocity.GetSafeNormal() * (Velocity.Size() - ((1.0f - Settings->SlideSlopeLoss) + ((1.0f - D) * Settings->SlideSlopeAngleMulti) ) * deltaTime) ;

		Velocity = UKismetMathLibrary::RotateAngleAxis(Velocity, AfterCutCharacterOwner->GetMovementVector2D().X , UpdatedComponent->GetUpVector());
		
		//Velocity += GetGravityDirection() * GravityScale;
		//CalcVelocity(deltaTime, Settings->SlideFriction,false, GetMaxBrakingDeceleration());
		
		FVector after = Velocity;
		
		FVector a = before - after;
		
		GEngine->AddOnScreenDebugMessage(66, 3.0f, FColor::Cyan, 
			FString::Printf(TEXT("the loss/gain is %f"), a.Size()));
		
		// Compute move parameters
		const FVector MoveVelocity = Velocity;
		const FVector Delta = timeTick * MoveVelocity;
		const bool bZeroDelta = Delta.IsNearlyZero();
		FStepDownResult StepDownResult;
		
		if ( bZeroDelta )
		{
			remainingTime = 0.f;
		}
		else
		{
			// try to move forward
			MoveAlongFloor(MoveVelocity, timeTick, &StepDownResult);

			if (IsSwimming()) //just entered water
			{
				StartSwimming(OldLocation, OldVelocity, timeTick, remainingTime, Iterations);
				return;
			}
			else if (MovementMode != StartingMovementMode || CustomMovementMode != StartingCustomMovementMode)
			{
				// pawn ended up in a different mode, probably due to the step-up-and-over flow
				// let's refund the estimated unused time (if any) and keep moving in the new mode
				const float DesiredDist = Delta.Size();
				if (DesiredDist > UE_KINDA_SMALL_NUMBER)
				{
					const float ActualDist = ProjectToGravityFloor(UpdatedComponent->GetComponentLocation() - OldLocation).Size();
					remainingTime += timeTick * (1.f - FMath::Min(1.f,ActualDist/DesiredDist));
				}
				StartNewPhysics(remainingTime,Iterations);
				return;
			}
		}
		
		// Update floor.
		// StepUp might have already done it for us.
		if (StepDownResult.bComputedFloor)
		{
			CurrentFloor = StepDownResult.FloorResult;
		}
		else
		{
			FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, bZeroDelta, NULL);
		}
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
		if (FloorAngel >= Settings->MaxFloorAngleForSlide) {
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
	if (Settings->ToggleSprint)
		bWantsToSprint = !bWantsToSprint;
	else
		bWantsToSprint = true;
}

void UAfterCutCharacterMovementComp::SprintReleased()
{
	if(!Settings->ToggleSprint)
		bWantsToSprint = false;
}

void UAfterCutCharacterMovementComp::CrouchPressed()
{
	if (Settings->ToggleCrouch)
		bWantsToCrouch = !bWantsToCrouch;
	else
		bWantsToCrouch = true;
}

void UAfterCutCharacterMovementComp::CrouchReleased()
{
	if(!Settings->ToggleCrouch)
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
