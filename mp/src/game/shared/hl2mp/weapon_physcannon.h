//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_PHYSCANNON_H
#define WEAPON_PHYSCANNON_H
#ifdef _WIN32
#pragma once
#endif

//-----------------------------------------------------------------------------
// Do we have the super-phys gun?
//-----------------------------------------------------------------------------
bool PlayerHasMegaPhysCannon();

// force the physcannon to drop an object (if carried)
void PhysCannonForceDrop( CBaseCombatWeapon *pActiveWeapon, CBaseEntity *pOnlyIfHoldingThis );
void PhysCannonBeginUpgrade( CBaseAnimating *pAnim );

#ifdef SDK2013CE_BUILD
bool PhysCannonAccountableForObject( CBaseCombatWeapon *pPhysCannon, CBaseEntity *pObject );
#endif

bool PlayerPickupControllerIsHoldingEntity( CBaseEntity *pPickupController, CBaseEntity *pHeldEntity );
float PlayerPickupGetHeldObjectMass( CBaseEntity *pPickupControllerEntity, IPhysicsObject *pHeldObject );
float PhysCannonGetHeldObjectMass( CBaseCombatWeapon *pActiveWeapon, IPhysicsObject *pHeldObject );

CBaseEntity *PhysCannonGetHeldEntity( CBaseCombatWeapon *pActiveWeapon );
CBaseEntity *GetPlayerHeldEntity( CBasePlayer *pPlayer );


#endif // WEAPON_PHYSCANNON_H
