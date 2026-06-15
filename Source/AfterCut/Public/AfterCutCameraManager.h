#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"

#include "AfterCutCameraManager.generated.h"

UCLASS()
class AFTERCUT_API AAfterCutCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

	typedef APlayerCameraManager Super;

	UPROPERTY(EditDefaultsOnly, Category = "AC|Crouch") float CrouchBlendDuration = 0.1f;
	float CrouchBlendTime = 0.0f;

public:
	AAfterCutCameraManager();

	virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) override;
};
