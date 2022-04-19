// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraShakeBase.h"
#include "Components/SceneComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"

#include "DrawDebugHelpers.h"

// Sets default values
ABaseCharacter::ABaseCharacter() :
	// Setup
	DefaultFOV(110.f),
	SlideFOVOffset(5.f),
	SlideFOV(0.f),
	SprintSpeed(617.22f),
	WalkSpeed(412.75f),
	CrouchSpeed(203.2f),
	WallrunSpeed(863.6f),
	bAutoSprint(true),
	CapsuleInterpSpeed(10.f),
	CrouchCapsuleHalfHeight(60.f),
	SlideBoostResetTime(2.f),
	SlideBoostForce(200.f),
	SlideCameraTiltAngle(15.f),
	SlideGroundFriction(.1f),
	SlideBrakingDeceleration(200.f),
	JumpZForce(625.f),
	InstantJumpMultiplier(.88f),
	// Status
	bInputForward(false),
	bPrevInputForward(false),
	bInputBackward(false),
	bInputRight(false),
	bInputLeft(false),
	bIsAccelForward(false),
	bIsSprinting(false),
	bWalkSprintInput(false),
	bIsWallrunning(false),
	bIsCrouching(false),
	bIsSliding(false),
	bCanSlideBoost(true),
	bIsJumping(false),
	bCanMaxJump(true),
	bCanDoubleJump(true),
	MovementStatus(EMovementStatus::MS_Land),
	SlideDirection(FVector::ZeroVector),
	DefaultMaxAcceleration(4096)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraPitchControlBase = CreateDefaultSubobject<USpringArmComponent>(FName("CameraPitchControlBase"));
	CameraPitchControlBase->SetupAttachment(GetRootComponent());
	CameraPitchControlBase->TargetArmLength = 0.f;
	CameraPitchControlBase->bUsePawnControlRotation = true;
	CameraPitchControlBase->SetRelativeLocation(FVector(0.f, 0.f, 30.f));

	CameraTiltControlBase = CreateDefaultSubobject<USceneComponent>(FName("CameraTiltControlBase"));
	CameraTiltControlBase->SetupAttachment(CameraPitchControlBase);
	CameraTiltControlBase->SetRelativeLocation(FVector(0.f, 0.f, 20.f));

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(FName("CameraComponent"));
	CameraComponent->SetupAttachment(CameraTiltControlBase);
	CameraComponent->FieldOfView = DefaultFOV;

	Arm = CreateDefaultSubobject<USkeletalMeshComponent>(FName("Arm"));
	Arm->SetupAttachment(CameraComponent);

	// CharacterMovementComponent Setup
	GetCharacterMovement()->SetCrouchedHalfHeight(50.f);
	GetCharacterMovement()->MaxAcceleration = 4096.f;
	GetCharacterMovement()->PerchRadiusThreshold = 25.f;
	GetCharacterMovement()->bUseFlatBaseForFloorChecks = true;
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	SlideFOV = DefaultFOV + SlideFOVOffset;
	SetMovementStatus(EMovementStatus::MS_Land);
	GetCharacterMovement()->JumpZVelocity = JumpZForce;
	// GetCharacterMovement()->bCrouchMaintainsBaseLocation = true;

	DefaultCapsuleHalfHeight = GetDefaultHalfHeight();
	DefaultGroundFirction = GetCharacterMovement()->GroundFriction;
	DefaultBrakingDeceleration = GetCharacterMovement()->BrakingDecelerationWalking;
}

void ABaseCharacter::MoveForward()
{
	bInputForward = true;
	if (bAutoSprint && MovementStatus == EMovementStatus::MS_Land)
	{
		bIsSprinting = true;
	}
}

void ABaseCharacter::MoveForwardStop()
{
	bInputForward = false;
	bIsSprinting = false;
}

void ABaseCharacter::MoveBackward()
{
	bInputBackward = true;
}

void ABaseCharacter::MoveBackwardStop()
{
	bInputBackward = false;
}

void ABaseCharacter::MoveRight()
{
	bInputRight = true;
}

void ABaseCharacter::MoveRightStop()
{
	bInputRight = false;
}

void ABaseCharacter::MoveLeft()
{
	bInputLeft = true;
}

void ABaseCharacter::MoveLeftStop()
{
	bInputLeft = false;
}

void ABaseCharacter::MoveForwardAxis(float Value)
{
	bIsAccelForward = Value > 0.f ? true : false;
	if (bAutoSprint && !bWalkSprintInput && !bIsCrouching)
	{
		bIsSprinting = Value >= 0.95f ? true : false;
	}
	AddMovementInput(GetActorForwardVector(), Value);
}

void ABaseCharacter::MoveRightAxis(float Value)
{
	AddMovementInput(GetActorRightVector(), Value);
}

void ABaseCharacter::FireWeapon_Implementation() {}

void ABaseCharacter::CustomJump()
{
	if (!CustomCanJump())
	{
		return;
	}

	FVector JumpDirection;
	if (MovementStatus == EMovementStatus::MS_Wallrun)
	{
		// Wall jump
		// TODO: Calc JumpDirection
	}
	else
	{
		JumpDirection.Z = GetCharacterMovement()->JumpZVelocity;
		if (MovementStatus == EMovementStatus::MS_Fall || MovementStatus == EMovementStatus::MS_JumpBeforeApex)
		{
			// DoubleJump
			bCanDoubleJump = false;
		}
		else
		{
			// Jump from floor
			if (!bCanMaxJump)
			{
				JumpDirection.Z *= InstantJumpMultiplier;
			}
			if (GetWorldTimerManager().IsTimerActive(MaxJumpTimer))
			{
				GetWorldTimerManager().ClearTimer(MaxJumpTimer);
			}
		}
	}
	LaunchCharacter(JumpDirection, false, true);
	SetMovementStatus(EMovementStatus::MS_JumpBeforeApex);
	GetCharacterMovement()->bNotifyApex = true;
	bIsJumping = true;
	bCanMaxJump = false;
	bIsSprinting = false;
	if (bIsSliding)
	{
		StopSlide();
	}

	ShakeCamera();
}

void ABaseCharacter::ActivateMaxJump()
{
	bCanMaxJump = true;
}

bool ABaseCharacter::CustomCanJump() const
{
	if (MovementStatus == EMovementStatus::MS_Fall)
	{
		return bCanDoubleJump;
	}
	return true;
}

void ABaseCharacter::CustomStartCrouch()
{
	bIsCrouching = true;
	bIsSprinting = false;

	if (CanSlide())
	{
		StartSlide();
	}
}

void ABaseCharacter::CustomStopCrouch()
{
	bIsCrouching = false;

	if (bIsSliding)
	{
		StopSlide();
	}
}

void ABaseCharacter::SprintOrWalk()
{
	bWalkSprintInput = !bWalkSprintInput;
	if (bInputForward)
	{
		bIsSprinting = !bIsSprinting;
	}
}

void ABaseCharacter::SetMovementStatus(EMovementStatus NewStatus)
{
	switch (NewStatus)
	{
	case EMovementStatus::MS_Land:
		break;
	case EMovementStatus::MS_Wallrun:
		break;
	case EMovementStatus::MS_Fall:
		break;
	case EMovementStatus::DefaultMax:
		break;
	}

	MovementStatus = NewStatus;
}

float ABaseCharacter::GetCurrentMaxSpeed() const
{
	if (bIsCrouching)
	{
		return CrouchSpeed;
	}
	else
	{
		if (bIsWallrunning)
		{
			return WallrunSpeed;
		}
		return bIsSprinting ? SprintSpeed : WalkSpeed;
	}
}

void ABaseCharacter::InterpCapsuleHalfHeight(float DeltaTime)
{
	const float CurrentHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	float TargetHalfHeight;
	if (bIsCrouching)
	{
		TargetHalfHeight = CrouchCapsuleHalfHeight;
	}
	else
	{
		TargetHalfHeight = DefaultCapsuleHalfHeight;
	}

	const float NextHalfHeight = FMath::FInterpTo(CurrentHalfHeight, TargetHalfHeight, DeltaTime, CapsuleInterpSpeed);
	const float DeltaHalfHeight = NextHalfHeight - CurrentHalfHeight;
	GetCapsuleComponent()->SetCapsuleHalfHeight(NextHalfHeight);

	CameraPitchControlBase->AddLocalOffset(FVector(0.f, 0.f, DeltaHalfHeight));

	// TODO: Set WallrunDetector height
}

void ABaseCharacter::ReachedJumpApex()
{
	UE_LOG(LogTemp, Warning, TEXT("In Apex"));
	SetMovementStatus(EMovementStatus::MS_Fall);
}

bool ABaseCharacter::CanSlide() const
{
	if (bIsCrouching && MovementStatus == EMovementStatus::MS_Land && GetVelocityXYKPH() > 20.f)
	{
		return true;
	}
	return false;
}

void ABaseCharacter::StartSlide()
{
	if (bIsSliding)
	{
		return;
	}

	// TODO: Reduce input accel
	GetCharacterMovement()->MaxAcceleration *= GetCharacterMovement()->AirControl;
	GetCharacterMovement()->GroundFriction = SlideGroundFriction;
	GetCharacterMovement()->BrakingDecelerationWalking = SlideBrakingDeceleration;

	SlideDirection = GetCharacterMovement()->GetLastUpdateVelocity();
	SlideDirection.Z = 0.f;
	SlideDirection.Normalize();
	bIsSliding = true;

	if (bCanSlideBoost)
	{
		GetCharacterMovement()->AddImpulse(
			SlideDirection * SlideBoostForce,
			true
		);
		bCanSlideBoost = false;
		UE_LOG(LogTemp, Warning, TEXT("Slide Boost applied"));
	}
	else if (GetWorldTimerManager().IsTimerActive(SlideBoostResetTimer))
	{
		GetWorldTimerManager().ClearTimer(SlideBoostResetTimer);
	}
}

void ABaseCharacter::StopSlide()
{
	if (!bIsSliding)
	{
		return;
	}

	GetWorldTimerManager().SetTimer(
		SlideBoostResetTimer,
		this,
		&ABaseCharacter::ActivateSlideBoost,
		SlideBoostResetTime
	);

	GetCharacterMovement()->MaxAcceleration = DefaultMaxAcceleration;
	GetCharacterMovement()->GroundFriction = DefaultGroundFirction;
	GetCharacterMovement()->BrakingDecelerationWalking = DefaultBrakingDeceleration;

	SlideDirection = FVector::ZeroVector;
	bIsSliding = false;
}

void ABaseCharacter::ActivateSlideBoost()
{
	bCanSlideBoost = true;
}

void ABaseCharacter::TiltCamera(float DeltaTime)
{
	if (0)
	{
		return;
	}

	float TiltValue;
	if (bIsSliding && SlideDirection.Length() > 0.f)
	{
		FVector LookRightVector = GetActorRightVector();
		LookRightVector.Normalize();
		const float TiltAmountBasedOnLookDirection =
			FVector::DotProduct(LookRightVector, SlideDirection) *
			SlideCameraTiltAngle * -1;
		const float VelocityMultiplier = FMath::Clamp((GetVelocityKPH() - 10.f) / 20.f, 0.f, 1.f);
		float TargetTiltValue = TiltAmountBasedOnLookDirection * VelocityMultiplier;

		TiltValue = FMath::FInterpTo(
			CameraTiltControlBase->GetRelativeRotation().Roll,
			TargetTiltValue,
			DeltaTime,
			20.f
		);
	}
	else
	{
		TiltValue = FMath::FInterpTo(
			CameraTiltControlBase->GetRelativeRotation().Roll,
			0.f,
			DeltaTime,
			20.f
		);
	}

	CameraTiltControlBase->SetRelativeRotation(FRotator(0.f, 0.f, TiltValue));
}

void ABaseCharacter::ChangeFOV(float DeltaTime)
{
	float FOVValue;
	if (bIsSliding)
	{
		FOVValue = FMath::FInterpTo(
			CameraComponent->FieldOfView,
			SlideFOV,
			DeltaTime,
			10.f
		);
	}
	else
	{
		FOVValue = FMath::FInterpTo(
			CameraComponent->FieldOfView,
			DefaultFOV,
			DeltaTime,
			10.f
		);
	}
	CameraComponent->FieldOfView = FOVValue;
}

void ABaseCharacter::ShakeCamera()
{
	if (JumpLandCameraShake)
	{
		GetWorld()->GetFirstPlayerController()->PlayerCameraManager->PlayWorldCameraShake(
			GetWorld(),
			JumpLandCameraShake,
			GetActorLocation(),
			0.f,
			100.f,
			1.f
		);
	}
}

void ABaseCharacter::MovementInputManagement()
{
	if (bPrevInputForward != bInputForward)
	{
		if (bInputForward)
		{
			UE_LOG(LogTemp, Warning, TEXT("First Input"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Last Input"));
		}
		bPrevInputForward = bInputForward;
	}

	FVector InputDirection = FVector::ZeroVector;
	if (MovementStatus == EMovementStatus::MS_JumpBeforeApex)
	{

	}
	if (bInputForward)
	{
		InputDirection += GetActorForwardVector();
	}
	if (bInputBackward)
	{
		InputDirection -= GetActorForwardVector();
	}
	if (bInputRight)
	{
		InputDirection += GetActorRightVector();
	}
	if (bInputLeft)
	{
		InputDirection -= GetActorRightVector();
	}
	AddMovementInput(InputDirection, 1.f, true);
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MovementInputManagement();
	GetCharacterMovement()->MaxWalkSpeed = GetCurrentMaxSpeed();
	InterpCapsuleHalfHeight(DeltaTime);
	TiltCamera(DeltaTime);
	ChangeFOV(DeltaTime);
	if (bIsSliding && GetVelocityKPH() <= 10.f)
	{
		StopSlide();
	}

	FHitResult HitResult;
	TArray<AActor*> ar;
	ECollisionChannel CC = ECollisionChannel::ECC_Visibility;
	ETraceTypeQuery TTQ = UEngineTypes::ConvertToTraceType(CC);
	UKismetSystemLibrary::SphereTraceSingle(
		this,
		GetActorLocation(),
		GetActorLocation() + FVector(0.f, 0.f, 10.f),
		50.f,
		TTQ,
		false,
		ar,
		EDrawDebugTrace::ForOneFrame,
		HitResult,
		true
	);
	if (HitResult.bBlockingHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s"), *HitResult.GetActor()->GetName());
	}
}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// PlayerInputComponent->BindAxis(FName("Move Forward / Backward"), this, &ABaseCharacter::MoveForwardAxis);
	// PlayerInputComponent->BindAxis(FName("Move Right / Left"), this, &ABaseCharacter::MoveRightAxis);
	PlayerInputComponent->BindAxis(FName("Look Up / Down Mouse"), this, &ACharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis(FName("Turn Right / Left Mouse"), this, &ACharacter::AddControllerYawInput);
	PlayerInputComponent->BindAction(FName("MoveForward"), EInputEvent::IE_Pressed, this, &ABaseCharacter::MoveForward);
	PlayerInputComponent->BindAction(FName("MoveForward"), EInputEvent::IE_Released, this, &ABaseCharacter::MoveForwardStop);
	PlayerInputComponent->BindAction(FName("MoveBackward"), EInputEvent::IE_Pressed, this, &ABaseCharacter::MoveBackward);
	PlayerInputComponent->BindAction(FName("MoveBackward"), EInputEvent::IE_Released, this, &ABaseCharacter::MoveBackwardStop);
	PlayerInputComponent->BindAction(FName("MoveRight"), EInputEvent::IE_Pressed, this, &ABaseCharacter::MoveRight);
	PlayerInputComponent->BindAction(FName("MoveRight"), EInputEvent::IE_Released, this, &ABaseCharacter::MoveRightStop);
	PlayerInputComponent->BindAction(FName("MoveLeft"), EInputEvent::IE_Pressed, this, &ABaseCharacter::MoveLeft);
	PlayerInputComponent->BindAction(FName("MoveLeft"), EInputEvent::IE_Released, this, &ABaseCharacter::MoveLeftStop);
	PlayerInputComponent->BindAction(FName("PrimaryAction"), EInputEvent::IE_Pressed, this, &ABaseCharacter::FireWeapon);
	PlayerInputComponent->BindAction(FName("Jump"), EInputEvent::IE_Pressed, this, &ABaseCharacter::CustomJump);
	PlayerInputComponent->BindAction(FName("Crouch"), EInputEvent::IE_Pressed, this, &ABaseCharacter::CustomStartCrouch);
	PlayerInputComponent->BindAction(FName("Crouch"), EInputEvent::IE_Released, this, &ABaseCharacter::CustomStopCrouch);
	PlayerInputComponent->BindAction(FName("Sprint / Walk"), EInputEvent::IE_Pressed, this, &ABaseCharacter::SprintOrWalk);
	PlayerInputComponent->BindAction(FName("Sprint / Walk"), EInputEvent::IE_Released, this, &ABaseCharacter::SprintOrWalk);
}

void ABaseCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	bCanDoubleJump = true;
	bIsJumping = false;
	SetMovementStatus(EMovementStatus::MS_Land);
	GetWorldTimerManager().SetTimer(
		MaxJumpTimer,
		this,
		&ABaseCharacter::ActivateMaxJump,
		.12f
	);

	if (CanSlide())
	{
		StartSlide();
	}

	ShakeCamera();
}

void ABaseCharacter::Falling()
{
	Super::Falling();

	if (MovementStatus != EMovementStatus::MS_JumpBeforeApex)
	{
		SetMovementStatus(EMovementStatus::MS_Fall);
	}
}

void ABaseCharacter::GetCameraLookDirection(FVector& OutWorldPosition, FVector& OutWorldDirection)
{
	// Get viewport size
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// Get screen space location of corsshairs
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition, CrosshairWorldDirection;

	// Get world position and direction of crosshairs
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		OutWorldPosition,
		OutWorldDirection
	);
}

float ABaseCharacter::GetVelocityCPS() const
{
	return GetCharacterMovement()->GetLastUpdateVelocity().Length();
}

float ABaseCharacter::GetVelocityKPH() const
{
	return GetVelocityCPS() / 1000.f * 36.f;
}

float ABaseCharacter::GetVelocityXYCPS() const
{
	return FVector(
		GetCharacterMovement()->GetLastUpdateVelocity().X,
		GetCharacterMovement()->GetLastUpdateVelocity().Y,
		0.f
	).Length();
}

float ABaseCharacter::GetVelocityXYKPH() const
{
	return GetVelocityXYCPS() / 1000.f * 36.f;
}