// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/Inv_InventoryItem.h"
#include "Items/Fragments/Inv_ItemFragment.h"
#include "Items/Manifest/Inv_ItemManifest.h"
#include "Types/Inv_GridTypes.h"
#include "Widgets/Inventory/GridSlots/Inv_GridSlot.h"
#include "Inv_InventoryGrid.generated.h"

class UInv_SlottedItem;
struct FInv_ItemManifest;
class UInv_ItemComponent;
class UInv_InventoryComponent;
class UCanvasPanel;
class UInv_GridSlot;
/**
 * 
 */
UCLASS()
class INVENTORY_API UInv_InventoryGrid : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	EInv_ItemCategory GetItemCategory() const { return ItemCategory; }
	FInv_SlotAvailabilityResult HasRoomForItem(const UInv_ItemComponent* ItemComponent);

	UFUNCTION()
	void AddItem(UInv_InventoryItem* Item);

private:
	void ConstructGrid();
	FInv_SlotAvailabilityResult HasRoomForItem(const UInv_InventoryItem* Item);
	FInv_SlotAvailabilityResult HasRoomForItem(const FInv_ItemManifest& Manifest);
	void AddItemToIndices(const FInv_SlotAvailabilityResult& Result, UInv_InventoryItem* NewItem);
	void SetSlottedItemImage(const UInv_SlottedItem* SlottedItem,
	                         const FInv_GridFragment* GridFragment,
	                         const FInv_ImageFragment* ImageFragment) const;
	FVector2D GetDrawSize(const FInv_GridFragment* GridFragment) const;
	void AddItemAtIndex(UInv_InventoryItem* Item, const int32 Index, const bool bStackable, const int32 StackAmount);
	UInv_SlottedItem* CreateSlottedItem(UInv_InventoryItem* Item, const bool bStackable, const int32 StackAmount,
	                                    const FInv_GridFragment* GridFragment, const FInv_ImageFragment* ImageFragment,
	                                    const int32 Index) const;
	void AddSlottedItemToCanvas(const int32 Index, const FInv_GridFragment* GridFragment,
	                            UInv_SlottedItem* SlottedItem) const;
	void UpdateGridSlots(UInv_InventoryItem* NewItem, const int32 Index, bool bStaclableItem, int32 StackAmount);
	bool IsIndexClaimed(const TSet<int32>& Indices, const int32 Index) const;
	bool HasRoomAtIndex(const UInv_GridSlot* GridSlot, const FIntPoint& Dimensions, const TSet<int32>& CheckedIndices,
	                    TSet<int32>& OutTentativelyClaimed, const FGameplayTag& ItemType, const int32 StackMaxSize);
	bool CheckSlotConstraints(const UInv_GridSlot* GridSlot, const UInv_GridSlot* SubGridSlot,
	                          const TSet<int32>& CheckedIndices, TSet<int32>& OutTentativelyClaimed,
	                          const FGameplayTag& ItemType, const int32 StackMaxSize) const;
	FIntPoint GetItemDimensions(const FInv_ItemManifest& Manifest) const;
	bool HasValidItem(const UInv_GridSlot* GridSlot) const;
	bool IsUpperLeftSlot(const UInv_GridSlot* GridSlot, const UInv_GridSlot* SubGridSlot) const;
	bool DoesItemTypeMatch(const UInv_InventoryItem* SubItem, const FGameplayTag& ItemType) const;
	bool MatchesCategory(const UInv_InventoryItem* Item) const;

	TWeakObjectPtr<UInv_InventoryComponent> InventoryComponent;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCanvasPanel> CanvasPanel;

	UPROPERTY(EditAnywhere, Category="Inventory")
	TSubclassOf<UInv_SlottedItem> SlottedItemClass;

	/** 保持对道具图标的引用，使用 Map 而非 Array 来确保非自动排序 */
	TMap<int32, TObjectPtr<UInv_SlottedItem>> SlottedItems;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"), Category="Inventory")
	EInv_ItemCategory ItemCategory;

	UPROPERTY()
	TArray<TObjectPtr<UInv_GridSlot>> GridSlots;

	UPROPERTY(EditAnywhere, Category="Inventory")
	TSubclassOf<UInv_GridSlot> GridSlotClass;

	UPROPERTY(EditAnywhere, Category="Inventory")
	int32 Rows;

	UPROPERTY(EditAnywhere, Category="Inventory")
	int32 Columns;

	UPROPERTY(EditAnywhere, Category="Inventory")
	float TileSize;
};
