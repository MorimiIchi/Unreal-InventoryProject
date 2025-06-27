// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/Spatial/Inv_SpacialInventory.h"

#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Widgets/Inventory/Spatial/Inv_InventoryGrid.h"

void UInv_SpacialInventory::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Button_Equippables->OnClicked.AddDynamic(this, &ThisClass::ShowEquippables);
	Button_Consumables->OnClicked.AddDynamic(this, &ThisClass::ShowConsumables);
	Button_Craftables->OnClicked.AddDynamic(this, &ThisClass::ShowCraftables);

	ShowEquippables();
}

FInv_SlotAvailabilityResult UInv_SpacialInventory::HasRoomForItem(UInv_ItemComponent* ItemComponent) const
{
	return Super::HasRoomForItem(ItemComponent);
}

void UInv_SpacialInventory::ShowEquippables()
{
	SetActiveGrid(Grid_Equippables, Button_Equippables);
}

void UInv_SpacialInventory::ShowConsumables()
{
	SetActiveGrid(Grid_Consumables, Button_Consumables);
}

void UInv_SpacialInventory::ShowCraftables()
{
	SetActiveGrid(Grid_Craftables, Button_Craftables);
}

void UInv_SpacialInventory::SetActiveGrid(UInv_InventoryGrid* Grid, UButton* Button)
{
	DisableButton(Button);
	Switcher->SetActiveWidget(Grid);
}

void UInv_SpacialInventory::DisableButton(UButton* Button)
{
	Button_Equippables->SetIsEnabled(true);
	Button_Consumables->SetIsEnabled(true);
	Button_Craftables->SetIsEnabled(true);
	Button->SetIsEnabled(false);
}
