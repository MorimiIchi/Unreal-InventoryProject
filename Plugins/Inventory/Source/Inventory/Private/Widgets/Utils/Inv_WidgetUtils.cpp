// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Utils/Inv_WidgetUtils.h"

#include "Items/Components/Inv_ItemComponent.h"

int32 UInv_WidgetUtils::GetIndexFromPosition(const FIntPoint& Position, const int32 Columns)
{
	return Position.X + Position.Y * Columns;
}

EInv_ItemCategory UInv_WidgetUtils::GetItemCategoryFromItemComp(UInv_ItemComponent* ItemComponent)
{
	if (!IsValid(ItemComponent)) return  EInv_ItemCategory::None;
	return ItemComponent->GetItemManifest().GetItemCategory();
}
