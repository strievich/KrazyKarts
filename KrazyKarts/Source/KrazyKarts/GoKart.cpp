// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKart.h"
#include "Components/InputComponent.h"
#include "DrawDebugHelpers.h"
#include "UnrealNetwork.h"

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
	
    if (HasAuthority())
    {
        NetUpdateFrequency = 1;
    }
}

// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (IsLocallyControlled())
    {
        FGoCartMove Move;
        Move.DeltaTime = DeltaTime;
        Move.SteeringThrow = SteeringThrow;
        Move.Throttle = Throttle;
        //TODO:set time
        Server_SendMove(Move);
        SimulateMove(Move);
    }
}

void AGoKart::SimulateMove(FGoCartMove Move)
{
    FVector Force = GetActorForwardVector() * MaxDrivingForce * Move.Throttle;

    Force += GetAirResistance();
    Force += GetRollingResistance();

    Acceleration = Force / Mass;
    Velocity += Acceleration * Move.DeltaTime;

    ApplyRotation(Move.DeltaTime, Move.SteeringThrow);
    UpdateLocomotionFromVelocity(Move.DeltaTime);

    DrawDebugString(GetWorld(), FVector(0, 0, 100), GetEnumText(Role.GetValue()), this, FColor::White, Move.DeltaTime, true);
}

FString AGoKart::GetEnumText(ENetRole InRole)
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

void AGoKart::ApplyRotation(float DeltaTime, float InSteeringThrow)
{
    float DeltaLocation = FVector::DotProduct(GetActorForwardVector(), Velocity)  *DeltaTime;


    float RotationAngle = (DeltaLocation / MinTurningRadius) * InSteeringThrow;
    FQuat RotationDelta(GetActorUpVector(), RotationAngle);

    Velocity = RotationDelta.RotateVector(Velocity);
    AddActorWorldRotation(RotationDelta);
}

void AGoKart::UpdateLocomotionFromVelocity(float DeltaTime)
{
    FVector Translation = Velocity * DeltaTime;

    if (Translation.X > 10000.9f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Speed delta too big"));
    }
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
    ensure(FMath::Abs(Value) <= 1.0f);
}

void AGoKart::MoveRight(float Value)
{
    SteeringThrow = Value;
    ensure(FMath::Abs(Value) <= 1.0f);
}

void AGoKart::Server_SendMove_Implementation(FGoCartMove Move)
{
    SimulateMove(Move);

    ServerState.LastMove = Move;
    ServerState.Transform = GetActorTransform();
    ServerState.Velocity = Velocity;
}

bool AGoKart::Server_SendMove_Validate(FGoCartMove Value)
{

    return true;
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

void AGoKart::OnRep_ReplicatedServerState()
{
    UE_LOG(LogTemp, Warning, TEXT("Replicated TransformLocation"));
    SetActorTransform(ServerState.Transform);
    Velocity = ServerState.Velocity;
}

void AGoKart::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AGoKart, ServerState);
}