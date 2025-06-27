// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryManagement/Components/Inv_InventoryComponent.h"

#include "Net/UnrealNetwork.h"
#include  "Widgets/Inventory/InventoryBase/Inv_InventoryBase.h"


UInv_InventoryComponent::UInv_InventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
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

void UInv_InventoryComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, InventoryList);
}

void UInv_InventoryComponent::TryAddItem(UInv_ItemComponent* ItemComponent)
{
	FInv_SlotAvailabilityResult Result = InventoryMenu->HasRoomForItem(ItemComponent);
	if (Result.TotalRoomToFill == 0)
	{
		NoRoomInInventory.Broadcast();
		return;
	}

	// 添加道具到道具栏中
	if (Result.Item.IsValid() && Result.bStackable)
	{
		// 为已经存在于道具栏中的单格道具添加叠加数量
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

	// 通知 Item Component 销毁自己的 Owner Actor
}

void UInv_InventoryComponent::Server_AddStacksToItem_Implementation(UInv_ItemComponent* ItemComponent, int32 StackCount,
                                                                    int32 Remainder)
{
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
