// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryManagement/FastArray/Inv_FastArray.h"
#include "Inv_InventoryComponent.generated.h"

class UInv_ItemComponent;
class UInv_InventoryBase;
class UInv_InventoryItem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInventoryItemChange, UInv_InventoryItem*, Item);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNoRoomInInventory);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStackChange, const FInv_SlotAvailabilityResult&, Result);

/**
 * InventoryComponent负责管理物品列表，并通过FastArraySerializer（快速数组序列化器）管理网络复制。
 */
UCLASS(ClassGroup=(Inventory), meta=(BlueprintSpawnableComponent), Blueprintable)
class INVENTORY_API UInv_InventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInv_InventoryComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Inventory")
	void TryAddItem(UInv_ItemComponent* ItemComponent);

	//--------------------------------
	// RPC 属性解释
	// Server：指定该函数在服务器上执行。客户端调用时，RPC 请求会从客户端发送到服务器，并在服务器端运行对应逻辑。
	// Reliable：确保 RPC 调用的可靠性，保证消息不会丢失或乱序，适用于对一致性要求高的关键操作。
	//-------------------------------

	/**
	 * 新增物品到库存（不存在时）
	 * @param ItemComponent 待添加的物品组件
	 * @param StackCount 本次添加的物品数量。
	 */
	UFUNCTION(Server, Reliable)
	void Server_AddNewItem(UInv_ItemComponent* ItemComponent, int32 StackCount);

	/**
	 * 增加已有物品堆叠数量（存在时）
	 * @param ItemComponent 待添加的物品组件
	 * @param StackCount 本次添加的物品数量
	 * @param Remainder （仅限堆叠情况）未能放入库存的剩余物品数量，用于决定是否保留或销毁剩余的拾取物。
	 */
	UFUNCTION(Server, Reliable)
	void Server_AddStacksToItem(UInv_ItemComponent* ItemComponent, int32 StackCount, int32 Remainder);

	void ToggleInventoryMenu();

	/**
	 * 实现 AddRepSubObject 方法来管理复制的子对象（新创建的InventoryItem）。
	 * 我们开启了“显式注册子对象”模式，关闭了默认模式，所以需要提供一个函数供为组件添加子对象的业务调用，显示添加子对象。
	 * 默认模式会对组件里所有 UPROPERTY 指针做一次递归扫描，找出潜在的子对象并复制；
	 * 当子对象数量可能频繁增减（背包格子、装备栏等）时，这种扫描既慢也容易漏掉运行时动态创建的对象。
	 * 显式模式能 按需、精准地复制，同时避免不必要的网络带宽浪费。
	 */
	void AddRepSubObj(UObject* SubObj);

	FInventoryItemChange OnItemAdded;
	FInventoryItemChange OnItemRemoved;
	FNoRoomInInventory NoRoomInInventory;
	FStackChange OnStackChange;

protected:
	virtual void BeginPlay() override;

private:
	void ConstructInventory();

	UPROPERTY(Replicated)
	FInv_InventoryFastArray InventoryList;

	TWeakObjectPtr<APlayerController> OwningController;

	/** Widget */
	UPROPERTY()
	TObjectPtr<UInv_InventoryBase> InventoryMenu;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<UInv_InventoryBase> InventoryMenuClass;

	bool bInventoryMenuOpen;
	void OpenInventoryMenu();
	void CloseInventoryMenu();
};
