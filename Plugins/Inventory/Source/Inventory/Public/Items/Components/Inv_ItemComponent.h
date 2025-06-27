// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/Manifest/Inv_ItemManifest.h"
#include "Inv_ItemComponent.generated.h"

/**
 * ItemComponent 是用于 Actor 的，并不代表 InventoryItem。
 * 主要的交互对象是 InventoryComponent。
 * 本身也携带一个 Manifest，用于告知系统应该创建什么类型的物品。相当于一个配置的入口。
 */
UCLASS(ClassGroup=(Inventory), meta=(BlueprintSpawnableComponent), Blueprintable)
class INVENTORY_API UInv_ItemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInv_ItemComponent();
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/** InventoryComponent 会把 Manifest 复制下来，创建一个新 inventoryItem，随后通过当前组件销毁 Owner Actor。 */
	FInv_ItemManifest GetItemManifest() const { return ItemManifest; }

	FString GetPickupMessage() const { return PickupMessage; }

private:
	UPROPERTY(Replicated, EditAnywhere, Category="Inventory")
	FInv_ItemManifest ItemManifest;

	UPROPERTY(EditAnywhere, Category="Inventory")
	FString PickupMessage;
};
