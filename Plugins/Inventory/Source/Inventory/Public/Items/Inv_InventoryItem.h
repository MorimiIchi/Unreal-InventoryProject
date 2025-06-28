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

	// 告诉引擎，这个对象可以参与网络复制：
	// 当你在 FastArray 里调用 AddReplicatedSubObject( NewItem )，引擎内部会检查这个对象的 IsSupportedForNetworking() 返回值，只有返回 true，它才会把它加入到网络复制队列里。
	// 如果不重载并返回 true，AddReplicatedSubObject 会跳过它，客户端根本拿不到这个新创建的物品对象，PostReplicatedAdd／OnItemAdded 也就不会触发。
	virtual bool IsSupportedForNetworking() const override { return true; }

	void SetItemManifest(const FInv_ItemManifest& Manifest);
	const FInv_ItemManifest& GetManifest() const { return ItemManifest.Get<FInv_ItemManifest>(); }
	FInv_ItemManifest& GetMutableManifest() { return ItemManifest.GetMutable<FInv_ItemManifest>(); }

private:
	/**
	 * 每个InventoryItem实例都拥有自己的Manifest副本，用于保存创建该物品时的相关信息。
	 * 使用FInstancedStruct存储Manifest，使其支持多态、蓝图访问以及更灵活的扩展性。
	 */
	UPROPERTY(VisibleAnywhere, meta=(BaseStruct="/Script/Inventory.Inv_ItemManifest"), Replicated)
	FInstancedStruct ItemManifest;
};
