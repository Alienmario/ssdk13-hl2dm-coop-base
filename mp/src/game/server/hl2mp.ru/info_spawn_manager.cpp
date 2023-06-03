#include "cbase.h"
#include "info_player_spawn.h"

class ism : public CBaseEntity
{
public:
	DECLARE_CLASS( ism, CBaseEntity );
	DECLARE_DATADESC();

	void SetCheckPoint( inputdata_t &inputData );
	void DisableOther( CBaseEntity *pIgnore );
	void TeleportPlayers( inputdata_t &inputData );
	void MovePlayers( inputdata_t &inputData );
	void TpAllPlayer( const Vector newPosition, const QAngle newAngles );

	COutputEvent m_OnFinish;

	EHANDLE m_hSpot;
};

LINK_ENTITY_TO_CLASS( info_spawn_manager, ism );
LINK_ENTITY_TO_CLASS( info_player_checkpoint, ism );
LINK_ENTITY_TO_CLASS( info_checkpoint, ism );

BEGIN_DATADESC( ism )

	DEFINE_INPUTFUNC( FIELD_STRING, "SetCheckPoint", SetCheckPoint ),
	DEFINE_INPUTFUNC( FIELD_STRING, "TeleportPlayers", TeleportPlayers ),
	DEFINE_INPUTFUNC( FIELD_STRING, "MovePlayers", MovePlayers ),
	DEFINE_OUTPUT( m_OnFinish, "OnFinish"),
	
END_DATADESC()

void ism::SetCheckPoint( inputdata_t &inputData )
{
	CBaseEntity *pMaster = gEntList.FindEntityByName( NULL, inputData.value.StringID(), NULL, this );
	CSpawnPoint *pSpot = dynamic_cast<CSpawnPoint *>( pMaster );
	
	if( pSpot ) {
		DisableOther( pSpot );
		pSpot->m_iDisabled = FALSE;
		m_hSpot = pSpot;
		return;
	}

	pSpot = (CSpawnPoint *)CreateEntityByName("info_player_deathmatch");
	if( pSpot ) {
		DisableOther( pSpot );
		pSpot->SetLocalOrigin( GetAbsOrigin() );
		pSpot->SetLocalAngles( GetAbsAngles() );
		pSpot->m_iDisabled = FALSE;
		m_hSpot = pSpot;
		if( GetParent() ) {
			pSpot->SetParent( GetParent() );
		}
	}
}

const char *gpPointListDelete[] = {
	"info_player_coop",
	"info_player_deathmatch",
	"info_player_start",
	"info_player_combine",
	"info_player_rebel",
};

void ism::DisableOther( CBaseEntity *pIgnore )
{
	int nHeads = ARRAYSIZE( gpPointListDelete );
	int i;
	for ( i = 0; i < nHeads; ++i )
	{
		CBaseEntity *pSpawn = NULL;
		while ( ( pSpawn = gEntList.FindEntityByClassname( pSpawn, gpPointListDelete[i] ) ) != NULL )
		{
			CSpawnPoint *pSpot = dynamic_cast<CSpawnPoint *>( pSpawn );
			if( pSpot && ( pSpot != pIgnore ) ) {
				pSpot->m_iDisabled = TRUE;
			}
		}
	}
}

void ism::TeleportPlayers( inputdata_t &inputData )
{
	CBaseEntity *pMaster = gEntList.FindEntityByName( NULL, inputData.value.StringID(), NULL, this );
	if( pMaster ) {
		TpAllPlayer( pMaster->GetAbsOrigin(), pMaster->GetAbsAngles() );	
	}
}

void ism::MovePlayers( inputdata_t &inputData )
{
	if( m_hSpot )
		TpAllPlayer( m_hSpot->GetAbsOrigin(), m_hSpot->GetAbsAngles() );
	else	
		TpAllPlayer( GetAbsOrigin(), GetAbsAngles() );
	m_OnFinish.FireOutput( inputData.pActivator, inputData.pCaller );
}

#define	FL_SAVERESTORE_CLIENT_NOTP (1<<0)
#define	FL_SAVERESTORE_NPC_NOTP (1<<1)
#define	FL_SAVERESTORE_NPC_NORESTORE (1<<2)

extern void SaveClientState( CBasePlayer *pPlayer, int pFlags = NULL );

void ism::TpAllPlayer( const Vector newPosition, const QAngle newAngles  )
{
	SaveClientState( NULL, FL_SAVERESTORE_CLIENT_NOTP );
	for( int i=1; i<=gpGlobals->maxClients; i++ )
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );
		if( pPlayer && pPlayer->IsConnected() && pPlayer->IsAlive() ) {
			if( pPlayer->IsInAVehicle() )
				pPlayer->LeaveVehicle();

			pPlayer->SetAbsOrigin( newPosition );
			pPlayer->SetAbsAngles( newAngles );
			pPlayer->Teleport( &newPosition, &newAngles, NULL );
		}
	}
	engine->ServerCommand( "sm_savetp_clearpoint\n");
}
