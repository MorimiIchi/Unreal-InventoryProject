// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Inventory/InventoryBase/Inv_InventoryBase.h"
#include "Inv_SpacialInventory.generated.h"

class UButton;
class UWidgetSwitcher;
class UInv_InventoryGrid;
/**
 * 
 */
UCLASS()
class INVENTORY_API UInv_SpacialInventory : public UInv_InventoryBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

private:
	UFUNCTION()
	void ShowEquippables();

	UFUNCTION()
	void ShowConsumables();

	UFUNCTION()
	void ShowCraftables();

	UFUNCTION()
	void SetActiveGrid(UInv_InventoryGrid* Grid, UButton* Button);
	void DisableButton(UButton* Button);

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UWidgetSwitcher> Switcher;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UInv_InventoryGrid> Grid_Equippables;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UInv_InventoryGrid> Grid_Consumables;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UInv_InventoryGrid> Grid_Craftables;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Button_Equippables;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Button_Consumables;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Button_Craftables;
};
