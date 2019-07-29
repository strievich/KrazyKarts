// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GoKartMovementReplicator.h"
#include "GoKartMovementComponent.generated.h"


FString GetEnumText(ENetRole InRole);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KRAZYKARTS_API UGoKartMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGoKartMovementComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    void SetVelocity(FVector InVelocity) { Velocity = InVelocity; }
    FVector GetVelocity() { return Velocity; }
    void SetAcceleration(FVector InAcceleration) { Acceleration = InAcceleration; }
    void SetThrottle(float InThrottle) { Throttle = InThrottle; }
    void SetSteeringThrow(float InSteeringThrow) { SteeringThrow = InSteeringThrow; }

    FGoCartMove CreateMove(float DeltaTime);
    void SimulateMove(FGoCartMove Move);
private:
    

    
    
    void ApplyRotation(float DeltaTime, float SteeringThrow);

    void UpdateLocomotionFromVelocity(float DeltaTime);

    FVector GetAirResistance();
    FVector GetRollingResistance();


    FVector Velocity = FVector::ZeroVector;

    FVector Acceleration = FVector::ZeroVector;


    float Throttle = 0.f;
    float SteeringThrow = 0.f;


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
		
};
