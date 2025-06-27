// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Items/Manifest/Inv_ItemManifest.h"
#include "Inv_InventoryItem.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORY_API UInv_InventoryItem : public UObject
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	void SetItemManifest(const FInv_ItemManifest& Manifest);

private:
	/**
	 * 每个InventoryItem实例都拥有自己的Manifest副本，用于保存创建该物品时的相关信息。
	 * 使用FInstancedStruct存储Manifest，使其支持多态、蓝图访问以及更灵活的扩展性。
	 */
	UPROPERTY(VisibleAnywhere, meta=(BaseStruct="/Script/Inventory.Inv_ItemManifest"), Replicated)
	FInstancedStruct ItemManifest;
};
