// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Inv_PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/HUD/Inv_HUDWidget.h"

void AInv_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
		GetLocalPlayer());
	if (IsValid(Subsystem))
	{
		for (UInputMappingContext* IMC : DefaultIMCs)
		{
			if (IsValid(IMC))
			{
				Subsystem->AddMappingContext(IMC, 0);
			}
		}
	}

	CreateHUDWidget();
}

void AInv_PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

	EnhancedInputComponent->BindAction(PrimaryInteractAction, ETriggerEvent::Started, this,
	                                   &AInv_PlayerController::PrimaryInteract);
}

void AInv_PlayerController::PrimaryInteract()
{
	UE_LOG(LogTemp, Log, TEXT("Primary Interact"))
}

void AInv_PlayerController::CreateHUDWidget()
{
	if (!IsLocalController()) return;
	HUDWidget = CreateWidget<UInv_HUDWidget>(this, HUDWidgetClass);
	if (IsValid(HUDWidget))
	{
		HUDWidget->AddToViewport();
	}
}
