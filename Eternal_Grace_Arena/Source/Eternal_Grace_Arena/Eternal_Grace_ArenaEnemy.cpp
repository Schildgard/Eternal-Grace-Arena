// Fill out your copyright notice in the Description page of Project Settings.


#include "Eternal_Grace_ArenaEnemy.h"
#include "CharacterAnimInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet//GameplayStatics.h"
#include "Perception/PawnSensingComponent.h"
AEternal_Grace_ArenaEnemy::AEternal_Grace_ArenaEnemy()
{
	HPBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HP Bar"));
	HPBarComponent->SetupAttachment(RootComponent);
	HPBarComponent->SetWidgetSpace(EWidgetSpace::Screen);
	HPBarComponent->SetRelativeLocation(FVector(0, 0, 163.0f));
	AttackRange = 350.f;
	HealthbarWidget = nullptr;

	ChasingDistanceThreshold = 150.0f;
	ChasingCountDown = ChasingTimer;
	ReturningToStartPosition = false;

	SensingComponent = CreateDefaultSubobject<UPawnSensingComponent>("Pawn Sensing");
}

bool AEternal_Grace_ArenaEnemy::CheckDistancetoPlayer(float Threshold)
{
	FVector OwnerLocation = GetActorLocation();
	FVector PlayerLocation = UGameplayStatics::GetPlayerCharacter(world, 0)->GetActorLocation();
	//insert distance calculation here
	float Distance = UKismetMathLibrary::Vector_Distance(PlayerLocation, OwnerLocation);
	if (Distance >= Threshold)
	{
		return false;
	}
	else return true;
}

void AEternal_Grace_ArenaEnemy::GetOffMeMove()
{
	if (!CharacterAnimationInstance->isAttacking)
	{
		CharacterAnimationInstance->isAttacking = true;
		if (CharacterAnimationInstance->isGuarding)
		{
			CancelGuard();
		}
		if (AttackMontages[2] != nullptr)
		{
			PlayAnimMontage(AttackMontages[2], 1.0f); // TO DO:REPLACE THAT MAGIC NUMBER WITH SOMETHING ELSE LIKE A POINTER TO A SPECIFIC MONTAGE OR SOMETHING
			FOnMontageEnded InterruptDelegate;
			InterruptDelegate.BindUObject(CharacterAnimationInstance, &UCharacterAnimInstance::InterruptAttack);
			CharacterAnimationInstance->Montage_SetBlendingOutDelegate(InterruptDelegate, AttackMontages[2]);
			CharacterAnimationInstance->Montage_SetEndDelegate(InterruptDelegate, AttackMontages[2]);
		}
	}
}

void AEternal_Grace_ArenaEnemy::BeginPlay()
{
	Super::BeginPlay();
		if (HPBarComponent)
		{
			UUserWidget* Widget = HPBarComponent->GetWidget();
			if (Widget)
			{
				HealthbarWidget = Cast<UEnemy_UI_Healthbar>(Widget);
				if (HealthbarWidget == nullptr)
				{
					UE_LOG(LogTemp, Warning, TEXT("Failed Widget to CustomProgressbar"))
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("HBBar has no widget"))
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("HP Bar is null"))
		}

	if (SensingComponent)
	{
		SensingComponent->OnSeePawn.AddDynamic(this, &AEternal_Grace_ArenaEnemy::SpotPlayer);
	}
}

void AEternal_Grace_ArenaEnemy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (HealthbarWidget)
	{
		if (HealthComponent)
		{
			HealthbarWidget->UpdateProgressBar(HealthbarWidget->Healthbar, HealthComponent->MaxHealth, HealthComponent->CurrentHealth);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Health Component is null"))
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Healthbar widget is null"))
	}

	if(isAggro)
	{
		if(CheckDistancetoPlayer(ChasingDistanceThreshold) == false)
		{
			ChasingCountDown -= DeltaSeconds;
			if(ChasingCountDown <= 0.0f)
			{
				ChasingCountDown = ChasingTimer;
				isAggro = false;
				ReturningToStartPosition = true;
				//INSERT RETURN TO START POSITION
			}
		}
		else
		{
			ChasingCountDown = ChasingTimer;
		}
	}
}

void AEternal_Grace_ArenaEnemy::LightAttack()
{
	if (!CharacterAnimationInstance->isAttacking)
	{
		CharacterAnimationInstance->isAttacking = true;
		RotateTowardsTarget(UGameplayStatics::GetPlayerCharacter(world, 0));
		if (CharacterAnimationInstance->isGuarding)
		{
			CancelGuard();
		}
		int RandomAttackIndex = UKismetMathLibrary::RandomInteger(2);

		PlayAnimMontage(AttackMontages[RandomAttackIndex], 1.0f);

		FOnMontageEnded InterruptDelegate;
		InterruptDelegate.BindUObject(CharacterAnimationInstance, &UCharacterAnimInstance::InterruptAttack);
		CharacterAnimationInstance->Montage_SetBlendingOutDelegate(InterruptDelegate, AttackMontages[RandomAttackIndex]);
		CharacterAnimationInstance->Montage_SetEndDelegate(InterruptDelegate, AttackMontages[RandomAttackIndex]);
		//
		//		FOnMontageEnded CompletedDelegate;
		//		CompletedDelegate.BindUObject(CharacterAnimationInstance, &UCharacterAnimInstance::OnAttackEnd);
		//		CharacterAnimationInstance->Montage_SetEndDelegate(CompletedDelegate, LightAttacks[0]);
		//		UE_LOG(LogTemp, Warning, TEXT("is AGAIN TEST Attacking : %s"), CharacterAnimationInstance->isAttacking ? TEXT("true") : TEXT("false"));
			//UE_LOG(LogTemp, Warning, TEXT("is Attacking : %s"), CharacterAnimationInstance->isAttacking ? TEXT("true") : TEXT("false"));
	}
}

void AEternal_Grace_ArenaEnemy::SpotPlayer(APawn* SpottedPawn)
{
	if (!isAggro && !ReturningToStartPosition)
	{
		isAggro = true;
		UE_LOG(LogTemp, Warning, TEXT("Enemy Spotted Player"))
			//CHECK IF THERE COULD BE A PROBLEM WITH RACE CONDITION IN LINE 118, SINCE THE EVENT CALL ALSO SETS ISAGGRO
	}
}
