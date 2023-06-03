//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPONCUBEMAP_H
#define WEAPONCUBEMAP_H
#ifdef _WIN32
#pragma once
#endif

class CWeaponCubemap : public CBaseCombatWeapon
{
public:

	DECLARE_CLASS( CWeaponCubemap, CBaseCombatWeapon );

	void	Precache( void );

	bool	HasAnyAmmo( void )	{ return true; }

	void	Spawn( void );

	DECLARE_SERVERCLASS();
};

#endif // WEAPONCUBEMAP_H