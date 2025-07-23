// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/Spatial/Inv_InventoryGrid.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "InventoryManagement/Components/Inv_InventoryComponent.h"
#include "InventoryManagement/Utils/Inv_InventoryStatics.h"
#include "Items/Inv_InventoryItem.h"
#include "Items/Components/Inv_ItemComponent.h"
#include "Items/Fragments/Inv_FragmentTags.h"
#include "Items/Fragments/Inv_ItemFragment.h"
#include "Widgets/Inventory/GridSlots/Inv_GridSlot.h"
#include "Widgets/Utils/Inv_WidgetUtils.h"
#include "Items/Manifest/Inv_ItemManifest.h"
#include "Widgets/Inventory/HoverItem/Inv_HoverItem.h"
#include "Widgets/Inventory/SlottedItems/Inv_SlottedItem.h"

void UInv_InventoryGrid::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ConstructGrid();

	InventoryComponent = UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer());
	InventoryComponent->OnItemAdded.AddDynamic(this, &ThisClass::AddItem);
	InventoryComponent->OnStackChange.AddDynamic(this, &ThisClass::AddStacks);
}

void UInv_InventoryGrid::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	const FVector2D CanvasPosition = UInv_WidgetUtils::GetWidgetPosition(CanvasPanel);
	const FVector2D MousePosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetOwningPlayer());

	UpdateTileParameters(CanvasPosition, MousePosition);
}

void UInv_InventoryGrid::UpdateTileParameters(const FVector2D& CanvasPosition, const FVector2D& MousePosition)
{
	// 如果鼠标不在 Canvas Panel 中就不处理

	// 算出 TileParameters，包括鼠标在哪一格，鼠标在哪个 Index 上，鼠标在格子上的哪一象限
	const FIntPoint HoveredTileCoordinates = CalculateHoveredCoordinates(CanvasPosition, MousePosition);

	LastTileParameters = TileParameters;
	TileParameters.TileCoordinates = HoveredTileCoordinates;
	TileParameters.TileIndex = UInv_WidgetUtils::GetIndexFromPosition(HoveredTileCoordinates, Columns);
	TileParameters.TileQuadrant = CalculateTileQuadrant(CanvasPosition, MousePosition);

	// 处理格子的高亮与否
	OnTileParameterUpdated(TileParameters);
}

void UInv_InventoryGrid::OnTileParameterUpdated(const FInv_TileParameters& Parameters)
{
	if (!IsValid(HoverItem)) return;

	// 获取到 Hover Item 的范围
	const FIntPoint Dimensions = HoverItem->GetGridDimensions();

	// 计算高亮的起始坐标
	const FIntPoint StartingCoordinate = CalculateStartingCoordinate(Parameters.TileCoordinates, Dimensions,
	                                                                 Parameters.TileQuadrant);

	ItemDropIndex = UInv_WidgetUtils::GetIndexFromPosition(StartingCoordinate, Columns);

	// 检查鼠标悬停位置
	CurrentQueryResult = CheckHoverPosition(StartingCoordinate, Dimensions);
}

FInv_SpaceQueryResult UInv_InventoryGrid::CheckHoverPosition(const FIntPoint& Position, const FIntPoint& Dimensions)
{
	FInv_SpaceQueryResult QueryResult;

	// 在 Grid 范围内吗？
	if (!IsInGridBounds(UInv_WidgetUtils::GetIndexFromPosition(Position, Columns), Dimensions)) return QueryResult;

	QueryResult.bHasSpace = true;

	// 这里有道具吗？若有物品，是否仅唯一单元格持有同一物品？
	// - 因为正在拖动的道具可能会覆盖住一片区域，该区域可能有多个道具的锚点在
	TSet<int32> OccupiedUpperLeftIndices;
	UInv_InventoryStatics::ForEach2D(GridSlots, UInv_WidgetUtils::GetIndexFromPosition(Position, Columns), Dimensions,
	                                 Columns, [&](const UInv_GridSlot* GridSlot)
	                                 {
		                                 if (GridSlot->GetInventoryItem().IsValid())
		                                 {
			                                 OccupiedUpperLeftIndices.Add(GridSlot->GetUpperLeftIndex());
			                                 QueryResult.bHasSpace = false;
		                                 }
	                                 });

	// 能和这个道具交换吗？
	// 等于 0：完全空闲，直接返回 hasSpace = true。
	// 等于 1：有且仅有一个物品覆盖区域，允许“交换”或“合并”：
	//  从集合中取出唯一索引，读取该格子的物品指针，赋给 result.validItem。
	//  同时将该索引写入 result.upperLeftIndex。
	// 大于 1：多于一个物品来源，既不可交换也不可放置，保持 hasSpace = false。

	if (OccupiedUpperLeftIndices.Num() == 1) // 只有一个道具在这个位置上 —— 可以交换或者合并
	{
		const int32 Index = *OccupiedUpperLeftIndices.CreateConstIterator();
		QueryResult.ValidItem = GridSlots[Index]->GetInventoryItem();
		QueryResult.UpperLeftIndex = GridSlots[Index]->GetUpperLeftIndex();
	}

	return QueryResult;
}

FIntPoint UInv_InventoryGrid::CalculateStartingCoordinate(const FIntPoint& Coordinate, const FIntPoint& Dimensions,
                                                          const EInv_TileQuadrant Quadrant) const
{
	// 鼠标悬停在格子当中时，起始坐标的计算取决于以下因素：
	// - 鼠标当前所处的网格坐标
	// - 物品本身的尺寸
	// - 鼠标所处的格子内部的象限位置

	// 当道具被拖动时，鼠标本身是位于道具（图标）的中央的
	// 而我们的预期是，如果道具需要占用多格，无论拖到哪里，我们都从锚点位置（左上角第一个格子）开始计算要高亮的格子的尺寸

	// 但这里需要注意的是格子的垂直格数及水平格数是奇数还是偶数。我们的预期是：
	// - 奇数格：鼠标在该方向上移动时，要完全进入另一格时才变更要悬停高亮的格子
	// - 偶数格：鼠标在该方向上移动时，只需要在同一个格子里换到另外半边，就能变更要悬停高亮的格子

	const int32 HasEvenWidth = Dimensions.X % 2 == 0 ? 1 : 0;
	const int32 HasEvenHeight = Dimensions.Y % 2 == 0 ? 1 : 0;

	FIntPoint StartingCoord;
	StartingCoord.X = Coordinate.X - FMath::FloorToInt(0.5f * Dimensions.X) + (
		(Quadrant == EInv_TileQuadrant::TopRight || Quadrant == EInv_TileQuadrant::BottomRight) ? HasEvenWidth : 0);
	StartingCoord.Y = Coordinate.Y - FMath::FloorToInt(0.5f * Dimensions.Y) + (
		(Quadrant == EInv_TileQuadrant::BottomLeft || Quadrant == EInv_TileQuadrant::BottomRight) ? HasEvenHeight : 0);

	return StartingCoord;
}

FIntPoint UInv_InventoryGrid::CalculateHoveredCoordinates(const FVector2D& CanvasPosition,
                                                          const FVector2D& MousePosition) const
{
	return FIntPoint{
		FMath::FloorToInt32((MousePosition.X - CanvasPosition.X) / TileSize),
		FMath::FloorToInt32((MousePosition.Y - CanvasPosition.Y) / TileSize),
	};
}

EInv_TileQuadrant UInv_InventoryGrid::CalculateTileQuadrant(const FVector2D& CanvasPosition,
                                                            const FVector2D& MousePosition) const
{
	// 计算鼠标在格子中的位置
	const float TileLocalX = FMath::Fmod(MousePosition.X - CanvasPosition.X, TileSize);
	const float TileLocalY = FMath::Fmod(MousePosition.Y - CanvasPosition.Y, TileSize);

	// 计算鼠标在格子中的象限
	const bool bIsTop = TileLocalY < TileSize / 2.f;
	const bool bIsLeft = TileLocalX < TileSize / 2.f;

	EInv_TileQuadrant HoveredQuadrant;

	if (bIsTop && bIsLeft) HoveredQuadrant = EInv_TileQuadrant::TopLeft;
	else if (bIsTop && !bIsLeft) HoveredQuadrant = EInv_TileQuadrant::TopRight;
	else if (!bIsTop && bIsLeft) HoveredQuadrant = EInv_TileQuadrant::BottomLeft;
	else HoveredQuadrant = EInv_TileQuadrant::BottomRight;

	return HoveredQuadrant;
}

FInv_SlotAvailabilityResult UInv_InventoryGrid::HasRoomForItem(const UInv_ItemComponent* ItemComponent)
{
	return HasRoomForItem(ItemComponent->GetItemManifest());
}

FInv_SlotAvailabilityResult UInv_InventoryGrid::HasRoomForItem(const UInv_InventoryItem* Item)
{
	return HasRoomForItem(Item->GetItemManifest());
}

FInv_SlotAvailabilityResult UInv_InventoryGrid::HasRoomForItem(const FInv_ItemManifest& Manifest)
{
	FInv_SlotAvailabilityResult Result;

	// 确定物品是否可堆叠
	const FInv_StackableFragment* StackableFragment = Manifest.GetFragmentOfType<FInv_StackableFragment>();
	Result.bStackable = StackableFragment != nullptr;

	// 确定需要添加的堆叠总数
	const int32 MaxStackSize = StackableFragment ? StackableFragment->GetMaxStackSize() : 1;
	int32 AmountToFill = StackableFragment ? StackableFragment->GetStackCount() : 1;

	TSet<int32> CheckedIndices;
	// 遍历物品栏中的每个格子：
	// 这一层只看“锚点”(UpperLeft）是否可用，并不会对道具占据的网格片区做检查，只能算是一次外部的快速检查。
	for (auto& GridSlot : GridSlots)
	{
		// 如果已无剩余堆叠需填充，则提前跳出循环
		if (AmountToFill == 0) break;

		// 判断当前格子是否已被认领
		if (IsIndexClaimed(CheckedIndices, GridSlot->GetTileIndex())) continue;

		// 判断物品锚点是否在边界中
		if (!IsInGridBounds(GridSlot->GetTileIndex(), GetItemDimensions(Manifest))) continue;

		// 判断物品尺寸是否适合当前格子（不会超出网格边界）
		// 这一层里面则是对每个道具占据的格子做判断
		TSet<int32> TentativelyClaimed;
		if (!HasRoomAtIndex(GridSlot, GetItemDimensions(Manifest), CheckedIndices, TentativelyClaimed,
		                    Manifest.GetItemType(), MaxStackSize))
		{
			continue;
		}

		// 确认当前格子可以填多少数量的堆叠
		const int32 AmountToFillInSlot = DetermineFillAmountForSlot(Result.bStackable, MaxStackSize, AmountToFill,
		                                                            GridSlot);
		if (AmountToFill == 0) continue;

		CheckedIndices.Append(TentativelyClaimed);

		// 把当前格子上的信息添加到 Result 中
		Result.TotalRoomToFill += AmountToFillInSlot;
		Result.SlotAvailabilities.Emplace(
			FInv_SlotAvailability{
				HasValidItem(GridSlot) ? GridSlot->GetUpperLeftIndex() : GridSlot->GetTileIndex(),
				Result.bStackable ? AmountToFillInSlot : 0,
				HasValidItem(GridSlot)
			}
		);

		AmountToFill -= AmountToFillInSlot;

		// 还有要添加的道具吗？没有就返回结果，有就继续找下一个格子的信息。
		Result.Remainder = AmountToFill;

		if (AmountToFill == 0) return Result;
	}

	return Result;
}

bool UInv_InventoryGrid::HasRoomAtIndex(const UInv_GridSlot* GridSlot, const FIntPoint& Dimensions,
                                        const TSet<int32>& CheckedIndices, TSet<int32>& OutTentativelyClaimed,
                                        const FGameplayTag& ItemType, const int32 StackMaxSize)
{
	bool bHasRoomAtIndex = true;

	// 对每个合适位置执行2D范围检查（foreach 2D），检查其它的重要条件。
	// SubGrid: 道具占据的每一个格子。这里是因为已经传入的用于获取索引的格子参数被命名为 GridSlot，所以我们把 ForEach2D 遍历的每一个格子叫做 SubGridSlot。
	UInv_InventoryStatics::ForEach2D(
		GridSlots, GridSlot->GetTileIndex(), Dimensions, Columns, [&](const UInv_GridSlot* SubGridSlot)
		{
			if (CheckSlotConstraints(GridSlot, SubGridSlot, CheckedIndices, OutTentativelyClaimed, ItemType,
			                         StackMaxSize))
			{
				// 如果当前格子可以给道具使用，就把它加入到计划要占据的网格片当中
				OutTentativelyClaimed.Add(SubGridSlot->GetTileIndex());
			}
			else
			{
				bHasRoomAtIndex = false;
			}
		});

	return bHasRoomAtIndex;
}

bool UInv_InventoryGrid::CheckSlotConstraints(const UInv_GridSlot* GridSlot, const UInv_GridSlot* SubGridSlot,
                                              const TSet<int32>& CheckedIndices, TSet<int32>& OutTentativelyClaimed,
                                              const FGameplayTag& ItemType, const int32 StackMaxSize) const
{
	//---------------------------------------------------------//
	// 这个函数进行的检查都是针对道具要占用的每一个网格进行的，
	// 外部进行的检查和这里进行的检查具有不同的意义，
	// 
	//---------------------------------------------------------//

	// 格子有被其它道具认领吗？
	if (IsIndexClaimed(CheckedIndices, SubGridSlot->GetTileIndex())) return false;

	// 格子上有其它道具吗？
	if (!HasValidItem(SubGridSlot))
	{
		// 没有的话可以先认领
		OutTentativelyClaimed.Add(SubGridSlot->GetTileIndex());
		return true;
	}

	// 这是一个 Upper Left Slot 吗？
	// 对于可堆叠的多格物品（比如 2×2 大小的药水），堆叠时只允许在该物品占用区域的上左方格（authoritative slot）进行：
	// 如果在其它子格执行堆叠，会导致图标部分重叠，且实际上只是累加计数，不会生成新的 Widget。
	// 因此，只有当子格正好是该已有物品的“上左”起始格时，才允许进行堆叠检查。
	if (!IsUpperLeftSlot(GridSlot, SubGridSlot)) return false;

	// 是可堆叠道具吗？
	// 因为之前确保了当前格子上是有道具的，所以在不可堆叠的情况下要直接返回 false
	const UInv_InventoryItem* SubItem = SubGridSlot->GetInventoryItem().Get();
	if (!SubItem->IsStackable()) return false;

	// 类型与我们要添加的道具的类型相同吗？
	if (!DoesItemTypeMatch(SubItem, ItemType)) return false;

	// 如果可堆叠，是否已经达到了堆叠上限？
	if (GridSlot->GetStackCount() >= StackMaxSize) return false;

	return true;
}

FIntPoint UInv_InventoryGrid::GetItemDimensions(const FInv_ItemManifest& Manifest) const
{
	const FInv_GridFragment* GridFragment = Manifest.GetFragmentOfType<FInv_GridFragment>();
	return GridFragment ? GridFragment->GetGridSize() : FIntPoint(1, 1);
}

bool UInv_InventoryGrid::HasValidItem(const UInv_GridSlot* GridSlot) const
{
	return GridSlot->GetInventoryItem().IsValid();
}

bool UInv_InventoryGrid::IsUpperLeftSlot(const UInv_GridSlot* GridSlot, const UInv_GridSlot* SubGridSlot) const
{
	return SubGridSlot->GetUpperLeftIndex() == GridSlot->GetTileIndex();
}

bool UInv_InventoryGrid::DoesItemTypeMatch(const UInv_InventoryItem* SubItem, const FGameplayTag& ItemType) const
{
	return SubItem->GetItemManifest().GetItemType().MatchesTagExact(ItemType);
}

bool UInv_InventoryGrid::IsInGridBounds(const int32 StartIndex, const FIntPoint& ItemDimensions) const
{
	if (StartIndex < 0 || StartIndex >= GridSlots.Num())
	{
		return false;
	}

	const int32 EndColumn = (StartIndex % Columns) + ItemDimensions.X;
	const int32 EndRow = (StartIndex / Columns) + ItemDimensions.Y;
	return EndColumn <= Columns && EndRow <= Rows;
}

int32 UInv_InventoryGrid::DetermineFillAmountForSlot(const bool bStackable, const int32 MaxStackSize,
                                                     const int32 AmountToFill, const UInv_GridSlot* GridSlot) const
{
	const int32 RoomInSlot = MaxStackSize - GetStackAmount(GridSlot);
	return bStackable ? FMath::Min(AmountToFill, RoomInSlot) : 1;
}

int32 UInv_InventoryGrid::GetStackAmount(const UInv_GridSlot* GridSlot) const
{
	int32 CurrentStackCount = GridSlot->GetStackCount();
	if (const int32 UpperLeftIndex = GridSlot->GetUpperLeftIndex(); UpperLeftIndex != INDEX_NONE)
	{
		UInv_GridSlot* UpperLeftGridSlot = GridSlots[UpperLeftIndex];
		CurrentStackCount = UpperLeftGridSlot->GetStackCount();
	}

	return CurrentStackCount;
}

bool UInv_InventoryGrid::IsRightClick(const FPointerEvent& MouseEvent) const
{
	return MouseEvent.GetEffectingButton() == EKeys::RightMouseButton;
}

bool UInv_InventoryGrid::IsLeftClick(const FPointerEvent& MouseEvent) const
{
	return MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton;
}

void UInv_InventoryGrid::PickUp(UInv_InventoryItem* ClickedInventoryItem, const int32 GridIndex)
{
	// Assign Hover Item
	AssignHoverItem(ClickedInventoryItem, GridIndex, GridIndex);
	// 从 Grid 移除点击的道具
	RemoveItemFromGrid(ClickedInventoryItem, GridIndex);
}

void UInv_InventoryGrid::AssignHoverItem(UInv_InventoryItem* InventoryItem, const int32 GridIndex,
                                         const int32 PreviousGridIndex)
{
	AssignHoverItem(InventoryItem);

	HoverItem->SetPreviousGridIndex(PreviousGridIndex);
	HoverItem->UpdateStackCount(InventoryItem->IsStackable() ? GridSlots[GridIndex]->GetStackCount() : 0);
}

void UInv_InventoryGrid::RemoveItemFromGrid(UInv_InventoryItem* InventoryItem, const int32 GridIndex)
{
	// 拿到 Grid Fragment
	const FInv_GridFragment* GridFragment = GetFragment<FInv_GridFragment>(InventoryItem, FragmentTags::GridFragment);
	if (!GridFragment) return;

	// 对每个格子取消占用
	UInv_InventoryStatics::ForEach2D(GridSlots, GridIndex, GridFragment->GetGridSize(), Columns,
	                                 [&](UInv_GridSlot* GridSlot)
	                                 {
		                                 GridSlot->SetInventoryItem(nullptr);
		                                 GridSlot->SetUpperLeftIndex(INDEX_NONE);
		                                 GridSlot->SetUnoccupiedTexture();
		                                 GridSlot->SetAvailable(false);
		                                 GridSlot->SetStackCount(0);
	                                 });

	// 从 Map 中移除
	if (SlottedItems.Contains(GridIndex))
	{
		TObjectPtr<UInv_SlottedItem> FoundSlottedItem;
		SlottedItems.RemoveAndCopyValue(GridIndex, FoundSlottedItem);
		FoundSlottedItem->RemoveFromParent();
	}
}

void UInv_InventoryGrid::AssignHoverItem(UInv_InventoryItem* InventoryItem)
{
	if (!IsValid(HoverItem))
	{
		HoverItem = CreateWidget<UInv_HoverItem>(GetOwningPlayer(), HoverItemClass);
	}

	const FInv_GridFragment* GridFragment = GetFragment<FInv_GridFragment>(InventoryItem, FragmentTags::GridFragment);
	const FInv_ImageFragment* ImageFragment = GetFragment<
		FInv_ImageFragment>(InventoryItem, FragmentTags::IconFragment);

	if (!GridFragment || !ImageFragment) return;

	const FVector2D DrawSize = GetDrawSize(GridFragment);

	FSlateBrush IconBrush;
	IconBrush.SetResourceObject(ImageFragment->GetIcon());
	IconBrush.DrawAs = ESlateBrushDrawType::Image;
	IconBrush.ImageSize = DrawSize * UWidgetLayoutLibrary::GetViewportScale(this);

	HoverItem->SetImageBrush(IconBrush);
	HoverItem->SetGridDimensions(GridFragment->GetGridSize());
	HoverItem->SetInventoryItem(InventoryItem);
	HoverItem->SetIsStackable(InventoryItem->IsStackable());

	GetOwningPlayer()->SetMouseCursorWidget(EMouseCursor::Default, HoverItem);
}

void UInv_InventoryGrid::AddStacks(const FInv_SlotAvailabilityResult& Result)
{
	if (!MatchesCategory(Result.Item.Get())) return;

	for (const auto& SlotAvailability : Result.SlotAvailabilities)
	{
		if (SlotAvailability.bItemAtIndex)
		{
			const auto& GridSlot = GridSlots[SlotAvailability.Index];
			const auto& SlottedItem = SlottedItems.FindChecked(SlotAvailability.Index);
			SlottedItem->UpdateStackCount(GridSlot->GetStackCount() + SlotAvailability.AmountToFill);
			GridSlot->SetStackCount(GridSlot->GetStackCount() + SlotAvailability.AmountToFill);
		}
		// 这个 else 是针对没有任何道具的 Index 的
		else
		{
			AddItemAtIndex(Result.Item.Get(), SlotAvailability.Index, Result.bStackable, SlotAvailability.AmountToFill);
			UpdateGridSlots(Result.Item.Get(), SlotAvailability.Index, Result.bStackable,
			                SlotAvailability.AmountToFill);
		}
	}
}

void UInv_InventoryGrid::OnSlottedItemClicked(const int32 GridIndex, const FPointerEvent& MouseEvent)
{
	check(GridSlots.IsValidIndex(GridIndex));

	UInv_InventoryItem* ClickedInventoryItem = GridSlots[GridIndex]->GetInventoryItem().Get();

	// 在没有 HoverItem + 左键单击的情况下进入拖动状态
	if (!IsValid(HoverItem) && IsLeftClick(MouseEvent))
	{
		PickUp(ClickedInventoryItem, GridIndex);
	}
}

void UInv_InventoryGrid::AddItem(UInv_InventoryItem* Item)
{
	if (!MatchesCategory(Item)) return;

	FInv_SlotAvailabilityResult Result = HasRoomForItem(Item);

	// 创建一个 Widget 来显示道具 icon，并添加到正确的 grid 中。
	AddItemToIndices(Result, Item);
}

void UInv_InventoryGrid::AddItemToIndices(const FInv_SlotAvailabilityResult& Result, UInv_InventoryItem* NewItem)
{
	for (const auto& Availability : Result.SlotAvailabilities)
	{
		AddItemAtIndex(NewItem, Availability.Index, Result.bStackable, Availability.AmountToFill);
		UpdateGridSlots(NewItem, Availability.Index, Result.bStackable, Availability.AmountToFill);
	}
}

void UInv_InventoryGrid::AddItemAtIndex(UInv_InventoryItem* Item, const int32 Index, const bool bStackable,
                                        const int32 StackAmount)
{
	// 获取 Grid Fragment，以确定该物品占用多少格子
	const FInv_GridFragment* GridFragment = GetFragment<FInv_GridFragment>(Item, FragmentTags::GridFragment);
	// 获取 Image Fragment，以显示物品图标
	const FInv_ImageFragment* ImageFragment = GetFragment<FInv_ImageFragment>(Item, FragmentTags::IconFragment);

	if (!GridFragment || !ImageFragment) return;

	// 创建一个用于添加到网格的 Widget
	UInv_SlottedItem* SlottedItem =
		CreateSlottedItem(Item, bStackable, StackAmount, GridFragment, ImageFragment, Index);

	// 将新创建添加到 Canvas Panel
	AddSlottedItemToCanvas(Index, GridFragment, SlottedItem);

	// 将新创建的 Widget 存储到容器中
	SlottedItems.Add(Index, SlottedItem);
}

UInv_SlottedItem* UInv_InventoryGrid::CreateSlottedItem(UInv_InventoryItem* Item, const bool bStackable,
                                                        const int32 StackAmount, const FInv_GridFragment* GridFragment,
                                                        const FInv_ImageFragment* ImageFragment,
                                                        const int32 Index) const
{
	UInv_SlottedItem* SlottedItem = CreateWidget<UInv_SlottedItem>(GetOwningPlayer(), SlottedItemClass);
	SlottedItem->SetInventoryItem(Item);
	SetSlottedItemImage(SlottedItem, GridFragment, ImageFragment);
	SlottedItem->SetGridIndex(Index);
	SlottedItem->SetIsStackable(bStackable);
	const int32 StackUpdateAmount = bStackable ? StackAmount : 0;
	SlottedItem->UpdateStackCount(StackUpdateAmount);
	SlottedItem->OnSlottedItemClicked.AddDynamic(this, &ThisClass::OnSlottedItemClicked);

	return SlottedItem;
}

void UInv_InventoryGrid::AddSlottedItemToCanvas(const int32 Index, const FInv_GridFragment* GridFragment,
                                                UInv_SlottedItem* SlottedItem) const
{
	CanvasPanel->AddChild(SlottedItem);
	UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(SlottedItem);
	CanvasSlot->SetSize(GetDrawSize(GridFragment));
	const FVector2D DrawPos = UInv_WidgetUtils::GetPositionFromIndex(Index, Columns) * TileSize;
	const FVector2D DrawPosWithPadding = DrawPos + FVector2D(GridFragment->GetGridPadding());
	CanvasSlot->SetPosition(DrawPosWithPadding);
}

void UInv_InventoryGrid::UpdateGridSlots(UInv_InventoryItem* NewItem, const int32 Index, bool bStaclableItem,
                                         const int32 StackAmount)
{
	check(GridSlots.IsValidIndex(Index));

	if (bStaclableItem)
	{
		GridSlots[Index]->SetStackCount(StackAmount);
	}

	const FInv_GridFragment* GridFragment = GetFragment<FInv_GridFragment>(NewItem, FragmentTags::GridFragment);
	if (!GridFragment) return;

	const FIntPoint Dimensions = GridFragment ? GridFragment->GetGridSize() : FIntPoint(1, 1);

	UInv_InventoryStatics::ForEach2D(GridSlots, Index, Dimensions, Columns, [&](UInv_GridSlot* GridSlot)
	{
		GridSlot->SetInventoryItem(NewItem);
		GridSlot->SetUpperLeftIndex(Index);
		GridSlot->SetOccupiedTexture();
		GridSlot->SetAvailable(false);
	});
}

bool UInv_InventoryGrid::IsIndexClaimed(const TSet<int32>& Indices, const int32 Index) const
{
	return Indices.Contains(Index);
}

FVector2D UInv_InventoryGrid::GetDrawSize(const FInv_GridFragment* GridFragment) const
{
	const float IconTileWidth = TileSize - GridFragment->GetGridPadding() * 2;
	return GridFragment->GetGridSize() * IconTileWidth;
}

void UInv_InventoryGrid::SetSlottedItemImage(const UInv_SlottedItem* SlottedItem,
                                             const FInv_GridFragment* GridFragment,
                                             const FInv_ImageFragment* ImageFragment) const
{
	FSlateBrush Brush;
	Brush.SetResourceObject(ImageFragment->GetIcon());
	Brush.DrawAs = ESlateBrushDrawType::Image;
	Brush.ImageSize = GetDrawSize(GridFragment);
	SlottedItem->SetImageBrush(Brush);
}


void UInv_InventoryGrid::ConstructGrid()
{
	GridSlots.Reserve(Rows * Columns);

	for (int32 j = 0; j < Rows; ++j)
	{
		for (int32 i = 0; i < Columns; ++i)
		{
			UInv_GridSlot* GridSlot = CreateWidget<UInv_GridSlot>(this, GridSlotClass);
			CanvasPanel->AddChild(GridSlot);

			// 设置元素的信息
			const FIntPoint TilePosition(i, j);
			GridSlot->SetTileIndex(UInv_WidgetUtils::GetIndexFromPosition(TilePosition, Columns));

			// 设置布局
			UCanvasPanelSlot* GridCPS = UWidgetLayoutLibrary::SlotAsCanvasSlot(GridSlot);
			GridCPS->SetSize(FVector2D(TileSize));
			GridCPS->SetPosition(TilePosition * TileSize);

			GridSlots.Add(GridSlot);
		}
	}
}

bool UInv_InventoryGrid::MatchesCategory(const UInv_InventoryItem* Item) const
{
	return Item->GetItemManifest().GetItemCategory() == ItemCategory;
}
