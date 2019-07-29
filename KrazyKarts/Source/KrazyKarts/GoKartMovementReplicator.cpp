// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKartMovementReplicator.h"
#include "GoKartMovementComponent.h"
#include "UnrealNetwork.h"


// Sets default values for this component's properties
UGoKartMovementReplicator::UGoKartMovementReplicator()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

    SetIsReplicated(true);
}

// Called when the game starts
void UGoKartMovementReplicator::BeginPlay()
{
	Super::BeginPlay();

	// ...
    MovementComponent = GetOwner()->FindComponentByClass<UGoKartMovementComponent>();
    ensure(MovementComponent != nullptr);
}

// Called every frame
void UGoKartMovementReplicator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (MovementComponent == nullptr) return;

    if (ROLE_AutonomousProxy == GetOwnerRole())
    {
        FGoCartMove Move = MovementComponent->CreateMove(DeltaTime);
        MovementComponent->SimulateMove(Move);
        UnacknowledgedMoves.Add(Move);
        Server_SendMove(Move);
    }
    //we are the server and in control of the pawn
    if (GetOwnerRole() == ROLE_Authority && GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
    {
        FGoCartMove Move = MovementComponent->CreateMove(DeltaTime);
        Server_SendMove(Move);
    }
    if (ROLE_SimulatedProxy == GetOwnerRole())
    {
        MovementComponent->SimulateMove(ServerState.LastMove);
    }
}
void UGoKartMovementReplicator::Server_SendMove_Implementation(FGoCartMove Move)
{
    MovementComponent->SimulateMove(Move);

    ServerState.LastMove = Move;
    ServerState.Transform = GetOwner()->GetActorTransform();
    ServerState.Velocity = MovementComponent->GetVelocity();
}

bool UGoKartMovementReplicator::Server_SendMove_Validate(FGoCartMove Value)
{
    return true;
}

void UGoKartMovementReplicator::OnRep_ReplicatedServerState()
{
    UE_LOG(LogTemp, Warning, TEXT("Replicated TransformLocation"));
    if (MovementComponent == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("MovementComponent == nullptr"));
        return;
    }
    GetOwner()->SetActorTransform(ServerState.Transform);
    MovementComponent->SetVelocity(ServerState.Velocity);

    ClearAcknowledgedMoves(ServerState.LastMove);

    for (const FGoCartMove& Move : UnacknowledgedMoves)
    {
        MovementComponent->SimulateMove(Move);
    }
}

void UGoKartMovementReplicator::ClearAcknowledgedMoves(FGoCartMove LastMove)
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

void UGoKartMovementReplicator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UGoKartMovementReplicator, ServerState);
}
