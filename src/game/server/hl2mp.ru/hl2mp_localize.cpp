#include "cbase.h"
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern void Hack_FixEscapeChars( char *str );

class CHL2MP_Localize {
public:
	CHL2MP_Localize() {
		main = new KeyValues( "Translator" );
		//Msg("[CHL2MP_Localize] Class init\n");
	}

	bool AddFile( const char *szFileName ) {
		KeyValues *manifest = new KeyValues( "Translator" );
		if( !manifest->LoadFromFile( filesystem, szFileName ) )
			return false;

		//Msg("[CHL2MP_Localize] Added file %s\n", szFileName);
		main->RecursiveMergeKeyValues( manifest );
		manifest->deleteThis();
		return true;
	}

	const char *TranslateMsg( const char *szLanguage, char const *msg ) {
		KeyValues *pKV = main->FindKey( msg );
		if( pKV != NULL ) {
			msg = pKV->GetString( szLanguage );
			if( !Q_stricmp( msg,  "" ) )
				msg = pKV->GetString( "english", msg );
		}
		
		char *nmsg = strdup( msg );
		Hack_FixEscapeChars( nmsg );
		return nmsg;
	}

	void Clear( void ) {
		//Msg("[CHL2MP_Localize] Cleared cache\n");
		curentlevel = NULL;
		main->Clear();
	}

	char *curentlevel;
	KeyValues *main;
};

static CHL2MP_Localize g_Localize;

const char *UTIL_TranslateMsg( const char *szLanguage, const char *msg ) {
	if( g_Localize.curentlevel != NULL && ( Q_stricmp( g_Localize.curentlevel, STRING( gpGlobals->mapname ) ) != 0 ) )
		g_Localize.Clear();

	if( g_Localize.curentlevel == NULL ) {
		g_Localize.curentlevel = strdup( STRING( gpGlobals->mapname ) );
		g_Localize.AddFile( "translate.txt" );
		g_Localize.AddFile( UTIL_VarArgs("maps/translate/%s.txt", STRING( gpGlobals->mapname ) ) );
	}
	return g_Localize.TranslateMsg( szLanguage, msg );
}