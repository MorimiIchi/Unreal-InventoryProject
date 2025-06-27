#pragma once

#include "Inv_GridTypes.generated.h"

class UInv_InventoryItem;

UENUM(BlueprintType)
enum class EInv_ItemCategory: uint8
{
	Equippable,
	Consumable,
	Craftable,
	None
};

/**
 * 表示单个格子的信息。
 */
USTRUCT()
struct FInv_SlotAvailability
{
	GENERATED_BODY()

	FInv_SlotAvailability()
	{
	}

	FInv_SlotAvailability(int32 ItemIndex, int32 Room, bool bHasItem)
		: Index(ItemIndex)
		  , AmountToFill(Room)
		  , bItemAtIndex(bHasItem)
	{
	}

	/** 格子索引 */
	int32 Index{INDEX_NONE};
	/** 该格子要填充的物品数量 */
	int32 AmountToFill{0};
	/** 格子中是否已有物品 */
	bool bItemAtIndex{false};
};

/**
 * 用于表示添加新物品时各个物品栏格子的可用信息。
 */
USTRUCT(BlueprintType)
struct FInv_SlotAvailabilityResult
{
	GENERATED_BODY()

	FInv_SlotAvailabilityResult()
	{
	}

	/** 指向物品栏中已有的物品 */
	TWeakObjectPtr<UInv_InventoryItem> Item;
	/** 当前可以放入的该物品数量 */
	int32  TotalRoomToFill{0};
	/** 无法放入的物品数量 */
	int32 Remainder{0};
	/** 是否可堆叠 */
	bool bStackable{false};
	/** 具体格子的数据集合 */
	TArray<FInv_SlotAvailability> SlotAvailabilities;
};
