// Fill out your copyright notice in the Description page of Project Settings.


#include "Reflectable.h"

#include "Kismet/GameplayStatics.h"

// Sets default values
AReflectable::AReflectable()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Player = Cast<AAfterCutCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(),0));
}

// Called when the game starts or when spawned
void AReflectable::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AReflectable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AReflectable::Reflect(FVector TargetLocation, AActor* TargetActor)
{
	(void) TargetActor;
	(void) TargetLocation;
}

