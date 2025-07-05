// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryManagement/Components/Inv_InventoryComponent.h"

#include "Net/UnrealNetwork.h"
#include "Items/Components/Inv_ItemComponent.h"
#include "Items/Inv_InventoryItem.h"
#include "Items/Fragments/Inv_ItemFragment.h"
#include "Widgets/Inventory/InventoryBase/Inv_InventoryBase.h"


UInv_InventoryComponent::UInv_InventoryComponent() : InventoryList(this)
{
	PrimaryComponentTick.bCanEverTick = false;

	// 为了网络复制：组件需启用SetIsReplicatedByDefault和bReplicateUsingRegisteredSubobjectList。

	// SetIsReplicatedByDefault(true) 告诉引擎：这个组件本身一旦附着到一个已在网络上存在的 Actor，就应该默认参与复制。
	SetIsReplicatedByDefault(true);

	// bReplicateUsingRegisteredSubObjectList 切换到 “显式注册子对象” 模式：只有通过 AddReplicatedSubObject() 注册过的 UObjects 才会被考虑发送到客户端。
	// 默认模式会对组件里所有 UPROPERTY 指针做一次递归扫描，找出潜在的子对象并复制；
	// 当子对象数量可能频繁增减（背包格子、装备栏等）时，这种扫描既慢也容易漏掉运行时动态创建的对象。
	// 显式模式能 按需、精准地复制，同时避免不必要的网络带宽浪费。
	bReplicateUsingRegisteredSubObjectList = true;

	bInventoryMenuOpen = false;
}

void UInv_InventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, InventoryList);
}

void UInv_InventoryComponent::TryAddItem(UInv_ItemComponent* ItemComponent)
{
	FInv_SlotAvailabilityResult Result = InventoryMenu->HasRoomForItem(ItemComponent);

	// 寻找背包里有没有相同类型的道具，将决定是堆叠还是添加新道具
	UInv_InventoryItem* FoundItem = InventoryList.FindFirstItemByType(ItemComponent->GetItemManifest().GetItemType());
	Result.Item = FoundItem;

	if (Result.TotalRoomToFill == 0)
	{
		NoRoomInInventory.Broadcast();
		return;
	}

	// 添加道具到道具栏中
	if (Result.Item.IsValid() && Result.bStackable)
	{
		// 为已经存在于道具栏中的单格道具添加叠加数量
		OnStackChange.Broadcast(Result);
		// 请求服务器添加堆叠
		Server_AddStacksToItem(ItemComponent, Result.TotalRoomToFill, Result.Remainder);
	}
	else if (Result.TotalRoomToFill > 0)
	{
		// 道具栏里并不存在该类道具，找一个空格子创建一个新的道具
		// 请求服务器添加道具
		Server_AddNewItem(ItemComponent, Result.bStackable ? Result.TotalRoomToFill : 0);
	}
}

void UInv_InventoryComponent::Server_AddNewItem_Implementation(UInv_ItemComponent* ItemComponent, int32 StackCount)
{
	UInv_InventoryItem* NewItem = InventoryList.AddEntry(ItemComponent);

	// 对于独立游戏（Standalone）或本地服务器（Listen Server），物品不会被复制到客户端，因此需要主动调用OnItemAdded委托。
	// Listen Server：本地玩家既是“发起者”（服务器）也会“接收”数据，但 Unreal 的复制系统默认不会把服务器自己当作“客户端”再发一次给自己，所以那条复制管线根本不会走。
	// Standalone：根本没有网络层，复制管线压根不启动。
	if (GetOwner()->GetNetMode() == NM_ListenServer || GetOwner()->GetNetMode() == NM_Standalone)
	{
		OnItemAdded.Broadcast(NewItem);
	}

	// 通知 Item Component 销毁自己的 Owner Actor
	ItemComponent->PickedUp();
}

void UInv_InventoryComponent::Server_AddStacksToItem_Implementation(UInv_ItemComponent* ItemComponent, int32 StackCount,
                                                                    int32 Remainder)
{
	const FGameplayTag& ItemType = IsValid(ItemComponent)
		                               ? ItemComponent->GetItemManifest().GetItemType()
		                               : FGameplayTag::EmptyTag;
	UInv_InventoryItem* Item = InventoryList.FindFirstItemByType(ItemType);
	if (!IsValid(Item)) return;

	Item->SetTotalStackCount(Item->GetTotalStackCount() + StackCount);

	// 如果全捡光了，就通知 Item Component 销毁自己的 Owner Actor
	// 不然就修改场景中道具的剩余数量——这里要解决的问题是，必须修改 Fragment，所以必须要拿到一个 Mutable Fragment
	if (Remainder == 0)
	{
		ItemComponent->PickedUp();
	}
	else if (FInv_StackableFragment* StackableFragment = ItemComponent->GetItemManifest().GetMutableFragmentOfType<
		FInv_StackableFragment>())
	{
		StackableFragment->SetStackCount(Remainder);
	}
}

void UInv_InventoryComponent::ToggleInventoryMenu()
{
	if (bInventoryMenuOpen)
	{
		CloseInventoryMenu();
	}
	else
	{
		OpenInventoryMenu();
	}
}

void UInv_InventoryComponent::AddRepSubObj(UObject* SubObj)
{
	// IsUsingRegisteredSubObjectList()：当前这个组件或 Actor 是否启用了 bReplicateUsingRegisteredSubObjectList 模式。
	// 如果返回 false，那说明当前是 默认复制模式，你不应该调用 AddReplicatedSubObject()，因为该模式会自动扫描 UPROPERTY 中的子对象指针，而不是依赖显式注册。

	// IsReadyForReplication()：组件是否已初始化并加入复制网络系统，准备开始复制。

	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && IsValid(SubObj))
	{
		AddReplicatedSubObject(SubObj);
	}
}

void UInv_InventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	ConstructInventory();
}

void UInv_InventoryComponent::ConstructInventory()
{
	OwningController = Cast<APlayerController>(GetOwner());
	checkf(OwningController.IsValid(), TEXT("Inventory Component should have a Player Controller as Owner."))
	if (!OwningController->IsLocalController()) return;

	InventoryMenu = CreateWidget<UInv_InventoryBase>(OwningController.Get(), InventoryMenuClass);
	InventoryMenu->AddToViewport();
	CloseInventoryMenu();
}

void UInv_InventoryComponent::OpenInventoryMenu()
{
	if (!IsValid(InventoryMenu)) return;
	InventoryMenu->SetVisibility(ESlateVisibility::Visible);
	bInventoryMenuOpen = true;
	if (!OwningController.IsValid()) return;
	FInputModeGameAndUI InputMode;
	OwningController->SetInputMode(InputMode);
	OwningController->SetShowMouseCursor(true);
}

void UInv_InventoryComponent::CloseInventoryMenu()
{
	if (!IsValid(InventoryMenu)) return;
	InventoryMenu->SetVisibility(ESlateVisibility::Collapsed);
	bInventoryMenuOpen = false;
	if (!OwningController.IsValid()) return;
	FInputModeGameOnly InputMode;
	OwningController->SetInputMode(InputMode);
	OwningController->SetShowMouseCursor(false);
}
