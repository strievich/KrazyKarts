// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKart.h"
#include "Components/InputComponent.h"
#include "DrawDebugHelpers.h"

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
    FVector Force = GetActorForwardVector() * MaxDrivingForce * ThrottleValue.Get(0.0f);
    if (ThrottleValue.GetValue() > 10000.9f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Throtle delta too big"));
    }

    Force += GetAirResistance();
    Force += GetRollingResistance();

    CalculatedAcceleration =  Force / Mass;
    CalcualtedVelocity += CalculatedAcceleration*DeltaTime;

    UpdateSteering(DeltaTime);
    UpdateLocomotionFromVelocity(DeltaTime);

    DrawDebugString(GetWorld(), FVector(0,0,100), GetEnumText(Role.GetValue()), this, FColor::White, DeltaTime, true);
}

FString AGoKart::GetEnumText(ENetRole Role)
{
    switch (Role)
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

void AGoKart::UpdateSteering(float DeltaTime)
{
    float DeltaLocation = FVector::DotProduct(GetActorForwardVector(), CalcualtedVelocity)  *DeltaTime;


    float RotationAngle = (DeltaLocation / MinTurningRadius) * SteeringThrow;
    FQuat RotationDelta(GetActorUpVector(), RotationAngle);


    //DrawDebugString(GetWorld(), FVector::ZeroVector , *FString::Printf(TEXT("Rotation delta[%s]"), *RotationDelta.ToString()), this,FColor::White,0,true);
    CalcualtedVelocity = RotationDelta.RotateVector(CalcualtedVelocity);
//     if (CalcualtedVelocity.X > 100000.9f)
//     {
//         UE_LOG(LogTemp, Warning, TEXT("Speed delta too big"));
//     }
    AddActorWorldRotation(RotationDelta);
}

void AGoKart::UpdateLocomotionFromVelocity(float DeltaTime)
{
    FVector Translation = CalcualtedVelocity * DeltaTime;

    //DrawDebugString(GetWorld(), GetActorUpVector()*50.f, *FString::Printf(TEXT("Transation delta[%s]"), *Translation.ToString()), this, FColor::White, 0, true);

    if (Translation.X > 10000.9f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Speed delta too big"));
    }
    FHitResult HitResult;
    AddActorWorldOffset(Translation, true, &HitResult);
    if (HitResult.IsValidBlockingHit())
    {
        CalcualtedVelocity = FVector::ZeroVector;
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
    ThrottleValue = Value;
    Server_MoveForward(Value);
}

void AGoKart::MoveRight(float Value)
{
    SteeringThrow = Value;
    Server_MoveRight(Value);
}

void AGoKart::Server_MoveForward_Implementation(float Value)
{
    ThrottleValue = Value;
    UE_LOG(LogTemp, Warning, TEXT("Throtle = %d"), ThrottleValue.GetValue());
    if (ThrottleValue.GetValue() > 10000.9f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Trottle Value TOOO BIG:  %d"), Value);
    }

}

bool AGoKart::Server_MoveForward_Validate(float Value)
{
    if(FMath::Abs(Value)>1) 
    {
        UE_LOG(LogTemp, Error, TEXT("Server_MoveForward_Validate(float Value) too big"));
        return false;
    }

    return true;
}

void AGoKart::Server_MoveRight_Implementation(float Value)
{
    SteeringThrow = Value;
}

bool AGoKart::Server_MoveRight_Validate(float Value)
{
    if (FMath::Abs(Value) > 1)
    {
        UE_LOG(LogTemp, Error, TEXT("Server_MoveRight_Validate(float Value) too big"));
        return false;
    }

    return true;
}

FVector AGoKart::GetAirResistance()
{
    return -CalcualtedVelocity.GetSafeNormal() * CalcualtedVelocity.SizeSquared() * DragCoefficient/100.f;
}

FVector AGoKart::GetRollingResistance()
{
    float AccelerationDueToGravity = -GetWorld()->GetGravityZ()/100.0f;
    float NormalForce = Mass * AccelerationDueToGravity;
    return -CalcualtedVelocity.GetSafeNormal()*RollingResistanceCoefficient*NormalForce;
}

