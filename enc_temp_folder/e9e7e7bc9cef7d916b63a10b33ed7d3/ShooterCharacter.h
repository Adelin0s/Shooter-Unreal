// =====================================================================
//   @ Author: Cucorianu Eusebiu Adelin                                                                                      
//   @ Create Time: 22-11-2022 9:52 PM                                                                                                                                                
//   @ Contact: cucorianu.adelin@protonmail.com                                                                                                                          
//   @ Modified time: 28-11-2022 2:23 PM                                                                                                                                    
//   @ Description:                                                                                                                                                                                
// =====================================================================

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterCharacter.generated.h"


// Forward declares
class USpringArmComponent;
class UCameraComponent;
class USoundCue;
class UParticleSystem;
class UAnimMontage;
class AItem;
class AWeapon;

UCLASS()
class SHOOTER_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooterCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Returns CameraBoom sub-object */
	FORCEINLINE USpringArmComponent *GetCameraBoom() const
	{
		return CameraBoom;
	}

	/** Returns FollowCamera sub-object */
	FORCEINLINE UCameraComponent *GetFollowCamera() const
	{
		return FollowCamera;
	}

	FORCEINLINE bool GetAiming() const noexcept
	{
		return bAiming;
	}

	UFUNCTION(BlueprintCallable)
	float GetCrosshairSpreadMultiplier() const noexcept
	{
		return CrosshairSpreadMultiplier;
	}

	FORCEINLINE uint8 GetOverlappedItemsCount() const noexcept
	{
		return OverlappedItemsCount;
	}

	/** Add or subtracts to/from OverlappedItemCount. */
	void IncrementOverlappedItemCount(int8 Amount);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Called for forwards/backwards input */
	void MoveForward(float Value);

	/** Called for side by side input */
	void MoveRight(float Value);

	/**
	 * Called via input to turn at a give rate.
	 * @param Rate This is a normalized rate, i.e 1.0 means 100% of desired rate.
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to look up/down at a given rate.
	 * @param Rate This is a normalized, i.e 1.0 means 100% of desired rate.
	 */
	void LookUpAtRate(float Rate);

	/**
	 * @brief Rotate controller based on mouse X movement.
	 * @param Rate  The input value from mouse movement
	 */
	void Turn(float Rate);

	/**
	 * @brief Rotate controller based on mouse Y movement.
	 * @param Rate  The input value from mouse movement
	 */
	void LookUp(float Rate);

	/** Called when the Fire Button is pressed. */
	void FireWeapon();

	bool GetBeamEndLocation(const FVector &MuzzleSocketLocation, FVector &OutBeamEndLocation) const;

	/** Set bAiming to true or false with button press. */
	void AimingButtonPressed();

	void AimingButtonReleased();

	void CameraInterpZoom(float DeltaTime);

	/** Set BaseTurnRate and BaseLookUpRate based on aiming. */
	void SetLookRates();

	void CalculateCrosshairSpread(float DeltaTime);

	void StartCrosshairBulletFire();

	UFUNCTION()
	void FinishCrosshairBulletFire();

	void FireButtonPressed();

	void FireButtonReleased();

	void StartFireTimer();

	UFUNCTION()
	void AutoFireReset();

	/** Line trace for items under the crosshairs. */
	void TraceUnderCrosshairs(FHitResult &OutHitResult, FVector *const OutHitLocation = nullptr) const;

	/** Trace for items if OverlappedItemsCount <= 0. */
	void TraceForItems();

	/** Spawns a default weapon and equips it. */ 
 	AWeapon* SpawnDefaultWeapon() const;

	/** Takes a weapon and attaches it to the mesh. */
	void EquipWeapon(AWeapon* WeaponToEquip);

	/** Detach weapon and let it to fall to the ground. */
	void DropWeapon();

	void SelectButtonPressed();
	void SelectButtonReleased();

private:
	/** Camera boom positioning behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent *CameraBoom;

	/** Camera that follows the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent *FollowCamera;

	/** Base turn rate, in deg/sec. Other scale rate may affect final turn rate */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scale rate may affect final turn rate */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseLookUpRate;

	/** True when aiming */
	bool bAiming;

	/** Turn rate while not aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float HipTurnRate;

	/** Look up rate while not aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float HipLookUpRate;

	/** Turn rate while aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float AimingTurnRate;

	/** Look up rate while aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float AimingLookUpRate;

	/** Scale factor for mouse look sensitivity. Turn rate when not aiming. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseHipTurnRate;

	/** Scale factor for mouse look sensitivity. Look up rate when not aiming. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseHipLookUpRate;

	/** Scale factor for mouse look sensitivity. Turn rate when aiming. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseAimingTurnRate;

	/** Scale factor for mouse look sensitivity. Look up rate when aiming. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseAimingLookUpRate;

	/** Randomize gunshot sound cue. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	USoundCue *FireSound;

	/** Flash spawned at BarrelSocket. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem *MuzzleFlash;

	/** Montage for firing the weapon. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage *HipFireMontage;

	/** Particles spawn upon bullet impact. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem *ImpactParticles;

	/** Smoke trail for bullets. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem *BeamParticles;

	/** Interp speed for zooming when aiming. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float ZoomInterpSpeed;

	/** Default camera field of view value. */
	float CameraDefaultFOV;

	/** Field of view value for when zoom in. */
	float CameraZoomedFOV;

	/** Current field of view this frame. */
	float CameraCurrentFOV;

	/** Determines the spread of the crosshairs. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshair, meta = (AllowPrivateAccess = "true"))
	float CrosshairSpreadMultiplier;

	/** Velocity component for crosshairs spread. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshair, meta = (AllowPrivateAccess = "true"))
	float CrosshairVelocityFactor;

	/** In air component for crosshairs spread. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshair, meta = (AllowPrivateAccess = "true"))
	float CrosshairInAirFactor;

	/** Aim component for crosshairs spread. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshair, meta = (AllowPrivateAccess = "true"))
	float CrosshairAimFactor;

	/** Shooting component for crosshairs spread. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshair, meta = (AllowPrivateAccess = "true"))
	float CrosshairShootingFactor;

	/** The time in which a shoot ends. */
	float ShootTimeDuration;

	/** A bullet  */
	bool bFiringBullet;

	FTimerHandle CrosshairShootTimer;

	/** Left mouse button pressed */
	bool bFireButtonPressed;

	/** Know when we can fire */
	bool bShouldFire;

	/** Rate of automatic gun fire */
	float AutoFireRate;

	/** Set a timer between gunshots */
	FTimerHandle AutoFireTimer;

	/** True if we should trace every frame for items. */
	bool bShouldTraceForItems;

	/** Number of overlapped AItems. */
	int8 OverlappedItemsCount;

	/** The AItem we hit last frame. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	AItem *TraceHitItemLastFrame;

	/** Currently equipped Weapon. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	AWeapon *EquippedWeapon;

	/** Set this in Blueprints for the default Weapon class. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TSubclassOf< AWeapon > DefaultWeaponClass;
};
