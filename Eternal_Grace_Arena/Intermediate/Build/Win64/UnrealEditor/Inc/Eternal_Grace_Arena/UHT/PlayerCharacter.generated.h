// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "PlayerCharacter.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
class AActor;
#ifdef ETERNAL_GRACE_ARENA_PlayerCharacter_generated_h
#error "PlayerCharacter.generated.h already included, missing '#pragma once' in PlayerCharacter.h"
#endif
#define ETERNAL_GRACE_ARENA_PlayerCharacter_generated_h

#define FID_Repository_Eternal_Grace_Arena_Eternal_Grace_Arena_Source_Eternal_Grace_Arena_PlayerCharacter_h_16_RPC_WRAPPERS_NO_PURE_DECLS \
	DECLARE_FUNCTION(execHeavyAttack); \
	DECLARE_FUNCTION(execChargeHeavyAttack); \
	DECLARE_FUNCTION(execCancelSprint); \
	DECLARE_FUNCTION(execSprint); \
	DECLARE_FUNCTION(execDodge); \
	DECLARE_FUNCTION(execSprintAttack); \
	DECLARE_FUNCTION(execIncreaseChargePower); \
	DECLARE_FUNCTION(execInteract); \
	DECLARE_FUNCTION(execGuardCounterAttack); \
	DECLARE_FUNCTION(execDisengageLockOn); \
	DECLARE_FUNCTION(execEngageLockOn); \
	DECLARE_FUNCTION(execScanForTargets); \
	DECLARE_FUNCTION(execFindNearestTarget); \
	DECLARE_FUNCTION(execSwitchLockOnTarget); \
	DECLARE_FUNCTION(execToggleLockOn);


#define FID_Repository_Eternal_Grace_Arena_Eternal_Grace_Arena_Source_Eternal_Grace_Arena_PlayerCharacter_h_16_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesAPlayerCharacter(); \
	friend struct Z_Construct_UClass_APlayerCharacter_Statics; \
public: \
	DECLARE_CLASS(APlayerCharacter, AEternal_Grace_ArenaCharacter, COMPILED_IN_FLAGS(0 | CLASS_Config), CASTCLASS_None, TEXT("/Script/Eternal_Grace_Arena"), NO_API) \
	DECLARE_SERIALIZER(APlayerCharacter)


#define FID_Repository_Eternal_Grace_Arena_Eternal_Grace_Arena_Source_Eternal_Grace_Arena_PlayerCharacter_h_16_ENHANCED_CONSTRUCTORS \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	APlayerCharacter(APlayerCharacter&&); \
	APlayerCharacter(const APlayerCharacter&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, APlayerCharacter); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(APlayerCharacter); \
	DEFINE_DEFAULT_CONSTRUCTOR_CALL(APlayerCharacter) \
	NO_API virtual ~APlayerCharacter();


#define FID_Repository_Eternal_Grace_Arena_Eternal_Grace_Arena_Source_Eternal_Grace_Arena_PlayerCharacter_h_13_PROLOG
#define FID_Repository_Eternal_Grace_Arena_Eternal_Grace_Arena_Source_Eternal_Grace_Arena_PlayerCharacter_h_16_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_Repository_Eternal_Grace_Arena_Eternal_Grace_Arena_Source_Eternal_Grace_Arena_PlayerCharacter_h_16_RPC_WRAPPERS_NO_PURE_DECLS \
	FID_Repository_Eternal_Grace_Arena_Eternal_Grace_Arena_Source_Eternal_Grace_Arena_PlayerCharacter_h_16_INCLASS_NO_PURE_DECLS \
	FID_Repository_Eternal_Grace_Arena_Eternal_Grace_Arena_Source_Eternal_Grace_Arena_PlayerCharacter_h_16_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


template<> ETERNAL_GRACE_ARENA_API UClass* StaticClass<class APlayerCharacter>();

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_Repository_Eternal_Grace_Arena_Eternal_Grace_Arena_Source_Eternal_Grace_Arena_PlayerCharacter_h


PRAGMA_ENABLE_DEPRECATION_WARNINGS
