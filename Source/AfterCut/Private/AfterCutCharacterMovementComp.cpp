#include "AfterCutCharacterMovementComp.h"

#include "AfterCutCharacter.h"

#include "Camera/CameraComponent.h"
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
	if (IsMovementMode(MOVE_Walking) && bWantsToSprint && !IsCrouching()) return SprintSpeed;

	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		return SlideSpeed;
	default:
		break;
	}


	return Super::GetMaxSpeed();
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
	if (bWantsToDash && CanDash())
		ExecuteDash();
	
	if (bWantsToCrouch && bPrevWantsToCrouch) { // sliding can be done in air to be able to slide down high angels that you cant walk on
		FHitResult PotentialSlideSurface;
		if (CanSlide(PotentialSlideSurface) && !IsCustomMovementMode(CMOVE_Slide)) {
			EnterSlide(PotentialSlideSurface);
		}
	}

	if (IsCustomMovementMode(CMOVE_Slide) && !bWantsToCrouch)
		ExitSlide();

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
	if (!CanSlide(SurfaceHit)) { // If not on valid surface or to slow stop sliding (CURRENTLY WILL BE HIT CUS WELL MOVEMENT ISNT MOVING)
		SLOG("SLIDE ENDED EARLY");
		ExitSlide();
		StartNewPhysics(deltaTime, Iterations);
		return;
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
	return gotReplaced; // if not the orginal point then we have a valid hit so return true else return false
}

bool UAfterCutCharacterMovementComp::CanSlide(FHitResult& Hit) const
{
	return GetSlideSurface(Hit) && Velocity.SizeSquared() > pow(MinSpeedToSlide, 2); // there is a surface bellow player and their vel is high enought
}

// DASH

void UAfterCutCharacterMovementComp::ExecuteDash()	
{
	SLOG("EXECTUING DASH");

	SetMovementMode(MOVE_Falling);

	bool UseAddtiveVel = false;
	bool UsesCamera = false;
	FVector DashDir = GetInputDir(UseAddtiveVel, UsesCamera);
	DashDir = AdjustDashAngelDependingOnFloor(DashDir,UsesCamera);
	ApplyDash(DashDir, UseAddtiveVel);
}

bool UAfterCutCharacterMovementComp::CanDash()
{
	return IsMovingOnGround();
}

FVector UAfterCutCharacterMovementComp::GetInputDir(bool& RUseAddativeVel, bool& RUsesCamera)
{
	RUsesCamera = Acceleration.GetSafeNormal2D().Length() == 0 || Acceleration.GetSafeNormal2D().Dot(AfterCutCharacterOwner->GetCamera()->GetForwardVector().GetSafeNormal2D()) > 0.9f ? true : false;

	FVector DashDir = RUsesCamera ? AfterCutCharacterOwner->GetCamera()->GetForwardVector() : Acceleration.GetSafeNormal2D();

	RUseAddativeVel = UKismetMathLibrary::DegAcos(Velocity.Dot(DashDir)) <= 45.0f ? true : false; // 45 should be angle for addative and be a editable flaot value

	return DashDir;
}

FVector UAfterCutCharacterMovementComp::AdjustDashAngelDependingOnFloor(FVector DashInputDir, bool UsesCamera)
{
	UWorld* World = UpdatedComponent->GetWorld();
	FVector DownVector = -UpdatedComponent->GetUpVector();
	FVector LineTraceStart = UpdatedComponent->GetComponentLocation() + DownVector * AfterCutCharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	FVector LineTraceGround = LineTraceStart + (DownVector * 30.0f); // 30.0f should be a editable float value
	FVector LineTraceInputDir = LineTraceStart + DashInputDir * 100.0f; // 100.0f should be a editable float value

	TArray<AActor*> IgnoreActors = AfterCutCharacterOwner->GetIgnoreActors();
	FHitResult GroundHit;
	FHitResult InputHit;

	if (!UKismetSystemLibrary::LineTraceSingle(World, LineTraceStart, LineTraceGround, ETraceTypeQuery::TraceTypeQuery2, false, IgnoreActors, EDrawDebugTrace::ForDuration, GroundHit, true) 
		&& !UKismetSystemLibrary::LineTraceSingle(World, LineTraceStart, LineTraceInputDir, ETraceTypeQuery::TraceTypeQuery2, false, IgnoreActors, EDrawDebugTrace::ForDuration, InputHit, true))
		return DashInputDir = AfterCutCharacterOwner->GetCapsuleComponent()->GetForwardVector().Dot(DashInputDir) < 0.0f ? DashInputDir : -DashInputDir;
	
	FVector correctedVector = InputHit.ImpactNormal.Dot(GroundHit.ImpactNormal) > 0.5f ? (UsesCamera ? DashInputDir + UpdatedComponent->GetUpVector() : DashInputDir).ProjectOnToNormal(InputHit.ImpactNormal) : DashInputDir;
	correctedVector *= 100; // make sure that once we normalize it doesnt return 0
	correctedVector.Normalize();
	return correctedVector = UpdatedComponent->GetForwardVector().Dot(correctedVector) <= 0.0f ? -correctedVector : correctedVector;
}

void UAfterCutCharacterMovementComp::ApplyDash(FVector DashDir, bool IsAddative)
{
	FVector VelXY = FVector(Velocity.X,Velocity.Y,0.0f);
	Velocity = IsAddative ? VelXY.Length() * DashDir + DashImpulse * DashDir : DashDir * DashImpulse;
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
	if (ToggleSprint)
		bWantsToSprint = !bWantsToSprint;
	else
		bWantsToSprint = true;
}

void UAfterCutCharacterMovementComp::SprintReleased()
{
	if(!ToggleSprint)
		bWantsToSprint = false;
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
