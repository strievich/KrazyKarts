// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKartMovementReplicator.h"
#include "GoKartMovementComponent.h"
#include "UnrealNetwork.h"
#include "Components/ActorComponent.h"


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

    FGoCartMove LastMove = MovementComponent->GetLastMove();

    if (GetOwnerRole() == ROLE_AutonomousProxy)
    {
        UnacknowledgedMoves.Add(LastMove);
        Server_SendMove(LastMove);
    }
    //we are the server and in control of the pawn
    if (GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
    {
        UpdateServerState(LastMove);
    }
    if (ROLE_SimulatedProxy == GetOwnerRole())
    {
        ClientTick(DeltaTime);
    }
}
void UGoKartMovementReplicator::Server_SendMove_Implementation(FGoCartMove Move)
{
    MovementComponent->SimulateMove(Move);

    UpdateServerState(Move);
}

bool UGoKartMovementReplicator::Server_SendMove_Validate(FGoCartMove Value)
{
    return true;
}

void UGoKartMovementReplicator::OnRep_ServerState()
{
    switch (GetOwnerRole())
    {
    case ROLE_AutonomousProxy:
        AutonomousProxy_OnRep_ServerState();
        break;
    case ROLE_SimulatedProxy:
        SimulatedProxy_OnRep_ServerState();
        break;
    default:
        break;
    }
}

void UGoKartMovementReplicator::SimulatedProxy_OnRep_ServerState()
{
    if (MovementComponent == nullptr) return;

    ClientTimeBetweenLastUpdate = ClientTimeSinceUpdate;
    ClientTimeSinceUpdate = 0;
    ClientStartTrasform = GetOwner()->GetActorTransform();
    
    if (MeshOffsetRoot != nullptr)
    {
        ClientStartTrasform.SetLocation(MeshOffsetRoot->GetComponentLocation());
        ClientStartTrasform.SetRotation(MeshOffsetRoot->GetComponentQuat());

    }

    ClientStartVelocity = MovementComponent->GetVelocity();

    GetOwner()->SetActorTransform(ServerState.Transform);
}

void UGoKartMovementReplicator::AutonomousProxy_OnRep_ServerState()
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

void UGoKartMovementReplicator::SetMeshOffsetRoot(USceneComponent* Root)
{
    MeshOffsetRoot = Root;
}

void UGoKartMovementReplicator::UpdateServerState(const FGoCartMove& Move)
{
    ServerState.LastMove = Move;
    ServerState.Transform = GetOwner()->GetActorTransform();
    ServerState.Velocity = MovementComponent->GetVelocity();
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

void UGoKartMovementReplicator::ClientTick(float DeltaTime)
{
    ClientTimeSinceUpdate += DeltaTime;
    if (MovementComponent == nullptr) return;

    if (ClientTimeBetweenLastUpdate < KINDA_SMALL_NUMBER) return;

    float LerpRatio = ClientTimeSinceUpdate / ClientTimeBetweenLastUpdate;

    FHermitCubicSpline Spline = CreateSpline();

    FVector NewLocation = Spline.InterpolateLocation(LerpRatio);
    FVector NewDerivative = Spline.InterpolateDerivative(LerpRatio);

    FVector NewVelocity = NewDerivative / ClientTimeBetweenLastUpdate;
    MovementComponent->SetVelocity(NewVelocity);

    FQuat TargetRotatation = ServerState.Transform.GetRotation();
    FQuat StartRotation = ClientStartTrasform.GetRotation();
    FQuat NewRotation = FQuat::Slerp(StartRotation, TargetRotatation, LerpRatio);


    if (MeshOffsetRoot != nullptr)
    {
        MeshOffsetRoot->SetWorldLocation(NewLocation);
        MeshOffsetRoot->SetWorldRotation(NewRotation);
    }
    else
    {
        GetOwner()->SetActorLocation(NewLocation);
        GetOwner()->SetActorRotation(NewRotation);
    }
    
}

FHermitCubicSpline UGoKartMovementReplicator::CreateSpline()
{
    FHermitCubicSpline Spline;
    Spline.TargetLocation = ServerState.Transform.GetLocation();
    Spline.StartLocation = ClientStartTrasform.GetLocation();
    Spline.StartDerivative = ClientStartVelocity * ClientTimeBetweenLastUpdate;
    Spline.TargetDerivative = ServerState.Velocity*ClientTimeBetweenLastUpdate;
    return Spline;
}

void UGoKartMovementReplicator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UGoKartMovementReplicator, ServerState);
}
