// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Utils/Inv_WidgetUtils.h"

#include "Blueprint/SlateBlueprintLibrary.h"
#include "Components/Widget.h"
#include "Items/Components/Inv_ItemComponent.h"

int32 UInv_WidgetUtils::GetIndexFromPosition(const FIntPoint& Position, const int32 Columns)
{
	return Position.X + Position.Y * Columns;
}

FIntPoint UInv_WidgetUtils::GetPositionFromIndex(const int32 Index, const int32 Columns)
{
	return FIntPoint(Index % Columns, Index / Columns);
}

FVector2D UInv_WidgetUtils::GetWidgetPosition(UWidget* Widget)
{
	// 目标：拿到控件左上角位置在视口坐标系中的位置

	// 拿到控件的几何信息
	const FGeometry Geometry = Widget->GetCachedGeometry();
	// 拿到控件的本地左上角位置，以便在之后转换为视口坐标
	// 本地坐标系：相对于控件左上角原点的坐标系
	// 因此，这一步相当于直接拿控件的原点
	const FVector2D LocalTopLeft = USlateBlueprintLibrary::GetLocalTopLeft(Geometry);

	FVector2D PixelPosition;
	FVector2D ViewportPosition;
	USlateBlueprintLibrary::LocalToViewport(Widget, Geometry, LocalTopLeft, PixelPosition, ViewportPosition);

	return ViewportPosition;
}

FVector2D UInv_WidgetUtils::GetWidgetSize(UWidget* Widget)
{
	const FGeometry Geometry = Widget->GetCachedGeometry();
	return Geometry.GetLocalSize();
}

bool UInv_WidgetUtils::IsWithinBounds(const FVector2D& BoundaryPos, const FVector2D& WidgetSize,
                                      const FVector2D& MousePos)
{
	return MousePos.X >= BoundaryPos.X && MousePos.X <= (BoundaryPos.X + WidgetSize.X) &&
		MousePos.Y >= BoundaryPos.Y && MousePos.Y <= (BoundaryPos.Y + WidgetSize.Y);
}
