// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AfterCut/Public/AfterCutCharacter.h"
#include "Reflectable.generated.h"

UCLASS()
class AFTERCUT_API AReflectable : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AReflectable();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	virtual void Reflect(FVector TargetLocation, AActor* TargetActor = nullptr);
	
	UPROPERTY(BlueprintReadOnly) AAfterCutCharacter* Player;
};
