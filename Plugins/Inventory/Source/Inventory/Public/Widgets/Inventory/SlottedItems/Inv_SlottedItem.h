// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inv_SlottedItem.generated.h"

class UInv_InventoryItem;
class UImage;
class UTextBlock;

UCLASS()
class INVENTORY_API UInv_SlottedItem : public UUserWidget
{
	GENERATED_BODY()

public:
	bool GetIsStackable() const { return bIsStackable; }
	void SetIsStackable(const bool bStackable) { this->bIsStackable = bStackable; }
	UImage* GetImage_Icon() const { return Image_Icon; }
	int32 GetGridIndex() const { return GridIndex; }
	void SetGridIndex(const int32 InGridIndex) { this->GridIndex = InGridIndex; }
	FIntPoint GetGridDimensions() const { return GridDimensions; }
	void SetGridDimensions(const FIntPoint& InGridDimensions) { this->GridDimensions = InGridDimensions; }
	UInv_InventoryItem* GetInventoryItem() const { return InventoryItem.Get(); }
	void SetInventoryItem(UInv_InventoryItem* InInventoryItem);
	void SetImageBrush(const FSlateBrush& Brush) const;
	void UpdateStackCount(const int32 StackCount) const;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Icon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_StackCount;

	int32 GridIndex;
	FIntPoint GridDimensions;
	TWeakObjectPtr<UInv_InventoryItem> InventoryItem;

public:


private:
	bool bIsStackable{false};

public:
};
