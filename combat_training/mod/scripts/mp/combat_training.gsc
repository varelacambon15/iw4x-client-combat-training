/*
	Combat Training: the online multiplayer of Modern Warfare 2, offline, against bots.

	Started from the main menu (COMBAT TRAINING), which runs `ct_start <playlist>` in the IW4x client. The client
	sets the match up as an online, ranked game and picks the playlist's maps and modes; this script does the rest:
	  - your experience, rank, unlocks and challenges count, as in a matchmaking game;
	  - the match fills with bots of a level close to yours, as matchmaking would put you with;
	  - players join over the first minutes, and now and then one leaves and another takes the place;
	  - the bots' skill follows how you play: win easily and they get better, struggle and they ease off;
	  - at the end you see what is next, and the next match starts by itself.

	The bots themselves are Bot Warfare's (maps/mp/bots, scripts/mp/bots*.gsc in this mod).
*/

#include maps\mp\_utility;
#include maps\mp\gametypes\_hud_util;
#include common_scripts\utility;

init()
{
	// Only in a Combat Training match: not in a private match lobby, not on servers
	if ( getdvarint( "ct_active" ) != 1 || getdvarint( "party_host" ) == 1 )
		return;

	level.ct = true;
	level.ctPlayers = getdvarint( "ct_players" );
	if ( level.ctPlayers < 2 )
		level.ctPlayers = 12;

	ct_load_mmr();

	// Bot Warfare: no host menu, bots counted together with you, teams kept even
	setdvar( "bots_main_menu", 0 );
	setdvar( "bots_main_firstIsHost", 1 );
	setdvar( "bots_main_waitForHostTime", 3 );
	setdvar( "bots_manage_fill_mode", 0 );
	setdvar( "bots_manage_fill_kick", 1 );
	setdvar( "bots_manage_fill_spec", 1 );
	setdvar( "bots_team", "autoassign" );
	setdvar( "bots_team_force", 1 );
	setdvar( "bots_skill", 0 );

	// Who joins first: most of the lobby, the rest over the first minutes (ct_matchmaking)
	setdvar( "bots_manage_fill", int( level.ctPlayers * randomfloatrange( 0.6, 0.85 ) ) );

	level thread ct_on_player_connect();
	level thread ct_matchmaking();
	level thread ct_end_of_match();
}

/* ------------------------------------------------------------------------------------------------------------ */
/* You                                                                                                            */
/* ------------------------------------------------------------------------------------------------------------ */

ct_on_player_connect()
{
	for ( ;; )
	{
		level waittill( "connected", player );

		if ( player ct_is_bot() )
			continue;

		player thread ct_player();
	}
}

ct_player()
{
	self endon( "disconnect" );

	// The stock scripts ask the engine whether this is "online data used offline" right after the notify this
	// thread came from, and give no experience if so. A local Combat Training match counts as online.
	waittillframeend;
	self.usingOnlineDataOffline = false;

	// The bots' skill, from your rank and prestige and how the last matches went
	ct_set_bot_skill( self maps\mp\gametypes\_rank::getRank(), self maps\mp\gametypes\_rank::getPrestigeLevel() );

	self waittill( "spawned_player" );
	wait 1.5;
	self iprintlnbold( "^3COMBAT TRAINING^7 - " + getdvar( "ct_playlist_name" ) );
}

ct_is_bot()
{
	if ( isdefined( self.pers ) && isdefined( self.pers[ "isBot" ] ) && self.pers[ "isBot" ] )
		return true;

	return self istestclient();
}

/* ------------------------------------------------------------------------------------------------------------ */
/* Skill                                                                                                          */
/* ------------------------------------------------------------------------------------------------------------ */

// Bot Warfare's skill levels go from 1 (easy) to 7 (hard). Your rank (0-69 internally) and prestige set the band,
// the rating from your recent matches (-2 to 2) moves it.
ct_set_bot_skill( rank, prestige )
{
	if ( prestige >= 5 )
	{
		low = 4;
		high = 6;
	}
	else if ( prestige >= 1 || rank >= 49 )
	{
		low = 3;
		high = 6;
	}
	else if ( rank >= 29 )
	{
		low = 3;
		high = 5;
	}
	else if ( rank >= 9 )
	{
		low = 2;
		high = 4;
	}
	else
	{
		low = 1;
		high = 3;
	}

	shift = int( level.ctMmr );
	low = ct_clamp( low + shift, 1, 7 );
	high = ct_clamp( high + shift, low, 7 );

	setdvar( "bots_skill_min", low );
	setdvar( "bots_skill_max", high );
}

ct_clamp( value, low, high )
{
	if ( value < low )
		return low;
	if ( value > high )
		return high;
	return value;
}

// The rating is kept between sessions in scriptdata/ct_mmr.txt
ct_load_mmr()
{
	level.ctMmr = 0;

	if ( getdvar( "ct_mmr" ) != "" )
	{
		level.ctMmr = getdvarfloat( "ct_mmr" );
		return;
	}

	if ( fileexists( "ct_mmr.txt" ) )
	{
		text = fileread( "ct_mmr.txt" );
		if ( isdefined( text ) && text != "" )
			level.ctMmr = float( text );
	}

	setdvar( "ct_mmr", level.ctMmr );
}

ct_save_mmr()
{
	level.ctMmr = ct_clamp( level.ctMmr, -2, 2 );
	setdvar( "ct_mmr", level.ctMmr );
	filewrite( "ct_mmr.txt", "" + level.ctMmr, "write" );
}

// After a match: where you finished on the scoreboard and your kills per death
ct_rate_match( player )
{
	if ( !isdefined( player ) || !isdefined( player.pers ) )
		return;

	kills = player.pers[ "kills" ];
	deaths = player.pers[ "deaths" ];
	if ( !isdefined( kills ) || !isdefined( deaths ) )
		return;

	if ( deaths < 1 )
		deaths = 1;

	ratio = kills / deaths;

	// Your place among everyone who played
	better = 0;
	total = 0;
	foreach ( other in level.players )
	{
		if ( !isdefined( other.pers ) || !isdefined( other.pers[ "score" ] ) )
			continue;

		total++;
		if ( other != player && other.pers[ "score" ] > player.pers[ "score" ] )
			better++;
	}

	if ( total < 2 )
		return;

	place = better / ( total - 1 ); // 0: first, 1: last

	if ( place <= 0.15 && ratio >= 1.8 )
		level.ctMmr += 0.5;
	else if ( place <= 0.35 && ratio >= 1.2 )
		level.ctMmr += 0.25;
	else if ( place >= 0.75 && ratio < 0.8 )
		level.ctMmr -= 0.5;
	else if ( place >= 0.5 && ratio < 1.0 )
		level.ctMmr -= 0.25;

	ct_save_mmr();
}

/* ------------------------------------------------------------------------------------------------------------ */
/* Players coming and going                                                                                       */
/* ------------------------------------------------------------------------------------------------------------ */

ct_matchmaking()
{
	level endon( "game_ended" );

	// The rest of the lobby joins during the first minutes
	while ( getdvarint( "bots_manage_fill" ) < level.ctPlayers )
	{
		wait randomfloatrange( 4, 18 );
		setdvar( "bots_manage_fill", getdvarint( "bots_manage_fill" ) + 1 );
	}

	// Then, now and then, someone leaves and someone else joins in their place
	for ( ;; )
	{
		wait randomfloatrange( 90, 240 );

		if ( randomint( 100 ) >= 35 )
			continue;

		bot = ct_bot_to_leave();
		if ( !isdefined( bot ) )
			continue;

		kick( bot getentitynumber(), "EXE_PLAYERKICKED" );

		// Their place is open for a while, then Bot Warfare fills it with a new name
		setdvar( "bots_manage_fill", level.ctPlayers - 1 );
		wait randomfloatrange( 8, 30 );
		setdvar( "bots_manage_fill", level.ctPlayers );
	}
}

// Someone from the bottom half of the scoreboard: the ones who do well rarely quit
ct_bot_to_leave()
{
	bots = [];
	foreach ( player in level.players )
	{
		if ( player ct_is_bot() && isdefined( player.pers[ "score" ] ) )
			bots[ bots.size ] = player;
	}

	if ( bots.size < 2 )
		return undefined;

	// The lowest scores first
	for ( i = 0; i < bots.size; i++ )
	{
		for ( j = i + 1; j < bots.size; j++ )
		{
			if ( bots[ j ].pers[ "score" ] < bots[ i ].pers[ "score" ] )
			{
				temp = bots[ i ];
				bots[ i ] = bots[ j ];
				bots[ j ] = temp;
			}
		}
	}

	return bots[ randomint( int( ( bots.size + 1 ) / 2 ) ) ];
}

/* ------------------------------------------------------------------------------------------------------------ */
/* End of the match                                                                                               */
/* ------------------------------------------------------------------------------------------------------------ */

ct_end_of_match()
{
	level waittill( "game_ended" );

	foreach ( player in level.players )
	{
		if ( !player ct_is_bot() )
			ct_rate_match( player );
	}

	gametype = getdvar( "ct_nextgametype" );
	map = getdvar( "ct_nextmap" );
	if ( gametype == "" || map == "" )
		return;

	wait 2;

	next = createserverfontstring( "objective", 1.4 );
	next setpoint( "TOP", "TOP", 0, 40 );
	next.sort = 1001;
	next.foreground = true;
	next.hidewheninmenu = false;
	next settext( "Siguiente partida: " + ct_gametype_name( gametype ) + " en " + ct_map_name( map ) );
}

ct_gametype_name( gametype )
{
	switch ( gametype )
	{
		case "war": return "Duelo por equipos";
		case "dm": return "Todos contra todos";
		case "dom": return "Dominio";
		case "koth": return "Cuartel general";
		case "sab": return "Sabotaje";
		case "sd": return "Buscar y destruir";
		case "dd": return "Demolicion";
		case "ctf": return "Captura la bandera";
		case "gtnw": return "Guerra termonuclear";
		case "oneflag": return "Una bandera";
		case "arena": return "Arena";
		case "vip": return "VIP";
	}

	return gametype;
}

ct_map_name( map )
{
	switch ( map )
	{
		case "mp_afghan": return "Afghan";
		case "mp_derail": return "Derail";
		case "mp_estate": return "Estate";
		case "mp_favela": return "Favela";
		case "mp_highrise": return "Highrise";
		case "mp_invasion": return "Invasion";
		case "mp_checkpoint": return "Karachi";
		case "mp_quarry": return "Quarry";
		case "mp_rundown": return "Rundown";
		case "mp_rust": return "Rust";
		case "mp_boneyard": return "Scrapyard";
		case "mp_nightshift": return "Skidrow";
		case "mp_subbase": return "Sub Base";
		case "mp_terminal": return "Terminal";
		case "mp_underpass": return "Underpass";
		case "mp_brecourt": return "Wasteland";
		case "mp_complex": return "Bailout";
		case "mp_compact": return "Salvage";
		case "mp_storm": return "Storm";
		case "mp_overgrown": return "Overgrown";
		case "mp_crash": return "Crash";
		case "mp_abandon": return "Carnival";
		case "mp_fuel2": return "Fuel";
		case "mp_strike": return "Strike";
		case "mp_trailerpark": return "Trailer Park";
		case "mp_vacant": return "Vacant";
	}

	return map;
}
