// Fill out your copyright notice in the Description page of Project Settings.

#include "OP_PlayerCam.h"
#include "GameFramework/PlayerController.h"


// Sets default values
AOP_PlayerCam::AOP_PlayerCam()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


}

// Called when the game starts or when spawned
void AOP_PlayerCam::BeginPlay()
{
	Super::BeginPlay();
	
	PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("OP_PlayerCam failed to retrieve a PlayerController"));
	}
	else
	{
		PlayerController->bShowMouseCursor = true;
	}
}

void AOP_PlayerCam::HandleTranslateForward(float val)
{
	MovementInput += (GetActorForwardVector() * val);
}

void AOP_PlayerCam::HandleTranslateRight(float val)
{
	MovementInput += (GetActorRightVector() * val);
}

void AOP_PlayerCam::HandleTranslateUp(float val)
{
	MovementInput += (GetActorUpVector() * val);
}

void AOP_PlayerCam::HandleRoll(float val)
{

	//RotationInput.Roll += val;
	FRotator rot = FRotator::ZeroRotator;
	rot.Roll += val;
	AddActorLocalRotation(rot);
}

void AOP_PlayerCam::HandlePitch(float val)
{
	if (!CameraLocked) return;

	//RotationInput.Pitch += val;
	FRotator rot = FRotator::ZeroRotator;
	rot.Pitch += val;
	AddActorLocalRotation(rot);
}

void AOP_PlayerCam::HandleYaw(float val)
{
	if (!CameraLocked) return;

	FRotator rot = FRotator::ZeroRotator;
	rot.Yaw += val;
	AddActorLocalRotation(rot);
}

void AOP_PlayerCam::HandleThrottle(float val)
{
	CurrentThrottle += val;
}

void AOP_PlayerCam::HandleCamLock()
{
	CameraLocked = true;

	if (PlayerController)
	{
		PlayerController->bShowMouseCursor = false;
		float x, y;
		PlayerController->GetMousePosition(x, y);
		MousePos = FVector2D(x, y);
	}
}

void AOP_PlayerCam::HandleCamUnlock()
{
	CameraLocked = false;
	if (PlayerController)
	{
		PlayerController->bShowMouseCursor = true;
	}
}

// Called every frame
void AOP_PlayerCam::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetActorLocation(GetActorLocation() + (MovementInput * GetCurrentSpeed()));
	//SetActorRotation(RotationInput);
	
	MovementInput = FVector::ZeroVector;
	//RotationInput = FRotator::ZeroRotator;

	if (CameraLocked && PlayerController)
	{
		PlayerController->SetMouseLocation(MousePos.X, MousePos.Y);
	}
}

// Called to bind functionality to input
void AOP_PlayerCam::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("OPTranslateForward", this, &AOP_PlayerCam::HandleTranslateForward);
	PlayerInputComponent->BindAxis("OPTranslateRight", this, &AOP_PlayerCam::HandleTranslateRight);
	PlayerInputComponent->BindAxis("OPTranslateUp", this, &AOP_PlayerCam::HandleTranslateUp);

	PlayerInputComponent->BindAxis("OPRoll", this, &AOP_PlayerCam::HandleRoll);
	PlayerInputComponent->BindAxis("OPPitch", this, &AOP_PlayerCam::HandlePitch);
	PlayerInputComponent->BindAxis("OPYaw", this, &AOP_PlayerCam::HandleYaw);

	PlayerInputComponent->BindAxis("OPThrottle", this, &AOP_PlayerCam::HandleThrottle);

	PlayerInputComponent->BindAction("OPCamLock", IE_Pressed, this, &AOP_PlayerCam::HandleCamLock);
	PlayerInputComponent->BindAction("OPCamLock", IE_Released, this, &AOP_PlayerCam::HandleCamUnlock);
}

