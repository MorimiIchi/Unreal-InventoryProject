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

	int32 Index{INDEX_NONE};
	int32 AmountToFill{0};
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

	TWeakObjectPtr<UInv_InventoryItem> Item;
	int32 TotalRoomToFill{0};
	int32 Remainder{0};
	bool bStackable{false};
	TArray<FInv_SlotAvailability> SlotAvailabilities;
};
