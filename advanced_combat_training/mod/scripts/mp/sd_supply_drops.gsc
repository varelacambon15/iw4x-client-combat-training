/*
	Advanced Combat Training: Supply Drops, in the match.

	The drops, the Armory, the Store and the Contracts are in the main menu (SUPPLY DROPS), and the variant of each
	weapon is also chosen in Create a Class. This script applies the equipped variants in the match and gives what is
	earned by playing: Supply Drops for time played, levels, prestige, the first win of the day and contracts, and
	Credits at the end of each match. The save file and the settings are handled by the game (SupplyDrops.cpp): the
	SD* functions. The settings are in supply_drops/config.txt.

	What a variant can change: damage (all, close range, long range), headshot damage, mobility and reserve ammo.
*/

#include maps\mp\_utility;
#include maps\mp\gametypes\_hud_util;
#include common_scripts\utility;

init()
{
	if ( getdvar( "sd_disable" ) == "1" || getdvarint( "ct_active" ) != 1 )
		return;

	level thread sd_hook_callbacks();
	level thread sd_on_player_connect();
	level thread sd_end_of_match();
}

// "m4_reflex_silencer_mp" -> "m4"; "" for the underbarrel weapons
sd_base_of( weapon )
{
	if ( !isdefined( weapon ) )
		return "";

	parts = strtok( weapon, "_" );
	if ( parts.size < 2 || parts[ 0 ] == "gl" || parts[ 0 ] == "alt" )
		return "";

	return parts[ 0 ];
}

sd_mod( weapon, type )
{
	base = sd_base_of( weapon );
	if ( base == "" )
		return 0;
	return sdmod( base, type );
}

/* ------------------------------------------------------------------------------------------------------------ */
/* Effects in the match                                                                                           */
/* ------------------------------------------------------------------------------------------------------------ */

sd_hook_callbacks()
{
	// The stock callbacks are in place once the game type has started
	while ( !isdefined( level.callbackPlayerDamage ) || !isdefined( level.callbackPlayerKilled ) )
		wait 0.05;

	level.sdDamage = level.callbackPlayerDamage;
	level.sdKilled = level.callbackPlayerKilled;
	level.callbackPlayerDamage = ::sd_player_damage;
	level.callbackPlayerKilled = ::sd_player_killed;
}

sd_player_damage( eInflictor, eAttacker, iDamage, iDFlags, sMeansOfDeath, sWeapon, vPoint, vDir, sHitLoc, psOffsetTime )
{
	if ( isdefined( eAttacker ) && isplayer( eAttacker ) && eAttacker != self && isdefined( sMeansOfDeath ) && sMeansOfDeath != "MOD_MELEE" )
	{
		base = sd_base_of( sWeapon );
		if ( base != "" && isdefined( eAttacker.sdHuman ) )
		{
			percent = sdmod( base, "dmg" );

			dist = distance( eAttacker.origin, self.origin );
			if ( dist < 750 )
				percent += sdmod( base, "cqb" );
			else if ( dist > 1500 )
				percent += sdmod( base, "lrg" );

			if ( isdefined( sHitLoc ) && ( sHitLoc == "head" || sHitLoc == "helmet" ) )
				percent += sdmod( base, "hs" );

			if ( percent != 0 )
			{
				iDamage = int( iDamage * ( 100 + percent ) / 100 + 0.5 );
				if ( iDamage < 1 )
					iDamage = 1;
			}
		}
	}

	self [[ level.sdDamage ]]( eInflictor, eAttacker, iDamage, iDFlags, sMeansOfDeath, sWeapon, vPoint, vDir, sHitLoc, psOffsetTime );
}

sd_player_killed( eInflictor, attacker, iDamage, sMeansOfDeath, sWeapon, vDir, sHitLoc, psOffsetTime, deathAnimDuration )
{
	if ( isdefined( attacker ) && isplayer( attacker ) && attacker != self && isdefined( attacker.sdHuman ) )
	{
		attacker sd_progress( "kills", 1 );
		if ( isdefined( sHitLoc ) && ( sHitLoc == "head" || sHitLoc == "helmet" ) )
			attacker sd_progress( "headshots", 1 );
		if ( isdefined( sWeapon ) && sd_base_of( sWeapon ) != "" )
			attacker sd_progress( "kills_" + weaponclass( sWeapon ), 1 );
	}

	self [[ level.sdKilled ]]( eInflictor, attacker, iDamage, sMeansOfDeath, sWeapon, vDir, sHitLoc, psOffsetTime, deathAnimDuration );
}

// After the stock code has set the speed for a weapon: the variant's mobility on top
sd_watch_weapon()
{
	self endon( "disconnect" );

	for ( ;; )
	{
		self waittill_any( "weapon_change", "spawned_player", "giveLoadout" );
		waittillframeend;
		wait 0.05;

		if ( !isalive( self ) )
			continue;

		weapon = self getcurrentweapon();
		base = sd_base_of( weapon );
		if ( base == "" )
			continue;

		name = sdvariantname( base );
		if ( name == "" )
			continue;

		mobility = sdmod( base, "mob" );
		if ( mobility != 0 )
		{
			scaler = 1;
			if ( isdefined( self.moveSpeedScaler ) )
				scaler = self.moveSpeedScaler;
			self setmovespeedscale( sd_class_speed( weapon ) * scaler * ( 100 + mobility ) / 100 );
		}

		if ( !isdefined( self.sdShown ) || self.sdShown != name )
		{
			self.sdShown = name;
			self iprintln( name );
		}
	}
}

// The speed _weapons.gsc gives each class of weapon
sd_class_speed( weapon )
{
	switch ( weaponclass( weapon ) )
	{
		case "rifle": return 0.95;
		case "mg": return 0.875;
		case "spread": return 0.95;
		case "rocketlauncher": return 0.8;
	}
	return 1.0;
}

// Reserve ammo, once per life, when the class is given
sd_watch_loadout()
{
	self endon( "disconnect" );

	for ( ;; )
	{
		self waittill( "giveLoadout" );
		waittillframeend;

		foreach ( weapon in self getweaponslistprimaries() )
		{
			ammo = sd_mod( weapon, "ammo" );
			if ( ammo == 0 )
				continue;

			stock = self getweaponammostock( weapon );
			stock = int( stock * ( 100 + ammo ) / 100 + 0.5 );
			if ( stock > weaponmaxammo( weapon ) )
				stock = weaponmaxammo( weapon );
			self setweaponammostock( weapon, stock );
		}
	}
}

/* ------------------------------------------------------------------------------------------------------------ */
/* Earning                                                                                                        */
/* ------------------------------------------------------------------------------------------------------------ */

sd_on_player_connect()
{
	for ( ;; )
	{
		level waittill( "connected", player );

		if ( player sd_is_bot() )
			continue;

		player thread sd_player();
	}
}

sd_is_bot()
{
	if ( isdefined( self.pers ) && isdefined( self.pers[ "isBot" ] ) && self.pers[ "isBot" ] )
		return true;
	return self istestclient();
}

sd_player()
{
	self endon( "disconnect" );

	self.sdHuman = true;
	self.sdLastPrestige = self maps\mp\gametypes\_rank::getPrestigeLevel();

	self thread sd_watch_weapon();
	self thread sd_watch_loadout();
	self thread sd_watch_playtime();
	self thread sd_watch_rank();

	self waittill( "spawned_player" );
	wait 4;
	total = sdcount();
	if ( total > 0 )
		self iprintln( "^3Supply Drops:^7 " + total + " waiting in the main menu" );
}

sd_watch_playtime()
{
	self endon( "disconnect" );
	level endon( "game_ended" );

	for ( ;; )
	{
		wait 60;
		if ( !isdefined( self.pers[ "team" ] ) || self.pers[ "team" ] == "spectator" )
			continue;

		if ( sdminuteplayed() )
			self sd_announce( "Time played", "+1 Supply Drop" );
	}
}

sd_watch_rank()
{
	self endon( "disconnect" );

	for ( ;; )
	{
		self waittill( "update_rank" );

		prestige = self maps\mp\gametypes\_rank::getPrestigeLevel();
		if ( prestige > self.sdLastPrestige )
		{
			self.sdLastPrestige = prestige;
			self sd_announce( "Prestige", sdaward( 0, sdconfig( "advanced_per_prestige" ) ) );
		}
		else
		{
			self sd_announce( "Level up", sdaward( sdconfig( "drops_per_level" ), 0 ) );
		}
	}
}

sd_progress( type, amount )
{
	text = sdprogress( type, amount );
	if ( text != "" )
	{
		foreach ( line in strtok( text, "\n" ) )
			self sd_announce( "", line );
	}
}

sd_announce( reason, text )
{
	if ( text == "" )
		return;

	if ( reason != "" )
		text = "^3" + reason + ":^7 " + text;
	else
		text = "^3" + text;
	self iprintlnbold( text );
	self playlocalsound( "mp_level_up" );
}

sd_end_of_match()
{
	level waittill( "game_ended", winner );

	foreach ( player in level.players )
	{
		if ( !isdefined( player.sdHuman ) )
			continue;

		player sd_progress( "matches", 1 );

		won = false;
		if ( level.teamBased )
			won = isdefined( winner ) && isdefined( player.pers[ "team" ] ) && winner == player.pers[ "team" ];
		else
			won = isdefined( winner ) && winner == player;

		if ( won )
		{
			player sd_progress( "wins", 1 );
			drops = sdfirstwin();
			if ( drops > 0 )
				player sd_announce( "First win of the day", "+" + drops + " Supply Drop" );
		}

		credits = sd_match_credits( sd_place( player ) );
		sdaddcredits( credits );
		player iprintln( "^3+" + credits + " Credits" );
	}
}

sd_match_credits( place )
{
	if ( place <= 0.15 )
		return sdconfig( "match_credits_top" );
	if ( place <= 0.5 )
		return sdconfig( "match_credits_middle" );
	return sdconfig( "match_credits_bottom" );
}

// 0 = top of the scoreboard, 1 = bottom
sd_place( player )
{
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
		return 0;
	return better / ( total - 1 );
}
