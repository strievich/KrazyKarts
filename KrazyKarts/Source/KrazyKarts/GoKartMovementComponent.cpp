// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKartMovementComponent.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "DrawDebugHelpers.h"

FString GetEnumText(ENetRole InRole)
{
    switch (InRole)
    {
    case ROLE_None:
        return "None";
    case ROLE_SimulatedProxy:
        return "SimulatedProxy";
    case ROLE_AutonomousProxy:
        return "AutonomousProxy";
    case ROLE_Authority:
        return "Authority";
    default:
        return "ERROR";
    }

}


// Sets default values for this component's properties
UGoKartMovementComponent::UGoKartMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UGoKartMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UGoKartMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
FGoCartMove UGoKartMovementComponent::CreateMove(float DeltaTime)
{
    FGoCartMove Move;
    Move.DeltaTime = DeltaTime;
    Move.SteeringThrow = SteeringThrow;
    Move.Throttle = Throttle;
    Move.Time = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
    return Move;
}

void UGoKartMovementComponent::SimulateMove(FGoCartMove Move)
{
    FVector Force = GetOwner()->GetActorForwardVector() * MaxDrivingForce * Move.Throttle;

    Force += GetAirResistance();
    Force += GetRollingResistance();

    Acceleration = Force / Mass;
    Velocity += Acceleration * Move.DeltaTime;

    ApplyRotation(Move.DeltaTime, Move.SteeringThrow);
    UpdateLocomotionFromVelocity(Move.DeltaTime);

    DrawDebugString(GetWorld(), FVector(0, 0, 100), GetEnumText(GetOwner()->Role.GetValue()), GetOwner(), FColor::White, Move.DeltaTime, true);
}



FVector UGoKartMovementComponent::GetAirResistance()
{
    return -Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoefficient / 100.f;
}

FVector UGoKartMovementComponent::GetRollingResistance()
{
    float AccelerationDueToGravity = -GetWorld()->GetGravityZ() / 100.0f;
    float NormalForce = Mass * AccelerationDueToGravity;
    return -Velocity.GetSafeNormal()*RollingResistanceCoefficient*NormalForce;
}

void UGoKartMovementComponent::ApplyRotation(float DeltaTime, float InSteeringThrow)
{
    float DeltaLocation = FVector::DotProduct(GetOwner()->GetActorForwardVector(), Velocity)  *DeltaTime;


    float RotationAngle = (DeltaLocation / MinTurningRadius) * InSteeringThrow;
    FQuat RotationDelta(GetOwner()->GetActorUpVector(), RotationAngle);

    Velocity = RotationDelta.RotateVector(Velocity);
    GetOwner()->AddActorWorldRotation(RotationDelta);
}

void UGoKartMovementComponent::UpdateLocomotionFromVelocity(float DeltaTime)
{
    FVector Translation = Velocity * DeltaTime;

    if (Translation.X > 10000.9f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Speed delta too big"));
    }
    FHitResult HitResult;
    GetOwner()->AddActorWorldOffset(Translation, true, &HitResult);
    if (HitResult.IsValidBlockingHit())
    {
        Velocity = FVector::ZeroVector;
    }
}