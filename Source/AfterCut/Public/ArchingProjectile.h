// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Reflectable.h"
#include "ArchingProjectile.generated.h"

/**
 * 
 */
UCLASS()
class AFTERCUT_API AArchingProjectile : public AReflectable
{
	GENERATED_BODY()
	
private:
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintCallable) FVector CalcParabala(FVector StartPos, FVector TargetPos, float Hight, float Time, FVector PullDirection);
	UFUNCTION(BlueprintCallable) void Reflect(FVector TargetLocation, AActor* TargetActor = nullptr) override;
	
};
