// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "OP_PlayerCam.generated.h"

UCLASS()
class ORBITPLANETARIUM_API AOP_PlayerCam : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AOP_PlayerCam();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void HandleTranslateForward(float val);
	void HandleTranslateRight(float val);
	void HandleTranslateUp(float val);

	void HandleRoll(float val);
	void HandlePitch(float val);
	void HandleYaw(float val);

	void HandleThrottle(float val);

	void HandleCamLock();
	void HandleCamUnlock();

	FVector MovementInput;
	FRotator RotationInput;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement)
	float CurrentThrottle = 1.0f;

	UPROPERTY(EditAnywhere, Category = Movement)
	float BaseSpeed = 1.2f;

	// Is the player moving the camera
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera)
	bool CameraLocked = false;

	// Previous position of the mouse
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera)
	FVector2D MousePos;
	
	// Stored reference to player controller for controlling mouse
	UPROPERTY()
	class APlayerController* PlayerController;

	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetCurrentSpeed() { return FMath::Pow(BaseSpeed, CurrentThrottle); }

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	
	
};
