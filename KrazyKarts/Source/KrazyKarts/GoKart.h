// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoKart.generated.h"

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

    FString GetEnumText(ENetRole Role);
    void UpdateSteering(float DeltaTime);

    void UpdateLocomotionFromVelocity(float DeltaTime);

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_MoveForward(float Value);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_MoveRight(float Value);

    void MoveForward(float Value);
    void MoveRight(float Value);

    UPROPERTY(EditAnywhere)
    float Mass = 1000; //mass of the car (kg)

    UPROPERTY(EditAnywhere)
    float MaxDrivingForce = 10000000; //(???)

    UPROPERTY(EditAnywhere)
    float MinTurningRadius = 10000; //(cm)

    UPROPERTY(EditAnywhere)
    float DragCoefficient = 16.0f;

    UPROPERTY(EditAnywhere)
    float RollingResistanceCoefficient = 0.015f;
private:

    FVector GetAirResistance();
    FVector GetRollingResistance();
    FVector CalcualtedVelocity = FVector::ZeroVector;
    FVector CalculatedAcceleration = FVector::ZeroVector;;
    TOptional<float> ThrottleValue = 0.f;
    float SteeringThrow =0.f;
    

};
