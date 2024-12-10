// Copyright Epic Games, Inc. All Rights Reserved.

#include "Eternal_Grace_ArenaCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "CharacterAnimInstance.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "I_Targetable.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AEternal_Grace_ArenaCharacter

AEternal_Grace_ArenaCharacter::AEternal_Grace_ArenaCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	LockedOnTarget = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// Input

void AEternal_Grace_ArenaCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AEternal_Grace_ArenaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AEternal_Grace_ArenaCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AEternal_Grace_ArenaCharacter::Look);

		//Sprinting
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &AEternal_Grace_ArenaCharacter::Sprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AEternal_Grace_ArenaCharacter::CancelSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Canceled, this, &AEternal_Grace_ArenaCharacter::CancelSprint);

		//Guarding
		EnhancedInputComponent->BindAction(GuardAction, ETriggerEvent::Triggered, this, &AEternal_Grace_ArenaCharacter::Guard);
		EnhancedInputComponent->BindAction(GuardAction, ETriggerEvent::Completed, this, &AEternal_Grace_ArenaCharacter::CancelGuard);
		EnhancedInputComponent->BindAction(GuardAction, ETriggerEvent::Canceled, this, &AEternal_Grace_ArenaCharacter::CancelGuard);

		//Light Attack
		EnhancedInputComponent->BindAction(LightAttackAction, ETriggerEvent::Triggered, this, &AEternal_Grace_ArenaCharacter::LightAttack);

		//Heavy Attack
		EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Started, this, &AEternal_Grace_ArenaCharacter::ChargeHeavyAttack);
		EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Triggered, this, &AEternal_Grace_ArenaCharacter::IncreaseChargePower);
		EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Completed, this, &AEternal_Grace_ArenaCharacter::HeavyAttack);

		//Lock On
		EnhancedInputComponent->BindAction(ToggleLockOnAction, ETriggerEvent::Triggered, this, &AEternal_Grace_ArenaCharacter::ToggleLockOn);
		EnhancedInputComponent->BindAction(SwitchLockOnTargetAction, ETriggerEvent::Triggered, this, &AEternal_Grace_ArenaCharacter::SwitchLockOnTarget);

	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AEternal_Grace_ArenaCharacter::BeginPlay()
{
	Super::BeginPlay();
	world = GetWorld();
	InitializeAnimationInstance();

}

void AEternal_Grace_ArenaCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (CharacterAnimationInstance && CharacterAnimationInstance->isLockedOn && LockedOnTarget != nullptr)
	{
		RotateTowardsTarget(LockedOnTarget);
	}
}

void AEternal_Grace_ArenaCharacter::InitializeAnimationInstance()
{

	//CHECK IF CHARACTER HAS A VIABLE ANIMATION INSTANCE AND GET IT
	if (UAnimInstance* CurrentAnimInstance = GetMesh()->GetAnimInstance())
	{
		//CAST ANIMATION INSTANCE TO SPECIFIC CHARACTER ANIMATION INSTANCE
		CharacterAnimationInstance = Cast<UCharacterAnimInstance>(CurrentAnimInstance);
		UE_LOG(LogTemp, Warning, TEXT("Animation Instance set"))
			return;
	}
	UE_LOG(LogTemp, Warning, TEXT("Animation Instance could not be found"))
}

void AEternal_Grace_ArenaCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AEternal_Grace_ArenaCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}


void AEternal_Grace_ArenaCharacter::Sprint()
{
	CharacterAnimationInstance->isRunning = true;

}

void AEternal_Grace_ArenaCharacter::CancelSprint()
{
	CharacterAnimationInstance->isRunning = false;
}

void AEternal_Grace_ArenaCharacter::LightAttack()
{
	if (!CharacterAnimationInstance->isAttacking)
	{
		CharacterAnimationInstance->isAttacking = true;

		switch (CharacterAnimationInstance->attackCount)
		{
		case 0:
			PlayAnimMontage(LightAttack01, 1.0f);
			break;
		case 1:
			PlayAnimMontage(LightAttack02, 1.0f);
			break;
		case 2:
			PlayAnimMontage(LightAttack03, 1.0f);
			break;
		case 3:
			PlayAnimMontage(LightAttack04, 1.0f);
			break;
		case 4:
			PlayAnimMontage(LightAttack05, 1.0f);
			break;
		default:
			break;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Character Already is Attacking"))
	}
}

void AEternal_Grace_ArenaCharacter::HeavyAttack()
{
	//When Heavy Attack Button is released from charge Position the Player Character unleashes his Heavy Attack 
	if (CharacterAnimationInstance->isCharging)
	{
		CharacterAnimationInstance->isCharging = false; //LEAVE CHARGING STATE
		CharacterAnimationInstance->isAttacking = true; //SET PLAYER IN ATTACK STATE, SO THE ANIMATION CAN NOT BE INTERUPTED BY A LIGHT ATTACK COMMAND
		CharacterAnimationInstance->isInHeavyAttack = true; // SET PLAYER IN HEAVY ATTACK STATE, SO ANOTHER HEAVY ATTACK COMMAND TRIGGERS THE SECOND ATTACK ANIM
		UE_LOG(LogTemp, Warning, TEXT("Character Releases Attack"))
			PlayAnimMontage(HeavyAttack01, 1.0f);
		currentChargePower = 0; //TODO: CHANGE THIS SO THAT A NOTIFY EVENT RESETS THE CHARGE POWER
	}
	else if (CharacterAnimationInstance->isInHeavyAttack)
	{
		//IF PLAYER IS IN HEAVY ATTACK, A SECOND HEAVY ATTACK COMMAND TRIGGERS THE FOLLOW UP ANIMATION
		CharacterAnimationInstance->isInHeavyAttack = false;//LEAVE HEAVY ATTACK STATE, SO THE FOLLOW UP ANIMATION ONLY TRIGGERS ONCE
		CharacterAnimationInstance->isAttacking = true; //SET IS ATTACKING TO TRUE TO MAKE SURE THE FOLLOW UP ANIMATION IS NOT CANCELED BY LIGHT ATTACK COMMAND
		PlayAnimMontage(HeavyAttack02, 1.0f);
		UE_LOG(LogTemp, Warning, TEXT("Character Already is not Charging"))
	}
}

void AEternal_Grace_ArenaCharacter::ChargeHeavyAttack()
{
	//When Heavy Attack Button is first pressed, Player goes into Charge Stand
	if (!CharacterAnimationInstance->isCharging && !CharacterAnimationInstance->isAttacking)
	{
		CharacterAnimationInstance->isCharging = true;
		PlayAnimMontage(ChargeAttack, 1.0f);
		UE_LOG(LogTemp, Warning, TEXT("Character Charges Attack"))
	}

}

void AEternal_Grace_ArenaCharacter::IncreaseChargePower()
{
	//When further Hold down, the Player remains in Charge Position and charges Attack power
	if (CharacterAnimationInstance->isCharging)
	{
		currentChargePower += world->DeltaTimeSeconds;
		if (currentChargePower >= maxChargePower)
		{
			currentChargePower = maxChargePower;
			HeavyAttack();
		}
	}
}

void AEternal_Grace_ArenaCharacter::SprintAttack()
{
	PlayAnimMontage(RunningAttack);
}

void AEternal_Grace_ArenaCharacter::GuardCounter()
{
	PlayAnimMontage(GuardCounterAttack);
}

void AEternal_Grace_ArenaCharacter::Guard()
{
	CharacterAnimationInstance->isGuarding = true;
}

void AEternal_Grace_ArenaCharacter::CancelGuard()
{
	CharacterAnimationInstance->isGuarding = false;
}

void AEternal_Grace_ArenaCharacter::BlockAttack()
{
	PlayAnimMontage(Block);
}

void AEternal_Grace_ArenaCharacter::BlockHeavy()
{
	PlayAnimMontage(BlockHeavyAttack);
}

void AEternal_Grace_ArenaCharacter::ToggleLockOn()
{
	if (CharacterAnimationInstance->isLockedOn == false)
	{
		AActor* ClosestTarget = FindNearestTarget();
		EngageLockOn(ClosestTarget);
		UE_LOG(LogTemp, Warning, TEXT("Lock On"))
	}
	else
	{
		DisengageLockOn();
		UE_LOG(LogTemp, Warning, TEXT("Lock On Off"))
	}
}

void AEternal_Grace_ArenaCharacter::SwitchLockOnTarget()
{
}

AActor* AEternal_Grace_ArenaCharacter::FindNearestTarget()
{
	//GET SCANNED ACTOR ARRAY FROM SCAN FUNCTION
	TArray<AActor*> ScannedActors = ScanForTargets();
	if (ScannedActors.Num() <= 0)
	{
			UE_LOG(LogTemp, Warning, TEXT("Find Nearest Target: No Actors Scanned"))
			return nullptr;
	}


	AActor* ClosestTarget = nullptr;
	float comparison = 0.0f;
	float distance = 10000.0f;

	//ITERATE THROUGH SCANNED ACTORS TO FIND SMALLEST DISTANCE
	for (AActor* Actor : ScannedActors)
	{
		comparison = GetDistanceTo(Actor);
		if (comparison <= distance)
		{
			distance = comparison; //SAVE SMALLEST DISTANCE
			ClosestTarget = Actor; //SAVE ACTOR IF DISTANCE IS SMALLEST
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Nearest Actor: %s "), *ClosestTarget->GetName());
	return ClosestTarget;
}

TArray<AActor*> AEternal_Grace_ArenaCharacter::ScanForTargets()
{
	FVector PlayerPosition = GetActorLocation();
	TArray<FHitResult> ScanHits; //SET UP A LIST FOR HITTED OBJECTS
	TArray<AActor*> ActorsToIgnore; //SET UP A LIST SO ACTORS WONT GET SCANNED MULTIPLE TIMES
	TArray<AActor*> ScannedActors;
	UKismetSystemLibrary::SphereTraceMultiForObjects(world, PlayerPosition, PlayerPosition, 750.0f, ObjectTypes, true, ActorsToIgnore, EDrawDebugTrace::ForDuration, ScanHits, true, FLinearColor::Red, FLinearColor::Green, 5.0f);

	if (ScanHits.Num() > 0)
	{
		//ITERARE THROUGH SCANHITS
		for (const FHitResult& Hit : ScanHits)
		{
			AActor* HitActor = Hit.GetActor();
			//ADD HITTED ACTOR TO IGNORE AND VIABLE TARGET LIST
			if (HitActor && !ActorsToIgnore.Contains(HitActor) && HitActor->Implements<UI_Targetable>())
			{
				ScannedActors.Add(HitActor);
				ActorsToIgnore.Add(HitActor);
			}
		}
	}
	return ScannedActors;
}

void AEternal_Grace_ArenaCharacter::EngageLockOn(AActor* Target)
{
	CharacterAnimationInstance->isLockedOn = true;
	LockedOnTarget = Target;

	//FIX CRASH!!!!
	if (LockedOnTarget != nullptr)
	{
		FRotator Look = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), LockedOnTarget->GetActorLocation());
		SetActorRotation(Look);
	}
	UE_LOG(LogTemp, Warning, TEXT("No LockedOnTarget"))
}

void AEternal_Grace_ArenaCharacter::DisengageLockOn()
{
	CharacterAnimationInstance->isLockedOn = false;
	LockedOnTarget = nullptr;
}

void AEternal_Grace_ArenaCharacter::RotateTowardsTarget(AActor* Target)
{
	//GET LOOK ROTATION BETWEEN PLAYER AND TARGET
	FRotator Look = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target->GetActorLocation());
	//BREAK LOOK ROTATION TO YAW ONLY
	FRotator DesiredRotation = FRotator(0, Look.Yaw, 0);
	SetActorRotation(DesiredRotation);
}

void AEternal_Grace_ArenaCharacter::GuardBreak()
{
	PlayAnimMontage(GuardBreakEvent);
}

void AEternal_Grace_ArenaCharacter::Stagger(int StaggerType)
{
	PlayAnimMontage(StaggerAnims[StaggerType]);
}

void AEternal_Grace_ArenaCharacter::Dodge()
{
	PlayAnimMontage(DodgeAction);
}

void AEternal_Grace_ArenaCharacter::Die()
{
	PlayAnimMontage(DeathAnimation);
}

