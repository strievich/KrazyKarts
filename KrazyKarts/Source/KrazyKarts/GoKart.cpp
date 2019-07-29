// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKart.h"
#include "Components/InputComponent.h"
#include "DrawDebugHelpers.h"
#include "UnrealNetwork.h"
#include "GameFramework/GameStateBase.h"

// Sets default values
AGoKart::AGoKart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    MovementComponent = CreateDefaultSubobject<UGoKartMovementComponent>(TEXT("MovementComponent"));
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

    if (MovementComponent == nullptr) return;

    if (ROLE_AutonomousProxy == Role)
    {
        FGoCartMove Move = MovementComponent->CreateMove(DeltaTime);
        MovementComponent->SimulateMove(Move);
        UnacknowledgedMoves.Add(Move);
        Server_SendMove(Move);
    }
    //we are the server and in control of the pawn
    if (Role == ROLE_Authority && GetRemoteRole() == ROLE_SimulatedProxy)
    {
        FGoCartMove Move = MovementComponent->CreateMove(DeltaTime);
        Server_SendMove(Move);
    }
    if (ROLE_SimulatedProxy == Role)
    {
        MovementComponent->SimulateMove(ServerState.LastMove);
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
    MovementComponent->SetThrottle(Value);
    ensure(FMath::Abs(Value) <= 1.0f);
}

void AGoKart::MoveRight(float Value)
{
    MovementComponent->SetSteeringThrow(Value);
    ensure(FMath::Abs(Value) <= 1.0f);
}

void AGoKart::Server_SendMove_Implementation(FGoCartMove Move)
{
    MovementComponent->SimulateMove(Move);

    ServerState.LastMove = Move;
    ServerState.Transform = GetActorTransform();
    ServerState.Velocity = MovementComponent->GetVelocity();
}

bool AGoKart::Server_SendMove_Validate(FGoCartMove Value)
{

    return true;
}

void AGoKart::OnRep_ReplicatedServerState()
{
    UE_LOG(LogTemp, Warning, TEXT("Replicated TransformLocation"));
    SetActorTransform(ServerState.Transform);
    MovementComponent->SetVelocity(ServerState.Velocity);

    ClearAcknowledgedMoves(ServerState.LastMove);

    for (const FGoCartMove& Move : UnacknowledgedMoves)
    {
        MovementComponent->SimulateMove(Move);
    }
}

void AGoKart::ClearAcknowledgedMoves(FGoCartMove LastMove)
{
    TArray<FGoCartMove> NewMoves;

    for (const FGoCartMove& Move : UnacknowledgedMoves)
    {
        if (Move.Time > LastMove.Time)
        {
            NewMoves.Add(Move);
        }
    }
    UnacknowledgedMoves = NewMoves;
}

void AGoKart::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AGoKart, ServerState);
}