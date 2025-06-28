#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Types/Inv_GridTypes.h"
#include "StructUtils/InstancedStruct.h"
#include "Inv_ItemManifest.generated.h"

/**
 * 引入 ItemManifest 结构体用于存储创建新物品时需要的所有信息（如物品类型、分类等）。
 */

USTRUCT(BlueprintType)
struct INVENTORY_API FInv_ItemManifest
{
	GENERATED_BODY()

public:
	/** 提供一个Manifest()方法，用于根据自身数据创建新的InventoryItem实例 */
	UInv_InventoryItem* Manifest(UObject* NewOuter);

	EInv_ItemCategory GetItemCategory() const { return ItemCategory; }

	FGameplayTag GetItemType() const { return ItemType; }

private:
	UPROPERTY(EditAnywhere)
	EInv_ItemCategory ItemCategory{EInv_ItemCategory::None};

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FGameplayTag ItemType;
};
