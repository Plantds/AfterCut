// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "EnhancedInput/Public/InputMappingContext.h"
#include "EnhancedInput/Public/InputAction.h"

#include "PlayerSettings.generated.h"

UCLASS()
class AFTERCUT_API UPlayerSettings : public UDataAsset
{
	GENERATED_BODY()
	
public:
	// Input
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input") UInputMappingContext* InputMapping;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Movement") UInputAction* LookAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Movement") UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Movement") UInputAction* JumpAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Movement") UInputAction* CrouchAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Movement") UInputAction* SprintAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Movement") UInputAction* DashAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Combat") UInputAction* SlashAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Combat") UInputAction* SheathAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Combat") UInputAction* ParryAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Interact") UInputAction* Interact;
	
	// Camera
	UPROPERTY(EditAnywhere, Category = "Camera|Sens", meta = (ClampMin = "0.001", ClampMax = "999.999", UIMin = "0.001", UIMax = "999.999")) float Sensitivity = 0.75f;
	UPROPERTY(EditAnywhere, Category = "Camera|Crouch") float CrouchBlendDuration = 0.1f;
	UPROPERTY(EditAnywhere, Category = "Camera|Crouch") float CrouchHeight = 0.0f;
	UPROPERTY(EditAnywhere, Category = "Camera|Boom") float CameraBoomHeight = 50.0f;
	
	// Movement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Toggles") bool ToggleSprint = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Toggles") bool ToggleCrouch = false;
	
	// Sprint
	UPROPERTY(EditAnywhere, Category = "Movement|Sprint") float SprintSpeed = 1000.0f;

	/// Slide
	UPROPERTY(EditAnywhere, Category = "Movement|Slide") float MinSpeedToSlide = 350.0f;
	UPROPERTY(EditAnywhere, Category = "Movement|Slide") float SlideSpeed = 1200.0f;
	UPROPERTY(EditAnywhere, Category = "Movement|Slide") float SlideEnterImpulse = 500.0f;
	UPROPERTY(EditAnywhere, Category = "Movement|Slide") float SlideGravityForce = 5000.0f;
	UPROPERTY(EditAnywhere, Category = "Movement|Slide") float SlideFriction = 2.0f;
	UPROPERTY(EditAnywhere, Category = "Movement|Slide") float SlideBrakingDeceleration = 3.0f;
	UPROPERTY(EditAnywhere, Category = "Movement|Slide") float SlideMaxAcceleration = 2048.0f;
	UPROPERTY(EditAnywhere, Category = "Movement|Slide") float MaxFloorAngleForSlide = 70.5f;
	UPROPERTY(EditAnywhere, Category = "Movement|Slide") float SlideSlopeGain = 10.0f;
	UPROPERTY(EditAnywhere, Category = "Movement|Slide") float SlideSlopeLoss = 10.0f;
	UPROPERTY(EditAnywhere, Category = "Movement|Slide") float SlideSlopeAngleMulti = 1.0f;
	

	/// Dash
	UPROPERTY(EditAnywhere, Category = "Movement|Dash") float DashImpulse = 1500.0f;
	UPROPERTY(EditAnywhere, Category = "Movement|Dash") float DashFriction = 0.0f;
	UPROPERTY(EditAnywhere, Category = "Movement|Dash") float DashBrakingDeceleration = 0.0f;
	UPROPERTY(EditAnywhere, Category = "Movement|Dash") float DashMaxAcceleration = 2048.0f;
	
	// Combat
	
};
