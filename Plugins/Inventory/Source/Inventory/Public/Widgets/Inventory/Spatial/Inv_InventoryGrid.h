﻿// Fill out your copyright notice in the Description page of Project Settings.

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
class UInv_HoverItem;
enum class EInv_GridSlotState : uint8;

/**
 * 道具网格
 */
UCLASS()
class INVENTORY_API UInv_InventoryGrid : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	EInv_ItemCategory GetItemCategory() const { return ItemCategory; }
	FInv_SlotAvailabilityResult HasRoomForItem(const UInv_ItemComponent* ItemComponent);

	void ShowCursor();
	void HideCursor();

	UFUNCTION()
	void AddItem(UInv_InventoryItem* Item);

private:
	void ConstructGrid();
	bool MatchesCategory(const UInv_InventoryItem* Item) const;
	FInv_SlotAvailabilityResult HasRoomForItem(const UInv_InventoryItem* Item);
	/** 在道具栏中查找所有可以给要添加的道具用的格子 */
	FInv_SlotAvailabilityResult HasRoomForItem(const FInv_ItemManifest& Manifest);
	void AddItemToIndices(const FInv_SlotAvailabilityResult& Result, UInv_InventoryItem* NewItem);
	void SetSlottedItemImage(const UInv_SlottedItem* SlottedItem,
	                         const FInv_GridFragment* GridFragment,
	                         const FInv_ImageFragment* ImageFragment) const;
	FVector2D GetDrawSize(const FInv_GridFragment* GridFragment) const;
	void AddItemAtIndex(UInv_InventoryItem* Item, const int32 Index, const bool bStackable, const int32 StackAmount);
	UInv_SlottedItem* CreateSlottedItem(UInv_InventoryItem* Item, const bool bStackable, const int32 StackAmount,
	                                    const FInv_GridFragment* GridFragment, const FInv_ImageFragment* ImageFragment,
	                                    const int32 Index);
	void AddSlottedItemToCanvas(const int32 Index, const FInv_GridFragment* GridFragment,
	                            UInv_SlottedItem* SlottedItem) const;
	void UpdateGridSlots(UInv_InventoryItem* NewItem, const int32 Index, bool bStackableItem, int32 StackAmount);
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
	bool IsInGridBounds(const int32 StartIndex, const FIntPoint& ItemDimensions) const;
	int32 DetermineFillAmountForSlot(const bool bStackable, const int32 MaxStackSize, const int32 AmountToFill,
	                                 const UInv_GridSlot* GridSlot) const;
	int32 GetStackAmount(const UInv_GridSlot* GridSlot) const;
	bool IsRightClick(const FPointerEvent& MouseEvent) const;
	bool IsLeftClick(const FPointerEvent& MouseEvent) const;
	void PickUp(UInv_InventoryItem* ClickedInventoryItem, const int32 GridIndex);

	/** 开始拖动道具时设置 HoverItem */
	void AssignHoverItem(UInv_InventoryItem* InventoryItem, const int32 GridIndex, const int32 PreviousGridIndex);

	/** 开始拖动道具时设置 HoverItem */
	void AssignHoverItem(UInv_InventoryItem* InventoryItem);

	void RemoveItemFromGrid(UInv_InventoryItem* InventoryItem, const int32 GridIndex);

	/** 计算瓦片的坐标系，用于处理网格插槽的高亮与否 */
	void UpdateTileParameters(const FVector2D& CanvasPosition, const FVector2D& MousePosition);

	/** 算出鼠标落在哪一格 */
	FIntPoint CalculateHoveredCoordinates(const FVector2D& CanvasPosition, const FVector2D& MousePosition) const;

	/** 计算鼠标位置在格子中的哪个象限上 */
	EInv_TileQuadrant CalculateTileQuadrant(const FVector2D& CanvasPosition, const FVector2D& MousePosition) const;

	/** TileParameter 更新时 */
	void OnTileParameterUpdated(const FInv_TileParameters& Parameters);

	/** 计算拖动道具时高亮道具的起始格 */
	FIntPoint CalculateStartingCoordinate(const FIntPoint& Coordinate, const FIntPoint& Dimensions,
	                                      const EInv_TileQuadrant Quadrant) const;

	/** 检查拖动道具结束时要放置道具的目标单元格的可用性 */
	FInv_SpaceQueryResult CheckHoverPosition(const FIntPoint& Position, const FIntPoint& Dimensions);

	/** 鼠标退出画布时就不再需要计算了 */
	bool CursorExitedCanvas(const FVector2D& BoundaryPos, const FVector2D& BoundarySize, const FVector2D& Location);

	/** 鼠标悬停在可用或可置换单元格上时高亮格子 */
	void HighLightSlots(const int32 Index, const FIntPoint& Dimensions);
	/** 鼠标离开时取消高亮格子 */
	void UnhighLightSlots(const int32 Index, const FIntPoint& Dimensions);

	/** 改变鼠标悬停时的格子材质 */
	void ChangeHoverType(const int32 Index, const FIntPoint& Dimensions, EInv_GridSlotState GridSlotState);

	/** 将当前拖动的道具放下到指定的索引上 */
	void PutDownOnIndex(const int32 Index);

	/** 放下道具后清理 HoverItem */
	void ClearHoverItem();

	UUserWidget* GetVisibleCursorWidget();
	UUserWidget* GetHiddenCursorWidget();

	/** 拖动道具并点击道具时，判断二者是否为同一类道具 */
	bool IsSameStackable(const UInv_InventoryItem* ClickedInventoryItem) const;

	/** 拖动道具，点击道具，并且二者可以交换时，交换二者 */
	void SwapWithHoverItem(UInv_InventoryItem* ClickedInventoryItem, const int32 GridIndex);

	bool ShouldSwapStackCounts(const int32 RoomInClickedSlot, const int32 HoveredStackCount,
	                           const int32 MaxStackSize) const;

	UPROPERTY(EditAnywhere, Category="Inventory")
	TSubclassOf<UUserWidget> VisibleCursorWidgetClass;

	UPROPERTY(EditAnywhere, Category="Inventory")
	TSubclassOf<UUserWidget> HiddenCursorWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> VisibleCursorWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> HiddenCursorWidget;

	UFUNCTION()
	void AddStacks(const FInv_SlotAvailabilityResult& Result);

	/** 玩家按下道具栏中的道具时处理下拖动事件 */
	UFUNCTION()
	void OnSlottedItemClicked(int32 GridIndex, const FPointerEvent& MouseEvent);


	/** 处理 GridSlot 的 GridSlotClicked 事件 */
	UFUNCTION()
	void OnGridSlotClicked(int32 GridIndex, const FPointerEvent& MouseEvent);

	/** 处理 GridSlot 的 GridSlotHovered 事件 */
	UFUNCTION()
	void OnGridSlotHovered(int32 GridIndex, const FPointerEvent& MouseEvent);

	/** 处理 GridSlot 的 GridSlotUnhovered 事件 */
	UFUNCTION()
	void OnGridSlotUnhovered(int32 GridIndex, const FPointerEvent& MouseEvent);

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

	UPROPERTY(EditAnywhere, Category="Inventory")
	TSubclassOf<UInv_HoverItem> HoverItemClass;

	UPROPERTY()
	TObjectPtr<UInv_HoverItem> HoverItem;

	/** 用来处理鼠标拖动物体时的高亮等信息 */
	FInv_TileParameters TileParameters;
	FInv_TileParameters LastTileParameters;

	/** 当拖动道具并最终点击一个可用的单元格时，将要放下道具的 Index */
	int32 ItemDropIndex{INDEX_NONE};
	FInv_SpaceQueryResult CurrentQueryResult;

	bool bMouseWithinCanvas;
	bool bLastMouseWithinCanvas;

	int32 LastHighlightedIndex;
	FIntPoint LastHighlightedDimensions;
};
