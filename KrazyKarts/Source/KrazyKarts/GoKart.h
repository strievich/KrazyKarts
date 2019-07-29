// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoKart.generated.h"

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

UCLASS()
class KRAZYKARTS_API AGoKart : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGoKart();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

    FGoCartMove CreateMove(float DeltaTime);

    void SimulateMove(FGoCartMove Move);


    FString GetEnumText(ENetRole Role);
    void ApplyRotation(float DeltaTime, float SteeringThrow);

    void UpdateLocomotionFromVelocity(float DeltaTime);

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_SendMove(FGoCartMove Value);

    void MoveForward(float Value);
    void MoveRight(float Value);

    UPROPERTY(EditAnywhere)
    float Mass = 1000; //mass of the car (kg)

    UPROPERTY(EditAnywhere)
    float MaxDrivingForce = 10000000; //(???)

    UPROPERTY(EditAnywhere)
    float MinTurningRadius = 1000; //(cm)

    UPROPERTY(EditAnywhere)
    float DragCoefficient = 16.0f;

    UPROPERTY(EditAnywhere)
    float RollingResistanceCoefficient = 0.015f;
private:

    FVector GetAirResistance();
    FVector GetRollingResistance();


    FVector Velocity = FVector::ZeroVector;
        
    FVector Acceleration = FVector::ZeroVector;;
    
    float Throttle = 0.f;
    float SteeringThrow =0.f;
    
    
    UFUNCTION()
    void OnRep_ReplicatedServerState();

    UPROPERTY(ReplicatedUsing = OnRep_ReplicatedServerState)
    FGoCartState ServerState;

    TArray<FGoCartMove> UnacknowledgedMoves;

    void ClearAcknowledgedMoves(FGoCartMove LastMove);

};
