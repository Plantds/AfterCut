// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h" 
#include "ReflectSettings.generated.h"

/**
 * 
 */
UCLASS()
class AFTERCUT_API UReflectSettings : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	// Reflection
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reflection", meta = (DisplayName = "ReflectionCheckDistance")) float ReflectionCheckDistance = 3000.0f;
	
	// Projectile
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (DisplayName = "Reflected Speed")) float pReflectedSpeed = 3000.0f;

	// Hit Scan
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HitScan", meta = (DisplayName = "Reflected Speed")) float hsReflectedSpeed = 3000.0f;
	
	// Beam
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beam", meta = (DisplayName = "Reflected Speed")) float bReflectedSpeed = 3000.0f;
};
