#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "AfterCut/Public/PlayerSettings.h"

#include "AfterCutCameraManager.generated.h"

UCLASS()
class AFTERCUT_API AAfterCutCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

	typedef APlayerCameraManager Super;
	
	float CrouchBlendTime = 0.0f;

public:
	AAfterCutCameraManager();

	virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) override;
	
};
