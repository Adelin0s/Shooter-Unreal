// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

// Forward declares
class UBoxComponent;
class UWidgetComponent;
class USphereComponent;

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	Eir_Damaged UMETA(DisplayName = "Damaged")
	,	Eir_Common UMETA(DisplayName = "Common")
	,	Eir_Uncommon UMETA(DisplayName = "Uncommon")
	,	Eir_Rare UMETA(DisplayName = "Rare")
	,	Eir_Legendary UMETA(DisplayName = "Legendary")
	, Eir_Max UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EItemState : uint8
{
	Eis_Pickup UMETA(DisplayName = "Pickup")
	,	Eis_EquipInterping UMETA(DisplayName = "EquipInterping")
	,	Eis_PickedUp UMETA(DisplayName = "PickedUp")
	,	Eis_Equipped UMETA(DisplayName = "Equipped")
	,	Eis_Falling	UMETA(DisplayName = "Falling")
	, Eir_Max UMETA(Hidden)
};

UCLASS()
class SHOOTER_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Call when overlapping AreaSphere. */
	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Call when end overlapping AreaSphere. */
	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex);

	/** Set the ActiveStars array of bools based on the item rarity. */
	void SetActiveStars();

	/** Sets properties of the Item's component based on State. */
	void SetItemProperties(EItemState State);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	/** Skeleton Mesh for the item */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* ItemMesh;

	/** Line trace collides with box to show HUD widgets */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* CollisionBox;

	/** Popup widget for when the player looks at the item. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* PickupWidget;

	/** Enables item tracing when overlapped. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	USphereComponent* AreaSphere;

	/** The name which appear on the PickupWidget. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	FName ItemName;

	/** Item ammo count. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 ItemAmmoCount;

	/** Item rarity - determines the number of stars in PickupWidget*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	EItemRarity ItemRarity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	TArray< bool > ActiveStars;

	/** State of the item. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	EItemState ItemState;

public:
	FORCEINLINE UWidgetComponent* GetPickupWidget() const noexcept { return PickupWidget; }

	FORCEINLINE USphereComponent* GetAreaSphere() const noexcept { return AreaSphere; }

	FORCEINLINE UBoxComponent* GetCollisionBox() const noexcept { return CollisionBox; }

	void SetItemState(const EItemState State);

	FORCEINLINE EItemState GetItemState() const noexcept { return ItemState; }

	FORCEINLINE USkeletalMeshComponent* GetItemMesh() const noexcept { return ItemMesh; }
};
