// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

AWeapon::AWeapon()
	:	ThrowWeaponTime(1.7f)
	,	bFalling(false)
{

}

void AWeapon::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Keep the Weapon falling
	if(GetItemState() == EItemState::Eis_Falling && bFalling)
	{
		const auto MeshRotation{ GetItemMesh()->GetComponentRotation() };
		GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
	}
}

void AWeapon::ThrowWeapon()
{
	const FRotator MeshRotation{ GetItemMesh()->GetComponentRotation() };
	GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);

	const FVector MeshForward{ GetItemMesh()->GetForwardVector() };
	const FVector MeshRight{ GetItemMesh()->GetRightVector() };

	// Direction in which we throw the Weapon
	FVector ImpulseDirection = MeshRight.RotateAngleAxis(-20.f, MeshForward);
	ImpulseDirection		 = MeshRight.RotateAngleAxis(FMath::FRandRange(10.f, 30.0f), FVector{ 0.f, 0.f, 1.f });
	ImpulseDirection *= 3'000.f;
	GetItemMesh()->AddImpulse(ImpulseDirection);

	bFalling = true;
	GetWorldTimerManager().SetTimer(ThrowWeaponTimer, this, &AWeapon::StopFalling, ThrowWeaponTime);
}

void AWeapon::StopFalling()
{
	bFalling = false;
	SetItemState(EItemState::Eis_Pickup);
}
