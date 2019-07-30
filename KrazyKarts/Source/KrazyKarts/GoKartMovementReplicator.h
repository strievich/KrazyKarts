// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GoKartMovementReplicator.generated.h"


class UGoKartMovementComponent;

USTRUCT()
struct FGoCartMove
{
    GENERATED_BODY()

        UPROPERTY()
        float Throttle;
    UPROPERTY()
        float SteeringThrow;

    UPROPERTY()
        float DeltaTime;
    UPROPERTY()
        float Time;
};

USTRUCT()
struct FGoCartState
{
    GENERATED_BODY()

    UPROPERTY()
    FTransform Transform;
    UPROPERTY()
    FVector Velocity;
    UPROPERTY()
    FGoCartMove LastMove;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KRAZYKARTS_API UGoKartMovementReplicator : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGoKartMovementReplicator();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


private:
    TArray<FGoCartMove> UnacknowledgedMoves;

    void ClearAcknowledgedMoves(FGoCartMove LastMove);

    void ClientTick(float DeltaTime);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_SendMove(FGoCartMove Value);

    UPROPERTY(ReplicatedUsing = OnRep_ServerState)
    FGoCartState ServerState;

    UFUNCTION()
    void OnRep_ServerState();
    void SimulatedProxy_OnRep_ServerState();
    void AutonomousProxy_OnRep_ServerState();

    UPROPERTY()
    UGoKartMovementComponent* MovementComponent;

    void UpdateServerState(const FGoCartMove& Move);

    float ClientTimeSinceUpdate;
    float ClientTimeBetweenLastUpdate;
    FVector ClientStartLocation;

};
