// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/SlottedItems/Inv_SlottedItem.h"
#include "Items/Inv_InventoryItem.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UInv_SlottedItem::SetInventoryItem(UInv_InventoryItem* InInventoryItem)
{
	InventoryItem = InInventoryItem;
}

void UInv_SlottedItem::SetImageBrush(const FSlateBrush& Brush) const
{
	Image_Icon->SetBrush(Brush);
}

void UInv_SlottedItem::UpdateStackCount(const int32 StackCount) const
{
	if (StackCount > 0)
	{
		Text_StackCount->SetVisibility(ESlateVisibility::Visible);
		Text_StackCount->SetText(FText::AsNumber(StackCount));
	}
	else
	{
		Text_StackCount->SetVisibility(ESlateVisibility::Collapsed);
	}
}
