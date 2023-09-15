// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterCharacter.h"

#include "Item.h"
#include "Weapon.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

// Sets default values
AShooterCharacter::AShooterCharacter()
	// Base rates for turning or looking up
	:	BaseTurnRate(45.0f)
	,	BaseLookUpRate(45.0f)

	// Turn rates for aiming or not aiming
	,	bAiming(false)
	,	HipTurnRate(90.0f)
	,	HipLookUpRate(90.0f)
	,	AimingTurnRate(20.0f)
	,	AimingLookUpRate(20.0f)

	// Mouse look sensitivity scale factors
	,	MouseHipTurnRate(1.0f)
	,	MouseHipLookUpRate(1.0f)
	,	MouseAimingTurnRate(0.2f)
	,	MouseAimingLookUpRate(0.2f)

	// Camera field of view values
	,	ZoomInterpSpeed(15.0f)
	,	CameraDefaultFOV(0.0f)
	,	CameraZoomedFOV(35.0f)
	,	CameraCurrentFOV(0.0f)

	// Crosshair spread factors
	,	CrosshairSpreadMultiplier(0.0f)
	,	CrosshairVelocityFactor(0.0f)
	,	CrosshairInAirFactor(0.0f)
	,	CrosshairAimFactor(0.0f)
	,	CrosshairShootingFactor(0.0f)

	// Bullet fire timer variables
	,	ShootTimeDuration(0.05f)
	,	bFiringBullet(false)

	// Automatic gun fire rate
	,	bFireButtonPressed(false)
	,	bShouldFire(true)
	,	AutoFireRate(0.1f)
	// Item trace variables
	,	bShouldTraceForItems(false)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create a camera boom (pulls in towards the character if there is a collision).
	CameraBoom = CreateDefaultSubobject< USpringArmComponent >(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 180.f; // The camera follows at this distance behind the character
	CameraBoom->bUsePawnControlRotation = true;	 // Rotate the arm based on the controller
	CameraBoom->SocketOffset = FVector(0.0f, 50.0f, 70.0f);

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject< UCameraComponent >(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach camera to end of boom
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Don't rotate when the controller rotates. Let the controller only affect the camera
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Configure character movement.
	GetCharacterMovement()->bOrientRotationToMovement = false; // Character moves in the direction of the input ->
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 500.0f;
	GetCharacterMovement()->AirControl = 0.2f;
}

void AShooterCharacter::IncrementOverlappedItemCount(const int8 Amount)
{
	if( OverlappedItemsCount + Amount <= 0 )
	{
		OverlappedItemsCount = 0;
		bShouldTraceForItems = false;
	}
	else
	{
		OverlappedItemsCount += Amount;
		bShouldTraceForItems = true;
	}
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if( FollowCamera )
	{
		CameraDefaultFOV = GetFollowCamera()->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}

	// Spawn the default weapon and equip it
	EquipWeapon(SpawnDefaultWeapon()); 
}

void AShooterCharacter::MoveForward(const float Value)
{
	if (Controller != nullptr && Value != 0.0f)
	{
		// Find out which way is forward
		const auto	   Rotation = Controller->GetControlRotation();
		const FRotator YawRotation{ 0.0f, Rotation.Yaw, 0.0f };

		const FVector Direction{ FRotationMatrix{ YawRotation }.GetUnitAxis(EAxis::X) };
		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::MoveRight(const float Value)
{
	if (Controller != nullptr && Value != 0.0f)
	{
		// Find out which way is right
		const auto	   Rotation = Controller->GetControlRotation();
		const FRotator YawRotation{ 0.0f, Rotation.Yaw, 0.0f };

		const FVector Direction{ FRotationMatrix{ YawRotation }.GetUnitAxis(EAxis::Y) };
		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::TurnAtRate(const float Rate)
{
	// Calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds()); // deg/sec * sec/frame
}

void AShooterCharacter::LookUpAtRate(const float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds()); // deg/sec * sec/frame
}

void AShooterCharacter::Turn(const float Rate)
{
	float TurnScaleFactor{ };
	if (bAiming)
	{
		TurnScaleFactor = MouseAimingTurnRate;
	}
	else
	{
		TurnScaleFactor = MouseHipTurnRate;
	}
	AddControllerYawInput(Rate * TurnScaleFactor);
}

void AShooterCharacter::LookUp(const float Rate)
{
	float LookUpScaleFactor{ };
	if (bAiming)
	{
		LookUpScaleFactor = MouseAimingLookUpRate;
	}
	else
	{
		LookUpScaleFactor = MouseHipLookUpRate;
	}
	AddControllerPitchInput(Rate * LookUpScaleFactor);
}

void AShooterCharacter::FireWeapon()
{
	if (nullptr != FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}

	const USkeletalMeshSocket* BarrelSocket{ GetMesh()->GetSocketByName("BarrelSocket") };
	if (nullptr != BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh());

		if (nullptr != MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}

		FVector	   BeamEndLocation;
		const auto bBeamEndLocation = GetBeamEndLocation(SocketTransform.GetLocation(), BeamEndLocation);
		if (bBeamEndLocation)
		{
			// Spawn impact particles after updating BeamEndPoint
			if (nullptr != ImpactParticles)
			{

				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamEndLocation);
			}

			if (nullptr != BeamParticles)
			{
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
				if (nullptr != Beam)
				{
					Beam->SetVectorParameter(FName("Target"), BeamEndLocation);
				}
			}
		}
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}

	// Start bullet fire timer for crosshair
	StartCrosshairBulletFire();
}

bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamEndLocation) const
{
	// Check for crosshair trace hit
	FHitResult CrosshairHitResult;
	TraceUnderCrosshairs(CrosshairHitResult, &OutBeamEndLocation);

	// Perform a second trace, this time from the gun barrel
	FHitResult	  WeaponTraceHit;
	const FVector WeaponTraceStart{ MuzzleSocketLocation };
	const FVector WeaponTraceEnd{ OutBeamEndLocation };
	GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECC_Visibility);

	// Update BeamEndPoint in case of collision
	if (WeaponTraceHit.bBlockingHit)
	{
		OutBeamEndLocation = WeaponTraceHit.Location;

		return true;
	}

	return false;
}

void AShooterCharacter::AimingButtonPressed()
{
	bAiming = true;
}

void AShooterCharacter::AimingButtonReleased()
{
	bAiming = false;
}

void AShooterCharacter::CameraInterpZoom(const float DeltaTime)
{
	// Set current camera field of view
	if (bAiming)
	{
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraZoomedFOV, DeltaTime, ZoomInterpSpeed);
	}
	else
	{
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraDefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	GetFollowCamera()->FieldOfView = CameraCurrentFOV;
}

void AShooterCharacter::SetLookRates()
{
	if (bAiming)
	{
		BaseTurnRate = AimingTurnRate;
		BaseLookUpRate = AimingLookUpRate;
	}
	else
	{
		BaseTurnRate = HipTurnRate;
		BaseLookUpRate = HipLookUpRate;
	}
}

void AShooterCharacter::CalculateCrosshairSpread(const float DeltaTime)
{
	const FVector2D WalkSpeedRange{ 0.0f, 600.0f };
	const FVector2D VelocityMultiplierRange{ 0.0f, 1.0f };
	FVector			Velocity = GetVelocity();
	Velocity.Z = 0;

	// Calculate crosshair velocity factor
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

	// Calculate crosshair in air factor
	if (GetCharacterMovement()->IsFalling())
	{
		// Spread the crosshairs slowly while in air
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
	}
	else
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.0f, DeltaTime, 20.0f);
	}

	// Calculate crosshair aim factor
	if (bAiming)
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.6f, DeltaTime, 20.0f);
	}
	else
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.0f, DeltaTime, 20.0f);
	}

	if (bFiringBullet)
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.3f, DeltaTime, 60.0f);
	}
	else
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.0f, DeltaTime, 60.0f);
	}

	CrosshairSpreadMultiplier = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairShootingFactor;
}

void AShooterCharacter::StartCrosshairBulletFire()
{
	bFiringBullet = true;

	GetWorldTimerManager().SetTimer(CrosshairShootTimer, this, &AShooterCharacter::FinishCrosshairBulletFire, ShootTimeDuration);
}

void AShooterCharacter::FinishCrosshairBulletFire()
{
	bFiringBullet = false;
}

void AShooterCharacter::FireButtonPressed()
{
	bFireButtonPressed = true;
	StartFireTimer();
}

void AShooterCharacter::FireButtonReleased()
{
	bFireButtonPressed = false;
}

void AShooterCharacter::StartFireTimer()
{
	if (bShouldFire)
	{
		FireWeapon();
		bShouldFire = false;
		GetWorldTimerManager().SetTimer(AutoFireTimer, this, &AShooterCharacter::AutoFireReset, AutoFireRate);
	}
}

void AShooterCharacter::AutoFireReset() 
{
	bShouldFire = true;

	if (bFireButtonPressed)
	{
		StartFireTimer();
	}
}

void AShooterCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult, FVector* const OutHitLocation) const
{
	// Get viewport size
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// Get screen space location of crosshairs
	const FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	// Get world position and direction of crosshairs
	const bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection);
	if (bScreenToWorld)
	{
		const FVector Start{ CrosshairWorldPosition };
		const FVector End{ Start + CrosshairWorldDirection * 50'000.f };
		GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECC_Visibility);

		if (nullptr != OutHitLocation)
		{
			if (OutHitResult.bBlockingHit)
			{
				*OutHitLocation = OutHitResult.Location;
			}
			else
			{
				*OutHitLocation = End;
			}
		}
	}
}

void AShooterCharacter::TraceForItems()
{
	if( bShouldTraceForItems )
	{
		FHitResult ItemTraceResult;
		TraceUnderCrosshairs(ItemTraceResult);
		if( ItemTraceResult.bBlockingHit )
		{
			TraceHitItem = Cast< AItem >(ItemTraceResult.GetActor());
			if( nullptr != TraceHitItem && nullptr != TraceHitItem->GetPickupWidget() )
			{
				// Show Item's Pickup Widget
				TraceHitItem->GetPickupWidget()->SetVisibility(true);
			}

			if( nullptr != TraceHitItemLastFrame )
			{
				if( TraceHitItem != TraceHitItemLastFrame )
				{
					TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
				}
			}

			// Store a reference to HitItem for next frame
			TraceHitItemLastFrame = TraceHitItem;
		}
	}
	else if( nullptr != TraceHitItemLastFrame )
	{
		// No longer overlapping any items, item last frame should not show widget
		TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
	}
}

AWeapon* AShooterCharacter::SpawnDefaultWeapon() const
{
	// Check the TSubclassOf variable
	if( nullptr != DefaultWeaponClass )
	{
		// Spawn the Weapon
		return GetWorld()->SpawnActor< AWeapon >(DefaultWeaponClass);
	}

	return nullptr;
}

void AShooterCharacter::EquipWeapon(AWeapon* WeaponToEquip)
{
	if( nullptr != WeaponToEquip )
	{
		// Get the Hand Socket
		const auto HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if( nullptr != HandSocket )
		{
			// Attach the Weapon to the hand socket
			HandSocket->AttachActor(WeaponToEquip, GetMesh());
		}
		// Set EquippedWeapon to the newly spawn Weapon
		EquippedWeapon = WeaponToEquip;
		EquippedWeapon->SetItemState(EItemState::Eis_Equipped);

	}
}

void AShooterCharacter::DropWeapon()
{
	if( EquippedWeapon )
	{
		const FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
		EquippedWeapon->GetItemMesh()->DetachFromComponent(DetachmentTransformRules);

		EquippedWeapon->SetItemState(EItemState::Eis_Falling);
		EquippedWeapon->ThrowWeapon();
	}
}

void AShooterCharacter::SelectButtonPressed()
{
	if( TraceHitItem )
	{
		const auto TraceHitWeapon = Cast< AWeapon >(TraceHitItem);
		SwapWeapon(TraceHitWeapon);
	}
}

void AShooterCharacter::SelectButtonReleased() 
{
	
}

void AShooterCharacter::SwapWeapon(AWeapon* Weapon)
{
	DropWeapon();
	EquipWeapon(Weapon);
	TraceHitItem = nullptr;
	TraceHitItemLastFrame = nullptr;
}

// Called every frame
void AShooterCharacter::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Handle interpolation for zoom when aiming
	CameraInterpZoom(DeltaTime);
	// Adapting look sensitivity based on aiming
	SetLookRates();
	// Calculate crosshair spread multiplier
	CalculateCrosshairSpread(DeltaTime);
	// Check OverlappedItemsCount, then trace for items
	TraceForItems();
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &AShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterCharacter::LookUpAtRate);
	PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::LookUp);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", IE_Released, this, &AShooterCharacter::FireButtonReleased);

	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AShooterCharacter::AimingButtonReleased);

	PlayerInputComponent->BindAction("Select", IE_Pressed, this, &AShooterCharacter::SelectButtonPressed);
	PlayerInputComponent->BindAction("Select", IE_Released, this, &AShooterCharacter::SelectButtonReleased);
}
