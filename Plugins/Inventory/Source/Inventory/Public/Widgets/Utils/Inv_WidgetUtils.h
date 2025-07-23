// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types/Inv_GridTypes.h"
#include "Inv_WidgetUtils.generated.h"

class UWidget;
class UInv_ItemComponent;
/**
 * 
 */
UCLASS()
class INVENTORY_API UInv_WidgetUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * 由单元格坐标获得其 Index
	 * @param Position 单元格坐标
	 * @param Columns  总列数
	 * @return 单元格 Index
	 */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	static int32 GetIndexFromPosition(const FIntPoint& Position, const int32 Columns);
	
	UFUNCTION(BlueprintCallable, Category="Inventory")
	static FIntPoint GetPositionFromIndex(const int32 Index,const int32 Columns);

	UFUNCTION(BlueprintCallable,Category="Inventory")
	static FVector2D GetWidgetPosition(UWidget* Widget);

	UFUNCTION(BlueprintCallable,Category="Inventory")
	static FVector2D GetWidgetSize(UWidget* Widget);

	UFUNCTION(BlueprintCallable,Category="Inventory")
	static bool IsWithinBounds(const FVector2D& BoundaryPos, const FVector2D& WidgetSize, const FVector2D& MousePos);
};
