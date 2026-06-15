#include "AfterCutCameraManager.h"

#include "AfterCutCharacter.h"
#include "AfterCutCharacterMovementComp.h"
#include "Components/CapsuleComponent.h"

AAfterCutCameraManager::AAfterCutCameraManager()
{

}

void AAfterCutCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
	Super::UpdateViewTarget(OutVT, DeltaTime);

	if (AAfterCutCharacter* AfterCutCharacter = Cast<AAfterCutCharacter>(GetOwningPlayerController()->GetPawn())) {
		UAfterCutCharacterMovementComp* ACCMC = AfterCutCharacter->GetAfterCutCharacterMovement();

		FVector TargetCrouchOffset = FVector(
			0.0f,
			0.0f,
			ACCMC->GetCrouchedHalfHeight() - AfterCutCharacter->GetClass()->GetDefaultObject<ACharacter>()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
		);

		FVector Offset = FMath::Lerp(FVector::ZeroVector, TargetCrouchOffset, FMath::Clamp(CrouchBlendTime / CrouchBlendDuration, 0.0f, 1.0f));

		if (ACCMC->IsCrouching()) {
			CrouchBlendTime = FMath::Clamp(CrouchBlendTime + DeltaTime, 0.0f, CrouchBlendDuration);
			Offset -= TargetCrouchOffset;
		}
		else {
			CrouchBlendTime = FMath::Clamp(CrouchBlendTime - DeltaTime, 0.0f, CrouchBlendDuration);
		}

		if (ACCMC->IsMovingOnGround())
			OutVT.POV.Location += Offset;
	}
}
