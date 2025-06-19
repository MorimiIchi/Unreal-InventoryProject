// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Inv_PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/HUD/Inv_HUDWidget.h"

AInv_PlayerController::AInv_PlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	TraceLength = 1000.f;
	TraceChannel = ECC_GameTraceChannel2;
}

void AInv_PlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	TraceForItem();
}

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

void AInv_PlayerController::TraceForItem()
{
	if (!GEngine || !GEngine->GameViewport) return;

	FVector2D ViewportSize;
	GEngine->GameViewport->GetViewportSize(ViewportSize);

	const FVector2D ScreenCenter = ViewportSize / 2.0f;
	FVector TraceStart;
	FVector TraceDirection;
	if (!DeprojectScreenPositionToWorld(ScreenCenter.X, ScreenCenter.Y, TraceStart, TraceDirection)) return;

	const FVector TraceEnd = TraceStart + TraceDirection * TraceLength;

	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, TraceChannel);

	LastActor = ThisActor;
	ThisActor = HitResult.GetActor();

	if (ThisActor == LastActor) return;

	if (ThisActor.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Started tracing a new actor."))
	}

	if (LastActor.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Started tracing last actor."))
	}
}
