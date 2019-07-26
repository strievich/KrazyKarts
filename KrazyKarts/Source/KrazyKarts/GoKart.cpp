// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKart.h"
#include "Components/InputComponent.h"

// Sets default values
AGoKart::AGoKart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGoKart::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    FVector Force = GetActorForwardVector() * MaxDrivingForce * Throttle;

    Force += GetAirResistance();
    Force += GetRollingResistance();

    Acceleration =  Force / Mass;
    Velocity += Acceleration*DeltaTime;

    UpdateSteering(DeltaTime);
    UpdateLocomotionFromVelocity(DeltaTime);

}

void AGoKart::UpdateSteering(float DeltaTime)
{
    float DeltaLocation = FVector::DotProduct(GetActorForwardVector(), Velocity)  *DeltaTime;


    float RotationAngle = DeltaLocation / MinTurningRadius * SteeringThrow;
    FQuat RotationDelta(GetActorUpVector(), RotationAngle);


    Velocity = RotationDelta.RotateVector(Velocity);

    AddActorWorldRotation(RotationDelta);
}

void AGoKart::UpdateLocomotionFromVelocity(float DeltaTime)
{
    FVector Translation = Velocity * DeltaTime;

    FHitResult HitResult;
    AddActorWorldOffset(Translation, true, &HitResult);
    if (HitResult.IsValidBlockingHit())
    {
        Velocity = FVector::ZeroVector;
    }
}

// Called to bind functionality to input
void AGoKart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("MoveForward", this, &AGoKart::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &AGoKart::MoveRight);

}

void AGoKart::MoveForward(float Value)
{
    Throttle = Value;
}

void AGoKart::MoveRight(float Value)
{
    SteeringThrow = Value;
}

FVector AGoKart::GetAirResistance()
{
    return -Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoefficient/100.f;
}

FVector AGoKart::GetRollingResistance()
{
    float AccelerationDueToGravity = -GetWorld()->GetGravityZ()/100.0f;
    float NormalForce = Mass * AccelerationDueToGravity;
    return -Velocity.GetSafeNormal()*RollingResistanceCoefficient*NormalForce;
}

