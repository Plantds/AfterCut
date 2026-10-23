// Fill out your copyright notice in the Description page of Project Settings.


#include "ArchingProjectile.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

FVector AArchingProjectile::CalcParabala(FVector StartPos, FVector TargetPos, float Hight, float Time, FVector PullDirection)
{
	FVector TDir = TargetPos - StartPos;
	float ParabilicT = Time * 2.0f - 1.0f;
	FVector ParabolaPoint = FVector::ZeroVector;
	if (/*UKismetMathLibrary::Abs(StartPos.Z - TargetPos.Z) < 0.1*/ true)
	{//simple (used if Z is close)
		ParabolaPoint = StartPos + (Time * TDir);
		ParabolaPoint += PullDirection.GetSafeNormal() * (ParabilicT * -ParabilicT + 1.0f) * Hight;
	}else
	{//expensive
		FVector Pos = StartPos - TargetPos;
	}
	
	return ParabolaPoint;
}

void AArchingProjectile::BeginPlay()
{
	Super::BeginPlay();
}

void AArchingProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AArchingProjectile::Reflect(FVector TargetLocation, AActor* TargetActor)
{
	Super::Reflect(TargetLocation, TargetActor);
}
