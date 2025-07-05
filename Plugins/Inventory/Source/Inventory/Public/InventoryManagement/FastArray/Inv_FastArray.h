#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "Inv_FastArray.generated.h"

struct FGameplayTag;
class UInv_ItemComponent;
class UInv_InventoryComponent;
class UInv_InventoryItem;

/** A single entry in an inventory */
USTRUCT(BlueprintType)
struct FInv_InventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

public:
	FInv_InventoryEntry()
	{
	}

private:
	friend struct FInv_InventoryFastArray;
	friend UInv_InventoryComponent;

	// 要存储的道具的指针
	UPROPERTY()
	TObjectPtr<UInv_InventoryItem> Item = nullptr;
};

/** List of inventory items */
USTRUCT(BlueprintType)
struct FInv_InventoryFastArray : public FFastArraySerializer
{
	GENERATED_BODY()

public:
	FInv_InventoryFastArray() : OwnerComponent(nullptr)
	{
	}

	FInv_InventoryFastArray(UActorComponent* InOwnerComponent) : OwnerComponent(InOwnerComponent)
	{
	}

	TArray<UInv_InventoryItem*> GetAllItems() const;

	// 添加或移除条目时的事件回调，可以在这里通知道具系统发生了数据变化，尽量确保这里只实现客户端侧的逻辑
	//~ FFastArraySerializer contract ~//
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	//~ End of FFastArraySerializer contract ~//

	// Delta 序列化函数，必须实现。
	// 引擎会默认调用标记了启用 NetDeltaSerializer 的属性的 NetDeltaSerialize。
	// 该函数负责告诉引擎 “把我的当前状态与基态对比，并把结果写到（或从）网络字节流里”。
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		// FInventoryItemEntry: 数组元素类型
		// FInventoryFastArray: 本结构体类型
		// Entries: 真正保存元素的 TArray 成员。本案例中 FastArray 内部持有的 `TArray<FInventoryEntry>`。模板函数会遍历它、计算增删改。
		// DeltaParams: 由引擎注入的运行时上下文：
		//	- bIsSaving（正在写服务器→客户端）  
		//	- Reader / Writer 指针（底层字节流）  
		//	- OldState（上一次基态）等。
		// *this: 当前 FastArray 实例本身。模板函数需要它来调用你的 `MarkItemDirty` 等工具函数。
		return FastArrayDeltaSerialize<FInv_InventoryEntry, FInv_InventoryFastArray>(Entries, DeltaParams, *this);
	}

	// 实际添加/移除条目函数的工具函数
	UInv_InventoryItem* AddEntry(UInv_ItemComponent* ItemComponent);
	UInv_InventoryItem* AddEntry(UInv_InventoryItem* Item);
	void RemoveEntry(UInv_InventoryItem* Item);
	UInv_InventoryItem* FindFirstItemByType(const FGameplayTag& ItemType);

private:
	friend UInv_InventoryComponent;

	// 被包装的 TArray
	// Replicated list of items
	UPROPERTY()
	TArray<FInv_InventoryEntry> Entries;

	UPROPERTY(NotReplicated)
	TObjectPtr<UActorComponent> OwnerComponent;
};

template <>
struct TStructOpsTypeTraits<FInv_InventoryFastArray> : public TStructOpsTypeTraitsBase2<FInv_InventoryFastArray>
{
	enum
	{
		// 当 Unreal 发现某个属性标记了 `WithNetDeltaSerializer = true` 时，
		// 就会调用该属性所在结构体的 `NetDeltaSerialize` 来完成“增删改”差量复制。
		WithNetDeltaSerializer = true
	};
};
