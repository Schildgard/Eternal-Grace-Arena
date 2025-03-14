// Fill out your copyright notice in the Description page of Project Settings.


#include "Spell_Teleport.h"
#include "Kismet//GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Eternal_Grace_ArenaCharacter.h"
#include "Eternal_Grace_ArenaEnemy_Caster.h"
#include "SpellComponent.h"
#include "Enemy_MageBoss.h"

void ASpell_Teleport::BeginPlay()
{
	Super::BeginPlay();
	Teleport();
}

void ASpell_Teleport::Teleport()
{
	if (InstigatingActor)
	{
		// TODO: NEED TO REBUILD THIS SYSTEM SO EVERY CHARACTER CAN HAS A SPELL COMPONENT AND HAS A GETTER FUNCTION FOR IT, BUT IT IS NULLPTR BUT FOR THE CASTERS
		AEnemy_MageBoss* Caster = Cast <AEnemy_MageBoss>(InstigatingActor); //REPLACE THIS WITH A GET SPELL COMPONENT FUNCTION TO MAKE IT FLEXIBLE BETWEEN BOSS AND NORMAL ENEMY	
		if (Caster)
		{
			FVector TargetPosition = Caster->GetSpellComponent()->GetTargetPosition();
			Caster->SetActorLocation(TargetPosition, false, nullptr, ETeleportType::ResetPhysics);
			if (SpawningSound)
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), SpawningSound, TargetPosition, 1.0f);
			}
			if (SpellVFX->GetAsset())
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(Caster->world, SpellVFX->GetAsset(), TargetPosition);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Teleport Skill had no Instigating Actor"))
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Teleport Skill had no Instigating Actor"))
	}

}

