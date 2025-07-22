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
	int32 TotalRoomToFill{0};
	/** 无法放入的物品数量 */
	int32 Remainder{0};
	/** 是否可堆叠 */
	bool bStackable{false};
	/** 具体格子的数据集合 */
	TArray<FInv_SlotAvailability> SlotAvailabilities;
};

/**
 * 拖动开始时的鼠标位置象限，用于拖动道具时处理高亮
 */
UENUM(BlueprintType)
enum class EInv_TileQuadrant : uint8
{
	TopLeft,
	TopRight,
	BottomLeft,
	BottomRight,
	None
};

/**
 * 在 InventoryGrid 中承载拖动信息
 */
USTRUCT(BlueprintType)
struct FInv_TileParameters
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory")
	FIntPoint TileCoordinates{};

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory")
	int32 TileIndex{INDEX_NONE};

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory")
	EInv_TileQuadrant TileQuadrant{EInv_TileQuadrant::None};
};

inline bool operator==(const FInv_TileParameters& A, const FInv_TileParameters& B)
{
	return A.TileCoordinates == B.TileCoordinates && A.TileIndex == B.TileIndex && A.TileQuadrant == B.TileQuadrant;
}

/**
 * 用来描述一个单元格上的空间可用状态
 */
USTRUCT()
struct FInv_SpaceQueryResult
{
	GENERATED_BODY()

	// 这个单元格上有没有空间（有空间就是没道具）
	bool bHasSpace{false};

	// 这个单元格上没有空间的话，它上面的道具是否可以被交换？
	TWeakObjectPtr<UInv_InventoryItem> ValidItem = nullptr;

	// 上述道具可以被交换的话，它的锚点 Index 是什么？
	int32 UpperLeftIndex{INDEX_NONE};
};
