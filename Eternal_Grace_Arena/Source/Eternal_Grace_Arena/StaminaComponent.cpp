// Fill out your copyright notice in the Description page of Project Settings.


#include "StaminaComponent.h"

// Sets default values for this component's properties
UStaminaComponent::UStaminaComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	MaxStamina = 100.0f;
	CurrentStamina = MaxStamina;
	Exhausted = false;
	StaminaRegenerationRate = 5.0f;
	ReducedStaminaRegenerationRate = 2.0f;
	ExhaustionCountdown = ExhaustionTimer;

	// ...
}


// Called when the game starts
void UStaminaComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

}

void UStaminaComponent::RegenerateStamina(float RegenerationRate)
{
	if (Exhausted == false)
	{
		CurrentStamina += RegenerationRate;
	}
	else
	{
		return;
	}
}


// Called every frame
void UStaminaComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentStamina <= 0)
	{
		CurrentStamina = 0;
		Exhausted = true;
	}
	if (Exhausted)
	{
		ExhaustionCountdown -= DeltaTime;
	}
	if (ExhaustionCountdown <= 0)
	{
		Exhausted = false;
		ExhaustionCountdown = ExhaustionTimer;
	}
}

