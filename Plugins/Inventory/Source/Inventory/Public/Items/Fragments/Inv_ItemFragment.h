#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "Inv_ItemFragment.generated.h"

USTRUCT(BlueprintType)
struct FInv_ItemFragment
{
	GENERATED_BODY()

	FInv_ItemFragment()
	{
	}

	// 复制
	FInv_ItemFragment(const FInv_ItemFragment&) = default;
	FInv_ItemFragment& operator=(const FInv_ItemFragment&) = default;
	// 移动
	FInv_ItemFragment(FInv_ItemFragment&&) = default;
	FInv_ItemFragment& operator=(FInv_ItemFragment&&) = default;

	virtual ~FInv_ItemFragment()
	{
	}

private:
	UPROPERTY(EditAnywhere, Category="Inventory")
	FGameplayTag FragmentTag = FGameplayTag::EmptyTag;
};

USTRUCT(BlueprintType)
struct FInv_GridFragment : public FInv_ItemFragment
{
	GENERATED_BODY()

public:
	FIntPoint GetGridSize() const { return GridSize; }
	void SetGridSize(const FIntPoint& Size) { GridSize = Size; }
	float GetGridPadding() const { return GridPadding; }
	void SetGridPadding(const float InGridPadding) { GridPadding = InGridPadding; }

private:
	UPROPERTY(EditAnywhere, Category="Inventory")
	FIntPoint GridSize{1, 1};

	UPROPERTY(EditAnywhere, Category="Inventory")
	float GridPadding{0.f};
};
