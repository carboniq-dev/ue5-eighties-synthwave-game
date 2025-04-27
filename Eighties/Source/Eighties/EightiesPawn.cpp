#include "EightiesPawn.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "ChaosWheeledVehicleMovementComponent.h"

#define LOCTEXT_NAMESPACE "VehiclePawn"

DEFINE_LOG_CATEGORY(LogTemplateVehicle);

AEightiesPawn::AEightiesPawn()
{
	FrontSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Front Spring Arm"));
	FrontSpringArm->SetupAttachment(GetMesh());
	FrontSpringArm->TargetArmLength = 0.0f;
	FrontSpringArm->bDoCollisionTest = false;
	FrontSpringArm->bEnableCameraRotationLag = true;
	FrontSpringArm->CameraRotationLagSpeed = 15.0f;
	FrontSpringArm->SetRelativeLocation(FVector(30.0f, 0.0f, 120.0f));

	FrontCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Front Camera"));
	FrontCamera->SetupAttachment(FrontSpringArm);
	FrontCamera->bAutoActivate = false;

	BackSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Back Spring Arm"));
	BackSpringArm->SetupAttachment(GetMesh());
	BackSpringArm->TargetArmLength = 650.0f;
	BackSpringArm->SocketOffset.Z = 150.0f;
	BackSpringArm->bDoCollisionTest = false;
	BackSpringArm->bInheritPitch = false;
	BackSpringArm->bInheritRoll = false;
	BackSpringArm->bEnableCameraRotationLag = true;
	BackSpringArm->CameraRotationLagSpeed = 2.0f;
	BackSpringArm->CameraLagMaxDistance = 50.0f;

	BackCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Back Camera"));
	BackCamera->SetupAttachment(BackSpringArm);

	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(FName("Vehicle"));

	ChaosVehicleMovement = CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());

}

void AEightiesPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// steering 
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Triggered, this, &AEightiesPawn::Steering);
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Completed, this, &AEightiesPawn::Steering);

		// throttle 
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &AEightiesPawn::Throttle);
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Completed, this, &AEightiesPawn::Throttle);

		// break 
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &AEightiesPawn::Brake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Started, this, &AEightiesPawn::StartBrake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Completed, this, &AEightiesPawn::StopBrake);

		// handbrake 
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Started, this, &AEightiesPawn::StartHandbrake);
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &AEightiesPawn::StopHandbrake);

		// look around 
		EnhancedInputComponent->BindAction(LookAroundAction, ETriggerEvent::Triggered, this, &AEightiesPawn::LookAround);

		// toggle camera 
		EnhancedInputComponent->BindAction(ToggleCameraAction, ETriggerEvent::Triggered, this, &AEightiesPawn::ToggleCamera);

		// reset the vehicle 
		EnhancedInputComponent->BindAction(ResetVehicleAction, ETriggerEvent::Triggered, this, &AEightiesPawn::ResetVehicle);

		// turbo
		EnhancedInputComponent->BindAction(TurboAction, ETriggerEvent::Started,
								   this, &AEightiesPawn::StartTurbo);
		EnhancedInputComponent->BindAction(TurboAction, ETriggerEvent::Completed,
										   this, &AEightiesPawn::StopTurbo);
	}
	else
	{
		UE_LOG(LogTemplateVehicle, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AEightiesPawn::Tick(float Delta)
{
	Super::Tick(Delta);

	bool bMovingOnGround = ChaosVehicleMovement->IsMovingOnGround();
	GetMesh()->SetAngularDamping(bMovingOnGround ? 0.0f : 3.0f);

	float CameraYaw = BackSpringArm->GetRelativeRotation().Yaw;
	CameraYaw = FMath::FInterpTo(CameraYaw, 0.0f, Delta, 1.0f);

	BackSpringArm->SetRelativeRotation(FRotator(0.0f, CameraYaw, 0.0f));
}

void AEightiesPawn::StartTurbo(const FInputActionValue&)
{
	if (bTurboActive || !ChaosVehicleMovement) return;

	StockTorque = ChaosVehicleMovement->EngineSetup.MaxTorque;
	StockDrag   = ChaosVehicleMovement->DragCoefficient;

	ChaosVehicleMovement->SetMaxEngineTorque(StockTorque * 1.5f);  
	ChaosVehicleMovement->SetDragCoefficient(StockDrag * 0.6f);   

	bTurboActive = true;
}

void AEightiesPawn::StopTurbo(const FInputActionValue&)
{
	if (!bTurboActive || !ChaosVehicleMovement) return;

	ChaosVehicleMovement->SetMaxEngineTorque(StockTorque);
	ChaosVehicleMovement->SetDragCoefficient(StockDrag);

	bTurboActive = false;
}

void AEightiesPawn::Steering(const FInputActionValue& Value)
{
	float SteeringValue = Value.Get<float>();

	ChaosVehicleMovement->SetSteeringInput(SteeringValue);
}

void AEightiesPawn::Throttle(const FInputActionValue& Value)
{
	float ThrottleValue = Value.Get<float>();

	ChaosVehicleMovement->SetThrottleInput(ThrottleValue);
}

void AEightiesPawn::Brake(const FInputActionValue& Value)
{
	float BreakValue = Value.Get<float>();

	ChaosVehicleMovement->SetBrakeInput(BreakValue);
}

void AEightiesPawn::StartBrake(const FInputActionValue& Value)
{
	BrakeLights(true);
}

void AEightiesPawn::StopBrake(const FInputActionValue& Value)
{
	BrakeLights(false);

	ChaosVehicleMovement->SetBrakeInput(0.0f);
}

void AEightiesPawn::StartHandbrake(const FInputActionValue& Value)
{
	ChaosVehicleMovement->SetHandbrakeInput(true);

	BrakeLights(true);
}

void AEightiesPawn::StopHandbrake(const FInputActionValue& Value)
{
	ChaosVehicleMovement->SetHandbrakeInput(false);

	BrakeLights(false);
}

void AEightiesPawn::LookAround(const FInputActionValue& Value)
{
	float LookValue = Value.Get<float>();

	BackSpringArm->AddLocalRotation(FRotator(0.0f, LookValue, 0.0f));
}

void AEightiesPawn::ToggleCamera(const FInputActionValue& Value)
{
	bFrontCameraActive = !bFrontCameraActive;

	FrontCamera->SetActive(bFrontCameraActive);
	BackCamera->SetActive(!bFrontCameraActive);
}

void AEightiesPawn::ResetVehicle(const FInputActionValue& Value)
{
	FVector ResetLocation = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);

	FRotator ResetRotation = GetActorRotation();
	ResetRotation.Pitch = 0.0f;
	ResetRotation.Roll = 0.0f;
	
	SetActorTransform(FTransform(ResetRotation, ResetLocation, FVector::OneVector), false, nullptr, ETeleportType::TeleportPhysics);

	GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	GetMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector);

	UE_LOG(LogTemplateVehicle, Error, TEXT("Reset Vehicle"));
}

#undef LOCTEXT_NAMESPACE