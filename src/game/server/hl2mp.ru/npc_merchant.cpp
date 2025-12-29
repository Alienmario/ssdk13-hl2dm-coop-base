#include "cbase.h"
#include "Sprite.h"
#include "npc_playercompanion.h"
#include "viewport_panel_names.h"
#include "te_effect_dispatch.h"
#include "hl2mp_player_fix.h"
#include "eventqueue.h"
#include "items.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CNPC_merchant : public CNPC_PlayerCompanion
{
public:
	DECLARE_CLASS( CNPC_merchant, CNPC_PlayerCompanion );

	void Spawn( void );
	void SelectModel();
	void Precache( void );
	bool IgnorePlayerPushing( void ) { return true; };
	Class_T Classify ( void );
	bool ShouldLookForBetterWeapon() { return false; } 
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	CHandle<CSprite> m_hSprite;
};

LINK_ENTITY_TO_CLASS( npc_merchant, CNPC_merchant );

Class_T	CNPC_merchant::Classify ( void )
{
	if( GetActiveWeapon() )
		return CLASS_PLAYER_ALLY;

	return	CLASS_NONE;
}

ConVar sk_merchant_health("sk_merchant_health","100");

//=========================================================
// Spawn
//=========================================================
void CNPC_merchant::Spawn()
{
	BaseClass::Spawn();
	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE );
	SetMoveType( MOVETYPE_STEP );
	SetUse( &CNPC_merchant::Use );
	
	CapabilitiesAdd( bits_CAP_MOVE_GROUND | bits_CAP_DOORS_GROUP | bits_CAP_TURN_HEAD | bits_CAP_DUCK | bits_CAP_SQUAD );
	CapabilitiesAdd( bits_CAP_USE_WEAPONS );
	CapabilitiesAdd( bits_CAP_ANIMATEDFACE );
	CapabilitiesAdd( bits_CAP_FRIENDLY_DMG_IMMUNE );
	CapabilitiesAdd( bits_CAP_AIM_GUN );
	CapabilitiesAdd( bits_CAP_MOVE_SHOOT );
	CapabilitiesAdd( bits_CAP_USE_SHOT_REGULATOR );

	m_iHealth = sk_merchant_health.GetInt();

	NPCInit();
	Vector origin = EyePosition();
	origin.z += 12.0;
	m_hSprite = CSprite::SpriteCreate( "hl2mp.ru/merchant.vmt", origin, false );
	if ( m_hSprite ) {
		m_hSprite->SetRenderMode( kRenderGlow );
		m_hSprite->SetScale( 0.2f );
		m_hSprite->SetGlowProxySize( 64.0f );
		m_hSprite->SetParent( this );
		m_hSprite->TurnOn();
	}

	int iAttachment = LookupBone( "ValveBiped.Bip01_R_Hand" );
	if( GetActiveWeapon() && iAttachment > 0 ) {
		SetCollisionGroup( COLLISION_GROUP_PLAYER );
	}
	else {
		if( GetActiveWeapon() )
			Weapon_Drop( GetActiveWeapon() );

		AddEFlags( EFL_NO_DISSOLVE );
		m_takedamage = DAMAGE_NO;
		AddSpawnFlags( SF_NPC_WAIT_FOR_SCRIPT );
		CapabilitiesClear();
	}
	UTIL_SayTextAll("#HL2MP_MerchantOnMap");
}

void CNPC_merchant::Precache()
{
	BaseClass::Precache();
	PrecacheModel( STRING( GetModelName() ) );
	PrecacheParticleSystem("Weapon_Combine_Ion_Cannon_Explosion");
	PrecacheParticleSystem("advisor_object_charge");
}

void CNPC_merchant::SelectModel()
{
	const char *szModel = STRING( GetModelName() );
	if (!szModel || !*szModel)
		SetModelName( AllocPooledString("models/zombie/classic.mdl") );
}

ConVar sv_merchant_url("sv_merchant_url","http://hl2mp.ru/motd/buy/27400/new/");
ConVar sv_merchant_url_hide("sv_merchant_url_hide","0");
ConVar sv_merchant_url_unload("sv_merchant_url_unload","1");

bool firstload[64];
bool inShop[64];
bool inInventory[64];

class testTimer : public CBasePlayer
{
	DECLARE_CLASS( testTimer, CBaseEntity );
	DECLARE_DATADESC();
public:
	void ContextThink( void );
};

BEGIN_DATADESC( testTimer )
	DEFINE_THINKFUNC( ContextThink ),
END_DATADESC()

void testTimer::ContextThink()
{
	if( inShop[entindex()] || inInventory[entindex()] ) {
		AddFlag( FL_FROZEN );
		AddFlag( FL_NOTARGET );
		AddFlag( FL_GODMODE );
		SetNextThink(gpGlobals->curtime + .2, "MerchantContext" );
	}
	else {
		RemoveFlag( FL_FROZEN );
		RemoveFlag( FL_NOTARGET );
		RemoveFlag( FL_GODMODE );
	}
}

extern const char *UTIL_TranslateMsg( const char *szLanguage, const char *msg );

void openshop( CBasePlayer *player )
{
	int pIndex = engine->IndexOfEdict( player->edict() );
	const char *szLanguage = engine->GetClientConVarValue( pIndex, "cl_language" );
	firstload[pIndex] = true;
	inInventory[pIndex] = false;
	inShop[pIndex] = true;
	char msg[254];
	Q_snprintf( msg, sizeof(msg), "%s/index.php?key=%d&userid=%d&language=%s", sv_merchant_url.GetString(), (int)player, player->GetUserID(), szLanguage );
	KeyValues *data = new KeyValues("data");
	const char* title = UTIL_TranslateMsg( engine->GetClientConVarValue( pIndex, "cl_language" ), "#HL2MP_MerchantTitle" );
	data->SetString( "title", title );		// info panel title
	data->SetString( "type", "2" );			// show userdata from stringtable entry
	data->SetString( "msg",	msg );		// use this stringtable entry
	data->SetString("cmd", "5" );
	data->SetBool( "unload", sv_merchant_url_unload.GetBool() );
	player->ShowViewPortPanel( PANEL_INFO, !sv_merchant_url_hide.GetBool(), data );
	data->deleteThis();
}

void CC_closed_htmlpage( const CCommand &args )
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	int pIndex = engine->IndexOfEdict( pPlayer->edict() );
	inShop[pIndex] = false;
	inInventory[pIndex] = false;
}

static ConCommand closed_htmlpage( "closed_htmlpage", CC_closed_htmlpage, "", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_SERVER_CAN_EXECUTE );

void CNPC_merchant::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CBasePlayer *pPlayer = ToBasePlayer( pActivator );
	if( pPlayer )
		openshop( pPlayer );
}

void CC_openshop( const CCommand& args )
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if( pPlayer && IsAdmin( pPlayer ) )
		openshop( pPlayer );
}

static ConCommand merchant("merchant", CC_openshop, "Force open merchant menu!", FCVAR_GAMEDLL );

void CC_sv_merchant_give( const CCommand &args )
{
	if ( !UTIL_IsCommandIssuedByServerAdmin() )
		return;

	const char *modify = args.Arg(1)+1;
	int userid = atoi(modify);
	CBasePlayer *player = UTIL_PlayerByUserId( userid );
	if( !player )
		return;
	
	const char *classname = args.Arg(2);
	if ( Q_stristr(classname, "weapon_" ) ) {
		CBaseEntity *item = player->GiveNamedItem( classname );
		if( !item )
			Msg( "{\"Merchant\":false}\n" );
		else {
			item->SetPlayerMP( player );
			item->AddSpawnFlags( SF_NPC_NO_WEAPON_DROP );
			for (int i=0; i<MAX_WEAPONS; ++i) 
			{
				CBaseCombatWeapon *pWeapon = player->GetWeapon(i);
				if (!pWeapon)
					continue;

				bool pEmpty = !pWeapon->HasAmmo();
				if( !pEmpty && FClassnameIs(pWeapon, classname) ) {
					player->Weapon_Switch(pWeapon);
					break;
				}
			}
		}
	}
	else {
		bool result = false;
		int count = atoi(args.Arg(3));
		do {
			CItem *pItem = dynamic_cast<CItem *>( CreateEntityByName(classname) );
			if( pItem ) {
				pItem->AddSpawnFlags( SF_NORESPAWN );
				if( !pItem->MyTouch( player ) ) {
					pItem->Remove();
					break;
				}
				result = true;
			}
		}
		while(count--);
		if( !result )
			Msg( "{\"Merchant\":false}\n" );
	}
}

static ConCommand sv_merchant_give( "sv_merchant_give", CC_sv_merchant_give, "", FCVAR_GAMEDLL );

void CC_sv_merchant_client_info( const CCommand &args )
{
	if ( !UTIL_IsCommandIssuedByServerAdmin() )
		return;

	if( strcmpi(args.Arg(1),"@all" ) == 0 ) {
		for( int i=1; i<=gpGlobals->maxClients; i++ )
		{
			CBasePlayer *player = UTIL_PlayerByIndex( i );
			if( player && !player->IsFakeClient() ) {
				CSteamID steamID;
				player->GetSteamID( &steamID );
				const char *szLanguage = engine->GetClientConVarValue( engine->IndexOfEdict( player->edict() ), "cl_language" );
				Msg( "{\"pUserId\": %d, \"pSteamID64\": \"%llu\", \"pLanguage\": \"%s\"}\n", player->GetUserID(), steamID.ConvertToUint64(), szLanguage );
			}
		}
		return;
	}

	int userid = atoi( args.Arg( 1 ) );
	CBasePlayer *player = UTIL_PlayerByUserId( userid );
	if( !player )
		return;

	int pIndex = engine->IndexOfEdict( player->edict() );

	if( strcmpi(args.Arg(2),"leaveshop" ) == 0 ) {
		inShop[pIndex] = false;
		inInventory[pIndex] = false;
		return;
	}
	
	if( strcmpi(args.Arg(2),"inmotd" ) == 0 ) {
		Msg( "{\"pInShop\":%s, \"pInInventory\":%s, \"pFirstLoad\":%s}\n", inShop[pIndex] ? "true" : "false", inInventory[pIndex] ? "true" : "false", firstload[pIndex] ? "true" : "false" );
		return;
	}

	if( strcmpi(args.Arg(2),"firstload" ) == 0 ) {
		Msg( "{\"Merchant\":%s}\n", firstload[pIndex] ? "true" : "false" );
		if( firstload[pIndex] && inShop[pIndex] ) {
			player->RegisterThinkContext( "MerchantContext" );
			player->SetContextThink( &testTimer::ContextThink, gpGlobals->curtime, "MerchantContext" );
		}
		firstload[pIndex] = false;
		return;
	}

	CSteamID steamID;
	player->GetSteamID( &steamID );
	Msg( "{\"pFrag\": %d, \"pAdmin\": %d, \"pKey\": %d, \"pTeam\": %d, \"pSteamID64\": \"%llu\"}\n", player->FragCount(), IsAdmin( player ), (int)player, player->GetTeamNumber(), steamID.ConvertToUint64() );
}

static ConCommand sv_merchant_client_info( "sv_merchant_client_info", CC_sv_merchant_client_info, "", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_SERVER_CAN_EXECUTE );

void CC_sv_merchant_decrement_frag( const CCommand &args )
{
	if ( !UTIL_IsCommandIssuedByServerAdmin() )
		return;

	int frag = atoi(args.Arg(2));
	int userid = atoi(args.Arg(1));
	CBasePlayer *player = UTIL_PlayerByUserId( userid );
	if(player)
		player->IncrementFragCount(-frag);
}

static ConCommand sv_merchant_decrement_frag( "sv_merchant_decrement_frag", CC_sv_merchant_decrement_frag, "", FCVAR_GAMEDLL );

void CC_sv_merchant_entity_kill( const CCommand &args )
{
	if ( !UTIL_IsCommandIssuedByServerAdmin() )
		return;

	bool result = false;
	int entindex = atoi( args.Arg( 1 ) );
	if( entindex != 0 ) {
		CBaseEntity *pEntity = UTIL_EntityByIndex( entindex );
		if( pEntity ) {
			pEntity->SetPlayerMP( NULL );
			pEntity->SetSectionName( NULL );

			if( Q_stristr(pEntity->GetClassname(), "weapon_" ) ) {
				CBaseCombatWeapon *pWeapon = dynamic_cast<CBaseCombatWeapon*>( pEntity );
				if( pWeapon ) {
					pWeapon->AddSpawnFlags( SF_NPC_NO_WEAPON_DROP );
					CBaseCombatCharacter *pOwner = pWeapon->GetOwner();
					if( pOwner ) {
						pOwner->Weapon_Drop( pWeapon );
						result = true;
					}
				}
			}

			CBaseCombatCharacter *pNpc = dynamic_cast<CBaseCombatCharacter*>( pEntity );
			if( pNpc ) {
				CBaseCombatWeapon *pWeapon = dynamic_cast<CBaseCombatWeapon*>( pNpc->GetActiveWeapon() );
				if( pWeapon )
					pWeapon->AddSpawnFlags( SF_NPC_NO_WEAPON_DROP );
			}

			if( !result ) {
				pEntity->TakeDamage( CTakeDamageInfo( pEntity, pEntity, pEntity->GetHealth()+100, DMG_GENERIC ) );

				CBaseAnimating *pDisolvingAnimating = dynamic_cast<CBaseAnimating*>( pEntity );
				if ( pDisolvingAnimating )
					pDisolvingAnimating->Dissolve( "", gpGlobals->curtime, false, ENTITY_DISSOLVE_NORMAL );
				else
					g_EventQueue.AddEvent( pEntity, "Kill", 0.0f, NULL, NULL );

				result = true;
			}
		}
	}
	if( result )
		Msg( "{\"Merchant\":true}\n" );
	else
		Msg( "{\"Merchant\":false}\n" );
}

static ConCommand sv_merchant_entity_kill( "sv_merchant_entity_kill", CC_sv_merchant_entity_kill, "", FCVAR_GAMEDLL );

void CC_sv_merchant_give_npc( const CCommand &args )
{
	if ( !UTIL_IsCommandIssuedByServerAdmin() )
		return;

	const char *modify = args.Arg(1)+1;
	int userid = atoi(modify);
	CBasePlayer *player = UTIL_PlayerByUserId( userid );
	if( !player )
		return;

	const char *classname = args.Arg(2);
	int limit = atoi(args.Arg(3));
	const char *weapon = args.Arg(4);
	int health = atoi(args.Arg(5));
	const char *section = args.Arg(6);

	if( !health )
		health = 1000;

	if( classname ) {
		CBaseEntity *ent = NULL;
		int maxnpc = 0;
		while ( (ent = gEntList.NextEnt(ent)) != NULL )
		{
			if( ent->GetSectionName() != NULL && FStrEq( section, ent->GetSectionName() ) )
			{
				if (ent->GetPlayerMP() == player)
					maxnpc++;

				if(maxnpc >= limit && limit != 0) {
					Msg( "{\"Merchant\":false}\n" );
					return;
				}
			}
		}

		CBaseEntity *entity = CreateEntityByName( classname );
		if ( entity != NULL )
		{
			entity->SetAbsOrigin( player->GetAbsOrigin() + Vector(0.0, 0.0, 10.0) );
			entity->SetCollisionGroup( COLLISION_GROUP_PLAYER );
			entity->SetPlayerMP( player );
			entity->SetSectionName( section );
			entity->KeyValue( "squadname", "merchant" );
			if( weapon )
				entity->KeyValue( "additionalequipment", weapon );

			DispatchSpawn( entity );
			entity->Activate();
			DispatchParticleEffect( "Weapon_Combine_Ion_Cannon_Explosion", entity->GetAbsOrigin()+Vector(0.0, 0.0, 10.0), vec3_angle );
			DispatchParticleEffect( "advisor_object_charge", entity->GetAbsOrigin()+Vector(0.0, 0.0, 32.0), vec3_angle );

			if( entity->IsNPC() ) {
				INPCInteractive *pInteractive = dynamic_cast<INPCInteractive *>( entity );
				if( pInteractive )
					pInteractive->NotifyInteraction( NULL );

				entity->SetHealth( health );
				entity->SetMaxHealth( health );
				entity->m_takedamage = DAMAGE_YES;
				entity->AddEFlags( EFL_NO_DISSOLVE );
				entity->AddSpawnFlags( SF_NPC_FADE_CORPSE );
			}
			Msg( "{\"Merchant\":true}\n" );
		}
		else
			Msg( "{\"Merchant\":false}\n" );
	}
}

static ConCommand sv_merchant_give_npc( "sv_merchant_give_npc", CC_sv_merchant_give_npc, "", FCVAR_GAMEDLL );

inline bool TeleportAll( CBasePlayer *pPlayer ) {
	bool ready = false;
	CBaseEntity *ent = NULL;
	while ( (ent = gEntList.NextEnt(ent)) != NULL )
	{
		if (ent->GetPlayerMP() && (ent->GetPlayerMP() == pPlayer) && !Q_stristr(ent->GetClassname(), "prop_vehicle" ) && !Q_stristr(ent->GetClassname(), "weapon_" ) ) {
			DispatchParticleEffect( "Weapon_Combine_Ion_Cannon_Explosion", ent->GetAbsOrigin()+Vector(0.0, 0.0, 10.0), vec3_angle );
			DispatchParticleEffect( "advisor_object_charge", ent->GetAbsOrigin()+Vector(0.0, 0.0, 32.0), vec3_angle );
			Vector newOrigin = pPlayer->GetAbsOrigin() + Vector(0.0, 0.0, 8.0);
			ent->Teleport(&newOrigin,&pPlayer->GetAbsAngles(),&vec3_origin);
			ready = true;
		}
	}
	return ready;
}

void CC_sv_merchant_tp_npc( const CCommand &args )
{
	if ( !UTIL_IsCommandIssuedByServerAdmin() )
		return;

	int userid = atoi( args.Arg( 1 ) );
	CBasePlayer *pPlayer = UTIL_PlayerByUserId( userid );
	if( !pPlayer )
		return;

	bool ready = false;
	int entindex = atoi( args.Arg( 2 ) );
	if( entindex != 0 ) {
		CBaseEntity *pEntity = UTIL_EntityByIndex( entindex );
		if( pEntity ) {
			DispatchParticleEffect( "Weapon_Combine_Ion_Cannon_Explosion", pEntity->GetAbsOrigin()+Vector(0.0, 0.0, 10.0), vec3_angle );
			DispatchParticleEffect( "advisor_object_charge", pEntity->GetAbsOrigin()+Vector(0.0, 0.0, 32.0), vec3_angle );
			Vector newOrigin = pPlayer->GetAbsOrigin() + Vector(0.0, 0.0, 8.0);
			pEntity->Teleport(&newOrigin,&pPlayer->GetAbsAngles(),&vec3_origin);
			ready = true;
		}
	}
	else {
		ready = TeleportAll( pPlayer );
	}

	if( !ready ) {
		const char *msg = UTIL_TranslateMsg( engine->GetClientConVarValue( ENTINDEX( pPlayer ), "cl_language" ), "#HL2MP_MerchantTeleportFalse" );
		UTIL_SayText(msg,pPlayer);
		Msg( "{\"Merchant\":false}\n" );
		return;
	}
	DispatchParticleEffect( "Weapon_Combine_Ion_Cannon_Explosion", pPlayer->GetAbsOrigin()+Vector(0.0, 0.0, 10.0), vec3_angle );
	DispatchParticleEffect( "advisor_object_charge", pPlayer->GetAbsOrigin()+Vector(0.0, 0.0, 32.0), vec3_angle );
	const char *msg = UTIL_TranslateMsg( engine->GetClientConVarValue( ENTINDEX( pPlayer ), "cl_language" ), "#HL2MP_MerchantTeleportTrue" );
	UTIL_SayText(msg,pPlayer);
	Msg( "{\"Merchant\":true}\n" );
}

static ConCommand sv_merchant_tp_npc( "sv_merchant_tp_npc", CC_sv_merchant_tp_npc, "", FCVAR_SERVER_CAN_EXECUTE );

void openinventory( CBasePlayer *player )
{
	int pIndex = engine->IndexOfEdict( player->edict() );
	firstload[pIndex] = true;
	inInventory[pIndex] = true;
	inShop[pIndex] = false;
	const char *szLanguage = engine->GetClientConVarValue( engine->IndexOfEdict( player->edict() ), "cl_language" );
	char msg[254];
	Q_snprintf( msg, sizeof(msg), "%s/index.php?inventory=true&key=%d&userid=%d&language=%s", sv_merchant_url.GetString(), (int)player, player->GetUserID(), szLanguage );
	KeyValues *data = new KeyValues("data");
	const char* title = UTIL_TranslateMsg( engine->GetClientConVarValue( pIndex, "cl_language" ), "#HL2MP_MerchantInventoryTitle" );
	data->SetString( "title", title );		// info panel title
	data->SetString( "type", "2" );			// show userdata from stringtable entry
	data->SetString( "msg",	msg );		// use this stringtable entry
	data->SetString("cmd", "5" );
	data->SetBool( "unload", sv_merchant_url_unload.GetBool() );
	player->ShowViewPortPanel( PANEL_INFO, !sv_merchant_url_hide.GetBool(), data );
	data->deleteThis();
}

void CC_sv_merchant_inventory( const CCommand &args )
{
	if( UTIL_GetCommandClient() )
		openinventory( UTIL_GetCommandClient() );

	if ( !UTIL_IsCommandIssuedByServerAdmin() )
		return;

	int userid = atoi( args.Arg( 1 ) );
	CBasePlayer *pPlayer = UTIL_PlayerByUserId( userid );
	if( !pPlayer )
		return;
	
	for (int i=0; i<MAX_WEAPONS; ++i) 
	{
		CBaseCombatWeapon *pWeapon = pPlayer->GetWeapon(i);
		if (!pWeapon)
			continue;

		Msg( "{ \"pClassName\": \"%s\", \"pEntIndex\": %d, \"pUserID\": %d, \"pEmpty\": \"%s\" }\n", pWeapon->GetClassname(), ENTINDEX( pWeapon ), pWeapon->GetPlayerMP() ? pWeapon->GetPlayerMP()->GetUserID() : NULL, !pWeapon->HasAmmo() ? "true" : "false" );
	}

	CBaseEntity *ent = NULL;
	while ( (ent = gEntList.NextEnt(ent)) != NULL )
	{
		if( Q_stristr( ent->GetClassname(), "weapon_" ) )
			continue;

		if (ent->GetPlayerMP() && (ent->GetPlayerMP() == pPlayer) ) {
			float result = ent->GetHealth() / (ent->GetMaxHealth() / 100.0);
			int health = ceil( result );
			Msg( "{ \"pSection\": \"%s\", \"pHealth\": %d, \"pScore\": %d, \"pEntIndex\": %d, \"pUserID\": %d, \"pClassName\": \"%s\" }\n", ent->GetSectionName(), health, ent->GetScore(), ENTINDEX( ent ), ent->GetPlayerMP()->GetUserID(), ent->GetClassname() );
		}
	}
}

static ConCommand sv_merchant_inventory( "sv_merchant_inventory", CC_sv_merchant_inventory, "", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_SERVER_CAN_EXECUTE );

void CC_sv_merchant_equip( const CCommand &args )
{
	if ( !UTIL_IsCommandIssuedByServerAdmin() )
		return;

	int userid = atoi(args.Arg(1));
	CBasePlayer *player = UTIL_PlayerByUserId( userid );
	if( !player || !player->IsAlive() )
		return;

	const char *classname = args.Arg(2);
	
	int bSpawn = atoi( args.Arg(3) );
	bool result = false;
	if( bSpawn > 0 ) {
		CBaseEntity *pWeapon = player->GiveNamedItem(classname);
		if( pWeapon )
			pWeapon->AddSpawnFlags( SF_NPC_NO_WEAPON_DROP );
	}
	for (int i=0; i<MAX_WEAPONS; ++i) 
	{
		CBaseCombatWeapon *pWeapon = player->GetWeapon(i);
		if (!pWeapon)
			continue;

		bool pEmpty = !pWeapon->HasAmmo();
		if( !pEmpty && FClassnameIs(pWeapon, classname) ) {
			player->Weapon_Switch(pWeapon);
			result = true;
			break;
		}
	}
	if( result )
		Msg( "{\"Merchant\":true}\n" );
	else
		Msg( "{\"Merchant\":false}\n" );
}

static ConCommand sv_merchant_equip( "sv_merchant_equip", CC_sv_merchant_equip, "", FCVAR_GAMEDLL );