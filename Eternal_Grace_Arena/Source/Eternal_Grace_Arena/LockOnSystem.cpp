// Fill out your copyright notice in the Description page of Project Settings.


#include "LockOnSystem.h"
#include "Eternal_Grace_ArenaCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "CharacterAnimInstance.h"
#include "Blueprint//UserWidget.h"


ULockOnSystem::ULockOnSystem()
{
	LockedOnTarget = nullptr;
	DistanceInfluenceOnZ = 2.0f;
	CharacterRotationInterpolation = 10.0f;
	CameraRotationInterpolation = 2.0f;

	MinZOffset = 30.f;
	MaxZDrop = 150.f;
	VisualizerWidgetClass = nullptr;
	Visualizer = nullptr;

}

void ULockOnSystem::LockOnTarget(AEternal_Grace_ArenaCharacter* NewTarget, AEternal_Grace_ArenaCharacter* LockingActor)
{
	LockedOnTarget = NewTarget;

	if (VisualizerWidgetClass)
	{
		if (!Visualizer)
		{
			Visualizer = CreateWidget<UUserWidget>(GetWorld()->GetFirstPlayerController(), VisualizerWidgetClass);
		}
		if (Visualizer)
		{
			Visualizer->AddToViewport();
		}
	}

}

void ULockOnSystem::UpdateLockOn(AEternal_Grace_ArenaCharacter* LockingActor, float DeltaSeconds)
{
	if (!LockedOnTarget || !LockingActor)
	{
		return;
	}
	//IT IS WEIRD THAT THE LOCK ON SYSTEM GETS ITS PLAYER CONTROLLER BUT THE PLAYER CONTROLLER CALLS THIS FUNCTION..TODO: CHANGE THIS FUNCTION TO ONLY DO CALCULATIONS AND ROTATE IN THE CONTROLLER
	APlayerController* CurrentPlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	//While sprinting the character rotates freely
	if (!LockingActor->CharacterAnimationInstance->isRunning)
	{
		LockingActor->RotateTowardsTarget(LockedOnTarget);
	}

	//GET RELEVANT POSITIONS
	FVector CameraPosition = LockingActor->GetFollowCamera()->GetComponentLocation();
	FVector TargetPosition = LockedOnTarget->GetTargetLocation();

	//DIVIDE DISTANCE TO OPTIMIZE CAMERA Z ANGLE
	float Influence = UKismetMathLibrary::Vector_Distance(CameraPosition, TargetPosition) / DistanceInfluenceOnZ;
	float ZAdjustment = FMath::Clamp(Influence, MinZOffset, MaxZDrop);
	FVector TargetCameraDirection = FVector(TargetPosition.X, TargetPosition.Y, TargetPosition.Z - ZAdjustment);

	//SET CAMERA ROTATION
	FRotator CameraLook = UKismetMathLibrary::FindLookAtRotation(CameraPosition, TargetCameraDirection);
	FRotator InterpolatedRotation = FMath::RInterpTo(CurrentPlayerController->GetControlRotation(), CameraLook, DeltaSeconds, CameraRotationInterpolation);

	// ONLY USE INTERPOLATED PITCH AND YAW
	FRotator UltimateRotation = FRotator(InterpolatedRotation.Pitch, InterpolatedRotation.Yaw, CurrentPlayerController->GetControlRotation().Roll);
	CurrentPlayerController->SetControlRotation(UltimateRotation);


	//UPDATE LOCK ON VISUALIZER POSITION
	if (Visualizer && LockedOnTarget)
	{
		FVector TargetSocketLocation = LockedOnTarget->GetMesh()->GetSocketLocation(FName("Target"));
		FVector2D ScreenPosition;
		bool Projected = GetWorld()->GetFirstPlayerController()->ProjectWorldLocationToScreen(TargetSocketLocation, ScreenPosition);

		if (Projected)
		{
			Visualizer->SetPositionInViewport(ScreenPosition, true);
		}
	}
}

void ULockOnSystem::UnlockTarget()
{
	LockedOnTarget = nullptr;
	if (Visualizer)
	{
		Visualizer->RemoveFromViewport();
	}
}

AEternal_Grace_ArenaCharacter* ULockOnSystem::GetLockedOnTarget()
{
	return LockedOnTarget;
}
