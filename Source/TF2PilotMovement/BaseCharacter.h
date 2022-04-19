// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseCharacter.generated.h"

class UCameraComponent;
class UCameraShake;
class USceneComponent;
class USpringArmComponent;

UENUM(BlueprintType)
enum class EMovementStatus : uint8
{
	MS_Land				UMETA(DisplayName = "Land"),
	MS_Wallrun			UMETA(DisplayName = "Wallrun"),
	MS_Fall				UMETA(DisplayName = "Fall"),
	MS_JumpBeforeApex	UMETA(DisplayName = "JumpBeforeApex"),

	DefaultMax			UMETA(DisplayName = "DefaultMax")
};

UCLASS()
class TF2PILOTMOVEMENT_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseCharacter();

	virtual void Landed(const FHitResult& Hit) override;
	virtual void Falling() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward();
	void MoveForwardStop();
	void MoveBackward();
	void MoveBackwardStop();
	void MoveRight();
	void MoveRightStop();
	void MoveLeft();
	void MoveLeftStop();
	void MoveForwardAxis(float Value);
	void MoveRightAxis(float Value);
	UFUNCTION(BlueprintNativeEvent)
	void FireWeapon();
	void FireWeapon_Implementation();
	void CustomJump();
	void ActivateMaxJump();
	bool CustomCanJump() const;
	void CustomStartCrouch();
	void CustomStopCrouch();
	void SprintOrWalk();

	void SetMovementStatus(EMovementStatus NewStatus);

	float GetCurrentMaxSpeed() const;

	void InterpCapsuleHalfHeight(float DeltaTime);

	UFUNCTION(BlueprintCallable)
	void ReachedJumpApex();

	bool CanSlide() const;
	void StartSlide();
	void StopSlide();
	void ActivateSlideBoost();

	void TiltCamera(float DeltaTime);
	void ChangeFOV(float DeltaTime);
	void ShakeCamera();

	void MovementInputManagement();

private:
	// Components
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess))
	USpringArmComponent* CameraPitchControlBase;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess))
	USceneComponent* CameraTiltControlBase;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess))
	UCameraComponent* CameraComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess))
	USkeletalMeshComponent* Arm;

	// Timers
	FTimerHandle SlideBoostResetTimer;
	FTimerHandle MaxJumpTimer;

	// Setups
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup|Camera", meta = (AllowPrivateAccess = "true"))
	float DefaultFOV;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup|Camera", meta = (AllowPrivateAccess = "true"))
	float SlideFOVOffset;
	float SlideFOV;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup|Camera", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UCameraShakeBase> JumpLandCameraShake;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|MaxSpeed", meta = (AllowPrivateAccess = "true"))
	float SprintSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|MaxSpeed", meta = (AllowPrivateAccess = "true"))
	float WalkSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|MaxSpeed", meta = (AllowPrivateAccess = "true"))
	float CrouchSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|MaxSpeed", meta = (AllowPrivateAccess = "true"))
	float WallrunSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup", meta = (AllowPrivateAccess = "true"))
	bool bAutoSprint;

	float DefaultCapsuleHalfHeight;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Crouch", meta = (AllowPrivateAccess = "true"))
	float CapsuleInterpSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Crouch", meta = (AllowPrivateAccess = "true"))
	float CrouchCapsuleHalfHeight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Slide", meta = (AllowPrivateAccess = "true"))
	float SlideBoostResetTime;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Slide", meta = (AllowPrivateAccess = "true"))
	float SlideBoostForce;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Slide", meta = (AllowPrivateAccess = "true"))
	float SlideCameraTiltAngle;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Slide", meta = (AllowPrivateAccess = "true"))
	float SlideGroundFriction;
	float DefaultGroundFirction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Slide", meta = (AllowPrivateAccess = "true"))
	float SlideBrakingDeceleration;
	float DefaultBrakingDeceleration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Jump", meta = (AllowPrivateAccess = "true"))
	float JumpZForce;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Jump", meta = (AllowPrivateAccess = "true"))
	float InstantJumpMultiplier;

	// Status
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Input", meta = (AllowPrivateAccess = "true"))
	bool bInputForward;
	bool bPrevInputForward;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Input", meta = (AllowPrivateAccess = "true"))
	bool bInputBackward;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Input", meta = (AllowPrivateAccess = "true"))
	bool bInputRight;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Input", meta = (AllowPrivateAccess = "true"))
	bool bInputLeft;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Status", meta = (AllowPrivateAccess = "true"))
	bool bIsAccelForward;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Status", meta = (AllowPrivateAccess = "true"))
	bool bIsSprinting;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Status", meta = (AllowPrivateAccess = "true"))
	bool bWalkSprintInput;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Status", meta = (AllowPrivateAccess = "true"))
	bool bIsWallrunning;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Status", meta = (AllowPrivateAccess = "true"))
	bool bIsCrouching;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Status", meta = (AllowPrivateAccess = "true"))
	bool bIsSliding;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Status", meta = (AllowPrivateAccess = "true"))
	bool bCanSlideBoost;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Status", meta = (AllowPrivateAccess = "true"))
	bool bIsJumping;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Status", meta = (AllowPrivateAccess = "true"))
	bool bCanMaxJump;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Status", meta = (AllowPrivateAccess = "true"))
	bool bCanDoubleJump;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	EMovementStatus MovementStatus;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Slide", meta = (AllowPrivateAccess = "true"))
	FVector SlideDirection;

	float DefaultMaxAcceleration;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable)
	void GetCameraLookDirection(FVector& OutWorldPosition, FVector& OutWorldDirection);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetVelocityCPS() const;
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetVelocityKPH() const;
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetVelocityXYCPS() const;
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetVelocityXYKPH() const;
};
