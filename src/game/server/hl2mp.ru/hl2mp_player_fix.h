#include "hl2mp_player.h"
#include "filesystem.h"
#include "viewport_panel_names.h"
#include "Sprite.h"

class CHL2MP_Player_fix : public CHL2MP_Player
{
public:
	DECLARE_CLASS(CHL2MP_Player_fix, CHL2MP_Player);
	//DECLARE_DATADESC();
	virtual void Spawn(void);
	virtual void PostThink(void);
	virtual void PickupObject(CBaseEntity *pObject, bool bLimitMassAndSize);
	virtual CBaseEntity * EntSelectSpawnPoint( void );
	virtual void ChangeTeam(int iTeam);
	virtual void TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator );
	virtual int OnTakeDamage( const CTakeDamageInfo &inputInfo );
	virtual void DeathNotice ( CBaseEntity *pVictim );
	virtual CBaseEntity * GetAimTarget ( float range = 3000.0 );
	virtual void PlayerDeathThink( void );
	virtual void UpdateOnRemove( void );

	float m_flNextChargeTime;
	int m_iLoadFirmware;
	float m_fLoadFirmware;
	EHANDLE m_hHealerTarget;
	bool m_bWalkBT;
	float m_fHoldR;
	float m_fStuck;
	float m_fNextHealth;
	CHandle<CHL2MP_Player_fix> m_hBot;
	CHandle<CSprite> m_hHealSprite;
	EHANDLE m_hHealBarTarget;
	bool m_bBlockDeathThink;
	float m_flBlockUse;
};

class CHL2MP_NameReplace {
public:
	CHL2MP_NameReplace() {
		main = new KeyValues("EntityInfo");
		//Msg("[CHL2MP_NameReplace] Class init\n");
	}

	bool Init() {
		if( !main->LoadFromFile(filesystem, "entityinfo.txt", "GAME") )
			return false;

		//Msg("[CHL2MP_NameReplace] Loaded file entityinfo.txt\n");
		return true;
	}

	const char *nameReplace( CBaseEntity *pSearch, CBasePlayer *pPlayer = NULL ) {
		if( pSearch->GetCustomName() != NULL )
			return pSearch->GetCustomName();

		const char *szLanguage = "english";

		if( pPlayer )
			szLanguage = engine->GetClientConVarValue( ENTINDEX( pPlayer ), "cl_language" );

		const char *classname = pSearch->GetClassname();
		KeyValues *pKV = main->FindKey( classname );
		if( pKV != NULL ) {
			classname = pKV->GetString( szLanguage );
			if( strcmpi( classname, "" ) == 0 )
				classname = pKV->GetString( "name", classname );
		}
		//pKV->deleteThis();
		return classname;
	}

	void Clear( void ) {
		//Msg("[CHL2MP_NameReplace] Cleared cache\n");
		curentlevel = NULL;
		main->Clear();
	}

	char *curentlevel;
	KeyValues *main;
};

inline const char *nameReplace(CBaseEntity *pSearch, CBasePlayer *pPlayer = NULL)
{
	static CHL2MP_NameReplace g_NameReplace;
	if( g_NameReplace.curentlevel && strcmpi( g_NameReplace.curentlevel, STRING( gpGlobals->mapname ) ) != 0 )
		g_NameReplace.Clear();

	if( g_NameReplace.curentlevel == NULL ) {
		g_NameReplace.curentlevel = strdup( STRING( gpGlobals->mapname ) );
		g_NameReplace.Init();
	}

	return g_NameReplace.nameReplace( pSearch, pPlayer );
}

inline bool IsAdmin( CBaseEntity *pEntity ) {
	CBasePlayer *pPlayer = ToBasePlayer( pEntity );
	if( !pPlayer )
		return false;

	CSteamID steamID;
	pPlayer->GetSteamID( &steamID );
	KeyValues *manifest;
	KeyValues *sub;
	bool pResult = false;
	manifest = new KeyValues("AdminsList");
	manifest->LoadFromFile(filesystem, "admins.txt", "GAME");
	for( sub = manifest->GetFirstSubKey(); sub != NULL; sub = sub->GetNextKey() )
	{
		if( sub->GetUint64("steamID64") == steamID.ConvertToUint64() ) {
			pResult = true;
			break;
		}
	}
	manifest->deleteThis();
	return pResult;
}

inline CBasePlayer *ToHL2MPFIXPlayer( CBaseEntity *pEntity )
{
	if ( !pEntity || !pEntity->IsPlayer() )
		return NULL;

	return dynamic_cast<CHL2MP_Player_fix*>( pEntity );
}