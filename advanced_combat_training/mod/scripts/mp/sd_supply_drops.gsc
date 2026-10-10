/*
	Advanced Combat Training: Supply Drops.

	Supply Drops are earned by playing (time in matches, levels, prestige, the first win of the day and contracts) and
	bought with Credits. Each drop holds weapon variants: a base weapon of MW2 with a rarity (Enlisted, Professional or
	Elite), a name and random changes to its stats, advantages and sometimes a drawback. Equip a variant and it applies
	whenever you use that weapon, in any class and with any attachments. Variants you don't want are salvaged for
	Credits.

	In a match: Action Slot 2 (D-pad down) opens the Supply Drops menu. Move with forward/back (left stick), select with
	jump (A) and go back with crouch (B). The tunables are in sd_config.gsc.

	What a variant can change: damage (all, close range, long range), headshot damage, mobility and reserve ammo.
	The save file is players/supply_drops/profile.txt in the game folder (SDRead/SDWrite of the Combat Training build).
*/

#include maps\mp\_utility;
#include maps\mp\gametypes\_hud_util;
#include common_scripts\utility;

init()
{
	if ( getdvar( "sd_disable" ) == "1" )
		return;

	sd_tables();
	level thread sd_hook_callbacks();
	level thread sd_on_player_connect();
	level thread sd_end_of_match();
}

/* ------------------------------------------------------------------------------------------------------------ */
/* Tables                                                                                                         */
/* ------------------------------------------------------------------------------------------------------------ */

sd_tables()
{
	level.sdWeapons = [];
	level.sdWeaponName = [];
	sd_weapon( "m4", "M4A1" ); sd_weapon( "famas", "FAMAS" ); sd_weapon( "scar", "SCAR-H" ); sd_weapon( "tavor", "TAR-21" );
	sd_weapon( "fal", "FAL" ); sd_weapon( "m16", "M16A4" ); sd_weapon( "masada", "ACR" ); sd_weapon( "fn2000", "F2000" );
	sd_weapon( "ak47", "AK-47" ); sd_weapon( "mp5k", "MP5K" ); sd_weapon( "ump45", "UMP45" ); sd_weapon( "kriss", "Vector" );
	sd_weapon( "p90", "P90" ); sd_weapon( "uzi", "Mini-Uzi" ); sd_weapon( "sa80", "L86 LSW" ); sd_weapon( "rpd", "RPD" );
	sd_weapon( "mg4", "MG4" ); sd_weapon( "aug", "AUG HBAR" ); sd_weapon( "m240", "M240" ); sd_weapon( "cheytac", "Intervention" );
	sd_weapon( "barrett", "Barrett .50cal" ); sd_weapon( "wa2000", "WA2000" ); sd_weapon( "m21", "M21 EBR" );
	sd_weapon( "spas12", "SPAS-12" ); sd_weapon( "aa12", "AA-12" ); sd_weapon( "striker", "Striker" ); sd_weapon( "ranger", "Ranger" );
	sd_weapon( "m1014", "M1014" ); sd_weapon( "model1887", "Model 1887" ); sd_weapon( "pp2000", "PP2000" ); sd_weapon( "glock", "G18" );
	sd_weapon( "beretta393", "M93 Raffica" ); sd_weapon( "tmp", "TMP" ); sd_weapon( "usp", "USP .45" );
	sd_weapon( "coltanaconda", ".44 Magnum" ); sd_weapon( "beretta", "M9" ); sd_weapon( "deserteagle", "Desert Eagle" );

	level.sdNames = strtok( "Ember,Gale,Fang,Viper,Onyx,Tempest,Rook,Havoc,Specter,Talon,Cinder,Frost,Nomad,Warden,Raptor,"
		+ "Obsidian,Monsoon,Ironclad,Banshee,Sabre,Vandal,Lancer,Harbinger,Mirage,Bulwark,Jackal,Eclipse,Ravager,Sentinel,Thorn,"
		+ "Cobalt,Wraith,Vortex,Kestrel,Bastion,Riptide,Outlaw,Phantom,Granite,Hornet,Requiem,Atlas,Siren,Valkyrie,Grim", "," );

	level.sdModName = [];
	level.sdModName[ "dmg" ] = "Damage";
	level.sdModName[ "cqb" ] = "Close range damage";
	level.sdModName[ "lrg" ] = "Long range damage";
	level.sdModName[ "hs" ] = "Headshot damage";
	level.sdModName[ "mob" ] = "Mobility";
	level.sdModName[ "ammo" ] = "Reserve ammo";
	level.sdModTypes = strtok( "dmg,cqb,lrg,hs,mob,ammo", "," );

	level.sdRarityName = [];
	level.sdRarityName[ "enlisted" ] = "Enlisted";
	level.sdRarityName[ "professional" ] = "Professional";
	level.sdRarityName[ "elite" ] = "Elite";
	level.sdRarityColor = [];
	level.sdRarityColor[ "enlisted" ] = ( 0.55, 0.85, 0.45 );
	level.sdRarityColor[ "professional" ] = ( 0.35, 0.65, 1 );
	level.sdRarityColor[ "elite" ] = ( 1, 0.75, 0.2 );

	level.sdContractTypes = strtok( "kills,headshots,wins,matches,kills_rifle,kills_smg,kills_mg,kills_sniper,kills_spread,kills_pistol", "," );
}

sd_weapon( base, name )
{
	level.sdWeapons[ level.sdWeapons.size ] = base;
	level.sdWeaponName[ base ] = name;
}

// "m4_reflex_silencer_mp" -> "m4"; "" for what is not one of the weapons above (launchers, the underbarrel ones...)
sd_base_of( weapon )
{
	if ( !isdefined( weapon ) )
		return "";

	parts = strtok( weapon, "_" );
	if ( parts.size < 2 || parts[ 0 ] == "gl" || parts[ 0 ] == "alt" )
		return "";

	if ( !isdefined( level.sdWeaponName[ parts[ 0 ] ] ) )
		return "";

	return parts[ 0 ];
}

/* ------------------------------------------------------------------------------------------------------------ */
/* The player's profile: load and save                                                                            */
/* ------------------------------------------------------------------------------------------------------------ */

sd_load()
{
	p = spawnstruct();
	p.credits = 0;
	p.drops = 1;          // one to start with
	p.advanced = 0;
	p.playtime = 0;
	p.nextid = 1;
	p.winday = "";
	p.day = "";
	p.week = "";
	p.items = [];
	p.daily = [];
	p.weekly = [];

	text = sdread( "profile.txt" );
	if ( isdefined( text ) && text != "" )
	{
		foreach ( line in strtok( text, "\n" ) )
		{
			kv = strtok( line, "=" );
			if ( kv.size < 2 )
				continue;

			switch ( kv[ 0 ] )
			{
				case "credits": p.credits = int( kv[ 1 ] ); break;
				case "drops": p.drops = int( kv[ 1 ] ); break;
				case "advanced": p.advanced = int( kv[ 1 ] ); break;
				case "playtime": p.playtime = int( kv[ 1 ] ); break;
				case "nextid": p.nextid = int( kv[ 1 ] ); break;
				case "winday": p.winday = kv[ 1 ]; break;
				case "day": p.day = kv[ 1 ]; break;
				case "week": p.week = kv[ 1 ]; break;
				case "daily": p.daily[ p.daily.size ] = sd_contract_parse( kv[ 1 ] ); break;
				case "weekly": p.weekly[ p.weekly.size ] = sd_contract_parse( kv[ 1 ] ); break;
				case "item":
					item = sd_item_parse( kv[ 1 ] );
					if ( isdefined( item ) )
						p.items[ p.items.size ] = item;
					break;
			}
		}
	}

	self.sd = p;
	self sd_refresh_contracts();
}

sd_save()
{
	p = self.sd;
	text = "credits=" + p.credits + "\ndrops=" + p.drops + "\nadvanced=" + p.advanced + "\nplaytime=" + p.playtime
		+ "\nnextid=" + p.nextid + "\nwinday=" + p.winday + "\nday=" + p.day + "\nweek=" + p.week + "\n";

	foreach ( c in p.daily )
		text += "daily=" + sd_contract_text( c ) + "\n";
	foreach ( c in p.weekly )
		text += "weekly=" + sd_contract_text( c ) + "\n";
	foreach ( item in p.items )
		text += "item=" + sd_item_text( item ) + "\n";

	sdwrite( "profile.txt", text );
}

// id;base;rarity;name;equipped;type:value,type:value
sd_item_text( item )
{
	mods = "";
	for ( i = 0; i < item.modtypes.size; i++ )
	{
		if ( i > 0 )
			mods += ",";
		mods += item.modtypes[ i ] + ":" + item.modvalues[ i ];
	}
	return item.id + ";" + item.base + ";" + item.rarity + ";" + item.name + ";" + item.equipped + ";" + mods;
}

sd_item_parse( text )
{
	f = strtok( text, ";" );
	if ( f.size < 5 || !isdefined( level.sdWeaponName[ f[ 1 ] ] ) )
		return undefined;

	item = spawnstruct();
	item.id = int( f[ 0 ] );
	item.base = f[ 1 ];
	item.rarity = f[ 2 ];
	item.name = f[ 3 ];
	item.equipped = int( f[ 4 ] );
	item.modtypes = [];
	item.modvalues = [];
	if ( f.size > 5 )
	{
		foreach ( m in strtok( f[ 5 ], "," ) )
		{
			tv = strtok( m, ":" );
			if ( tv.size == 2 )
			{
				item.modtypes[ item.modtypes.size ] = tv[ 0 ];
				item.modvalues[ item.modvalues.size ] = int( tv[ 1 ] );
			}
		}
	}
	return item;
}

/* ------------------------------------------------------------------------------------------------------------ */
/* Variants                                                                                                       */
/* ------------------------------------------------------------------------------------------------------------ */

sd_roll_rarity( advanced )
{
	r = randomint( 100 );
	if ( r < scripts\mp\sd_config::chance_elite( advanced ) )
		return "elite";
	if ( r < scripts\mp\sd_config::chance_elite( advanced ) + scripts\mp\sd_config::chance_professional( advanced ) )
		return "professional";
	return "enlisted";
}

sd_new_variant( rarity )
{
	item = spawnstruct();
	item.id = self.sd.nextid;
	self.sd.nextid++;
	item.base = level.sdWeapons[ randomint( level.sdWeapons.size ) ];
	item.rarity = rarity;
	item.name = level.sdNames[ randomint( level.sdNames.size ) ];
	item.equipped = 0;
	item.modtypes = [];
	item.modvalues = [];

	// Advantages: different stats, each once
	pool = level.sdModTypes;
	count = scripts\mp\sd_config::advantages( rarity );
	low = scripts\mp\sd_config::advantage_min( rarity );
	high = scripts\mp\sd_config::advantage_max( rarity );
	for ( i = 0; i < count && pool.size > 0; i++ )
	{
		pick = randomint( pool.size );
		item.modtypes[ item.modtypes.size ] = pool[ pick ];
		item.modvalues[ item.modvalues.size ] = randomintrange( low, high + 1 );
		pool = sd_array_without( pool, pick );
	}

	// A drawback on one of the stats left
	if ( pool.size > 0 && randomint( 100 ) < scripts\mp\sd_config::drawback_chance( rarity ) )
	{
		pick = randomint( pool.size );
		item.modtypes[ item.modtypes.size ] = pool[ pick ];
		item.modvalues[ item.modvalues.size ] = 0 - randomintrange( scripts\mp\sd_config::drawback_min( rarity ), scripts\mp\sd_config::drawback_max( rarity ) + 1 );
	}

	return item;
}

sd_array_without( a, index )
{
	out = [];
	for ( i = 0; i < a.size; i++ )
	{
		if ( i != index )
			out[ out.size ] = a[ i ];
	}
	return out;
}

sd_item_title( item )
{
	return level.sdWeaponName[ item.base ] + " '" + item.name + "'";
}

sd_item_stats( item )
{
	text = "";
	for ( i = 0; i < item.modtypes.size; i++ )
	{
		if ( i > 0 )
			text += "   ";
		v = item.modvalues[ i ];
		if ( v >= 0 )
			text += "+" + v + "% ";
		else
			text += v + "% ";
		text += level.sdModName[ item.modtypes[ i ] ];
	}
	return text;
}

sd_item_mod( item, type )
{
	for ( i = 0; i < item.modtypes.size; i++ )
	{
		if ( item.modtypes[ i ] == type )
			return item.modvalues[ i ];
	}
	return 0;
}

sd_equipped_for( weapon )
{
	if ( !isdefined( self.sd ) )
		return undefined;

	base = sd_base_of( weapon );
	if ( base == "" )
		return undefined;

	foreach ( item in self.sd.items )
	{
		if ( item.equipped && item.base == base )
			return item;
	}
	return undefined;
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
		item = eAttacker sd_equipped_for( sWeapon );
		if ( isdefined( item ) )
		{
			percent = sd_item_mod( item, "dmg" );

			dist = distance( eAttacker.origin, self.origin );
			if ( dist < 750 )
				percent += sd_item_mod( item, "cqb" );
			else if ( dist > 1500 )
				percent += sd_item_mod( item, "lrg" );

			if ( isdefined( sHitLoc ) && ( sHitLoc == "head" || sHitLoc == "helmet" ) )
				percent += sd_item_mod( item, "hs" );

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
	if ( isdefined( attacker ) && isplayer( attacker ) && attacker != self && isdefined( attacker.sd ) )
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
		item = self sd_equipped_for( weapon );
		if ( !isdefined( item ) )
			continue;

		mobility = sd_item_mod( item, "mob" );
		if ( mobility != 0 )
		{
			scaler = 1;
			if ( isdefined( self.moveSpeedScaler ) )
				scaler = self.moveSpeedScaler;
			self setmovespeedscale( sd_class_speed( weapon ) * scaler * ( 100 + mobility ) / 100 );
		}

		if ( !isdefined( self.sdShown ) || self.sdShown != item.id )
		{
			self.sdShown = item.id;
			self iprintln( "^3" + sd_item_title( item ) + "^7  (" + level.sdRarityName[ item.rarity ] + ")" );
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
			item = self sd_equipped_for( weapon );
			if ( !isdefined( item ) )
				continue;

			ammo = sd_item_mod( item, "ammo" );
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

	self sd_load();
	self.sdLastPrestige = self maps\mp\gametypes\_rank::getPrestigeLevel();

	self thread sd_watch_weapon();
	self thread sd_watch_loadout();
	self thread sd_watch_playtime();
	self thread sd_watch_rank();
	self thread sd_menu_input();

	self waittill( "spawned_player" );
	wait 3;
	total = self.sd.drops + self.sd.advanced;
	if ( total > 0 )
		self iprintln( "^3Supply Drops:^7 " + total + " to open  -  [{+actionslot 2}] to open the menu" );
	else
		self iprintln( "^3Supply Drops:^7 [{+actionslot 2}] to open the menu" );
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

		self.sd.playtime++;
		if ( self.sd.playtime >= scripts\mp\sd_config::minutes_per_drop() )
		{
			self.sd.playtime = 0;
			self sd_give_drops( 1, 0, "Time played" );
		}
		else
		{
			self sd_save();
		}
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
			self sd_give_drops( 0, scripts\mp\sd_config::advanced_per_prestige(), "Prestige" );
		}
		else
		{
			self sd_give_drops( scripts\mp\sd_config::drops_per_level(), 0, "Level up" );
		}
	}
}

sd_give_drops( drops, advanced, reason )
{
	if ( drops <= 0 && advanced <= 0 )
		return;

	self.sd.drops += drops;
	self.sd.advanced += advanced;
	self sd_save();

	text = "^3" + reason + ":^7 ";
	if ( drops > 0 )
		text += "+" + drops + " Supply Drop";
	if ( advanced > 0 )
	{
		if ( drops > 0 )
			text += ", ";
		text += "+" + advanced + " Advanced Supply Drop";
	}
	self iprintlnbold( text );
	self playlocalsound( "mp_level_up" );
}

/* ------------------------------------------------------------------------------------------------------------ */
/* Contracts                                                                                                      */
/* ------------------------------------------------------------------------------------------------------------ */

sd_contract_parse( text )
{
	f = strtok( text, "," );
	c = spawnstruct();
	c.type = f[ 0 ];
	c.target = int( f[ 1 ] );
	c.progress = int( f[ 2 ] );
	c.done = int( f[ 3 ] );
	return c;
}

sd_contract_text( c )
{
	return c.type + "," + c.target + "," + c.progress + "," + c.done;
}

sd_new_contract( weekly )
{
	c = spawnstruct();
	c.type = level.sdContractTypes[ randomint( level.sdContractTypes.size ) ];
	c.progress = 0;
	c.done = 0;

	switch ( c.type )
	{
		case "kills": c.target = 40; break;
		case "headshots": c.target = 10; break;
		case "wins": c.target = 3; break;
		case "matches": c.target = 4; break;
		default: c.target = 20; break;
	}
	if ( weekly )
		c.target *= 5;
	return c;
}

sd_contract_name( c )
{
	switch ( c.type )
	{
		case "kills": return "Get " + c.target + " kills";
		case "headshots": return "Get " + c.target + " headshots";
		case "wins": return "Win " + c.target + " matches";
		case "matches": return "Finish " + c.target + " matches";
		case "kills_rifle": return "Get " + c.target + " kills with assault rifles";
		case "kills_smg": return "Get " + c.target + " kills with SMGs";
		case "kills_mg": return "Get " + c.target + " kills with LMGs";
		case "kills_sniper": return "Get " + c.target + " kills with sniper rifles";
		case "kills_spread": return "Get " + c.target + " kills with shotguns";
		case "kills_pistol": return "Get " + c.target + " kills with pistols";
	}
	return c.type;
}

// New daily contracts every day, a new weekly one every week
sd_refresh_contracts()
{
	today = sddate();
	if ( self.sd.day != today || self.sd.daily.size == 0 )
	{
		self.sd.day = today;
		self.sd.daily = [];
		for ( i = 0; i < scripts\mp\sd_config::daily_contracts(); i++ )
			self.sd.daily[ i ] = sd_new_contract( false );
	}

	week = sdweek();
	if ( self.sd.week != week || self.sd.weekly.size == 0 )
	{
		self.sd.week = week;
		self.sd.weekly = [];
		self.sd.weekly[ 0 ] = sd_new_contract( true );
	}
}

sd_progress( type, amount )
{
	changed = false;

	for ( i = 0; i < self.sd.daily.size; i++ )
	{
		c = self.sd.daily[ i ];
		if ( c.type == type && !c.done )
		{
			c.progress += amount;
			changed = true;
			if ( c.progress >= c.target )
			{
				c.progress = c.target;
				c.done = 1;
				self sd_give_drops( 1, 0, "Contract complete" );
			}
		}
	}

	for ( i = 0; i < self.sd.weekly.size; i++ )
	{
		c = self.sd.weekly[ i ];
		if ( c.type == type && !c.done )
		{
			c.progress += amount;
			changed = true;
			if ( c.progress >= c.target )
			{
				c.progress = c.target;
				c.done = 1;
				self sd_give_drops( 0, scripts\mp\sd_config::weekly_contract_advanced(), "Weekly contract complete" );
			}
		}
	}

	return changed;
}

/* ------------------------------------------------------------------------------------------------------------ */
/* End of the match                                                                                               */
/* ------------------------------------------------------------------------------------------------------------ */

sd_end_of_match()
{
	level waittill( "game_ended", winner );

	foreach ( player in level.players )
	{
		if ( !isdefined( player.sd ) )
			continue;

		player sd_menu_close();
		player sd_progress( "matches", 1 );

		won = false;
		if ( level.teamBased )
			won = isdefined( winner ) && isdefined( player.pers[ "team" ] ) && winner == player.pers[ "team" ];
		else
			won = isdefined( winner ) && winner == player;

		if ( won )
		{
			player sd_progress( "wins", 1 );
			if ( player.sd.winday != sddate() )
			{
				player.sd.winday = sddate();
				player sd_give_drops( scripts\mp\sd_config::first_win_of_day_drops(), 0, "First win of the day" );
			}
		}

		credits = scripts\mp\sd_config::match_credits( sd_place( player ) );
		player.sd.credits += credits;
		player iprintln( "^3+" + credits + " Credits" );
		player sd_save();
	}
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

/* ------------------------------------------------------------------------------------------------------------ */
/* Opening drops                                                                                                  */
/* ------------------------------------------------------------------------------------------------------------ */

sd_open_drop( advanced )
{
	if ( advanced )
	{
		if ( self.sd.advanced <= 0 )
			return;
		self.sd.advanced--;
	}
	else
	{
		if ( self.sd.drops <= 0 )
			return;
		self.sd.drops--;
	}

	items = [];
	for ( i = 0; i < scripts\mp\sd_config::items_per_drop(); i++ )
	{
		rarity = sd_roll_rarity( advanced );
		if ( advanced && i == 0 && rarity == "enlisted" )
			rarity = "professional";
		item = self sd_new_variant( rarity );
		items[ items.size ] = item;
		self.sd.items[ self.sd.items.size ] = item;
	}
	bonus = scripts\mp\sd_config::bonus_credits_per_drop();
	self.sd.credits += bonus;
	self sd_save();

	self sd_menu_close();
	self thread sd_reveal( items, bonus, advanced );
}

// The contents, one by one, in their rarity's colour
sd_reveal( items, bonus, advanced )
{
	self endon( "disconnect" );
	self notify( "sd_reveal" );
	self endon( "sd_reveal" );

	title = self createfontstring( "objective", 1.6 );
	title setpoint( "CENTER", "CENTER", 0, -110 );
	title.hidewheninmenu = true;
	if ( advanced )
		title settext( "ADVANCED SUPPLY DROP" );
	else
		title settext( "SUPPLY DROP" );

	lines = [];
	wait 0.8;
	for ( i = 0; i < items.size; i++ )
	{
		item = items[ i ];
		name = self createfontstring( "objective", 1.4 );
		name setpoint( "CENTER", "CENTER", 0, -60 + i * 55 );
		name.color = level.sdRarityColor[ item.rarity ];
		name.hidewheninmenu = true;
		name settext( toupper( level.sdRarityName[ item.rarity ] ) + "   " + sd_item_title( item ) );
		stats = self createfontstring( "default", 1.2 );
		stats setpoint( "CENTER", "CENTER", 0, -38 + i * 55 );
		stats.hidewheninmenu = true;
		stats settext( sd_item_stats( item ) );
		lines[ lines.size ] = name;
		lines[ lines.size ] = stats;
		if ( item.rarity == "elite" )
			self playlocalsound( "mp_level_up" );
		else
				wait 1.2;
	}

	credits = self createfontstring( "default", 1.3 );
	credits setpoint( "CENTER", "CENTER", 0, -60 + items.size * 55 );
	credits.hidewheninmenu = true;
	credits settext( "+" + bonus + " Credits   -   equip them in the Armory" );
	lines[ lines.size ] = credits;

	wait 5;
	title destroy();
	foreach ( l in lines )
		l destroy();
}

sd_salvage( index )
{
	item = self.sd.items[ index ];
	credits = scripts\mp\sd_config::salvage_credits( item.rarity );
	self.sd.credits += credits;
	self.sd.items = sd_array_without( self.sd.items, index );
	self sd_save();
	self iprintln( "Salvaged " + sd_item_title( item ) + ": ^3+" + credits + " Credits" );
}

sd_toggle_equip( index )
{
	item = self.sd.items[ index ];
	if ( item.equipped )
	{
		item.equipped = 0;
	}
	else
	{
		// One variant per weapon
		foreach ( other in self.sd.items )
		{
			if ( other.base == item.base )
				other.equipped = 0;
		}
		item.equipped = 1;
	}
	self sd_save();
	self.sdShown = undefined;
}

sd_buy( advanced )
{
	if ( advanced )
		price = scripts\mp\sd_config::price_advanced();
	else
		price = scripts\mp\sd_config::price_drop();

	if ( self.sd.credits < price )
	{
		self iprintln( "^1Not enough Credits^7 (" + self.sd.credits + "/" + price + ")" );
		return;
	}

	self.sd.credits -= price;
	if ( advanced )
		self.sd.advanced++;
	else
		self.sd.drops++;
	self sd_save();
	self iprintln( "^3Bought.^7 Credits left: " + self.sd.credits );
}

/* ------------------------------------------------------------------------------------------------------------ */
/* Menu                                                                                                           */
/* ------------------------------------------------------------------------------------------------------------ */

sd_menu_input()
{
	self endon( "disconnect" );

	self notifyonplayercommand( "sd_toggle", "+actionslot 2" );
	self notifyonplayercommand( "sd_up", "+forward" );
	self notifyonplayercommand( "sd_down", "+back" );
	self notifyonplayercommand( "sd_select", "+gostand" );
	self notifyonplayercommand( "sd_back", "+stance" );

	self thread sd_menu_key( "sd_up" );
	self thread sd_menu_key( "sd_down" );
	self thread sd_menu_key( "sd_select" );
	self thread sd_menu_key( "sd_back" );

	for ( ;; )
	{
		self waittill( "sd_toggle" );
		if ( isdefined( self.sdMenu ) )
			self sd_menu_close();
		else if ( isalive( self ) )
			self sd_menu_open( "main" );
	}
}

sd_menu_key( key )
{
	self endon( "disconnect" );

	for ( ;; )
	{
		self waittill( key );
		if ( !isdefined( self.sdMenu ) )
			continue;

		switch ( key )
		{
			case "sd_up":
				self.sdCursor--;
				if ( self.sdCursor < 0 )
					self.sdCursor = self.sdOptions.size - 1;
				self sd_menu_highlight();
				break;
			case "sd_down":
				self.sdCursor++;
				if ( self.sdCursor >= self.sdOptions.size )
					self.sdCursor = 0;
				self sd_menu_highlight();
				break;
			case "sd_select":
				self sd_menu_action( self.sdActions[ self.sdCursor ] );
				break;
			case "sd_back":
				self sd_menu_action( "back" );
				break;
		}
	}
}

sd_menu_open( page )
{
	self sd_menu_destroy();
	self.sdMenu = page;
	self.sdCursor = 0;
	self.sdOptions = [];
	self.sdActions = [];
	self freezecontrols( true );

	switch ( page )
	{
		case "main":
			self.sdTitle = "SUPPLY DROPS      Credits: " + self.sd.credits;
			self sd_option( "Open Supply Drop (" + self.sd.drops + ")", "open" );
			self sd_option( "Open Advanced Supply Drop (" + self.sd.advanced + ")", "open_adv" );
			self sd_option( "Armory (" + self.sd.items.size + " variants)", "armory" );
			self sd_option( "Store", "store" );
			self sd_option( "Contracts", "contracts" );
			self sd_option( "Close", "close" );
			break;

		case "store":
			self.sdTitle = "STORE      Credits: " + self.sd.credits;
			self sd_option( "Supply Drop - " + scripts\mp\sd_config::price_drop() + " Credits", "buy" );
			self sd_option( "Advanced Supply Drop - " + scripts\mp\sd_config::price_advanced() + " Credits", "buy_adv" );
			self sd_option( "Back", "back" );
			break;

		case "contracts":
			self.sdTitle = "CONTRACTS";
			foreach ( c in self.sd.daily )
				self sd_option( sd_contract_line( c, "Daily" ), "none" );
			foreach ( c in self.sd.weekly )
				self sd_option( sd_contract_line( c, "Weekly" ), "none" );
			self sd_option( "Back", "back" );
			break;

		case "armory":
			self.sdTitle = "ARMORY";
			if ( !isdefined( self.sdPage ) )
				self.sdPage = 0;
			perPage = 8;
			pages = int( ( self.sd.items.size + perPage - 1 ) / perPage );
			if ( self.sdPage >= pages )
				self.sdPage = 0;
			for ( i = self.sdPage * perPage; i < self.sd.items.size && i < ( self.sdPage + 1 ) * perPage; i++ )
			{
				item = self.sd.items[ i ];
				mark = "";
				if ( item.equipped )
					mark = "^2[E] ^7";
				self sd_option( mark + sd_item_title( item ) + " - " + level.sdRarityName[ item.rarity ], "item:" + i );
			}
			if ( pages > 1 )
				self sd_option( "Next page (" + ( self.sdPage + 1 ) + "/" + pages + ")", "nextpage" );
			self sd_option( "Back", "back" );
			break;

		case "item":
			item = self.sd.items[ self.sdItem ];
			self.sdTitle = sd_item_title( item ) + " - " + level.sdRarityName[ item.rarity ];
			self sd_option( sd_item_stats( item ), "none" );
			if ( item.equipped )
				self sd_option( "Unequip", "equip" );
			else
				self sd_option( "Equip", "equip" );
			self sd_option( "Salvage (+" + scripts\mp\sd_config::salvage_credits( item.rarity ) + " Credits)", "salvage" );
			self sd_option( "Back", "back" );
			break;
	}

	self sd_menu_draw();
}

sd_contract_line( c, kind )
{
	if ( c.done )
		return "^2" + kind + ": " + sd_contract_name( c ) + " - done";
	return kind + ": " + sd_contract_name( c ) + " - " + c.progress + "/" + c.target;
}

sd_option( text, action )
{
	self.sdOptions[ self.sdOptions.size ] = text;
	self.sdActions[ self.sdActions.size ] = action;
}

sd_menu_action( action )
{
	if ( !isdefined( self.sdMenu ) )
		return;

	page = self.sdMenu;
	if ( isSubStr( action, "item:" ) )
	{
		self.sdItem = int( getsubstr( action, 5 ) );
		self sd_menu_open( "item" );
		return;
	}

	switch ( action )
	{
		case "close": self sd_menu_close(); break;
		case "open": self sd_open_drop( false ); break;
		case "open_adv": self sd_open_drop( true ); break;
		case "armory": self.sdPage = 0; self sd_menu_open( "armory" ); break;
		case "store": self sd_menu_open( "store" ); break;
		case "contracts": self sd_menu_open( "contracts" ); break;
		case "buy": self sd_buy( false ); self sd_menu_open( "store" ); break;
		case "buy_adv": self sd_buy( true ); self sd_menu_open( "store" ); break;
		case "nextpage": self.sdPage++; self sd_menu_open( "armory" ); break;
		case "equip": self sd_toggle_equip( self.sdItem ); self sd_menu_open( "item" ); break;
		case "salvage": self sd_salvage( self.sdItem ); self sd_menu_open( "armory" ); break;
		case "back":
			if ( page == "item" )
				self sd_menu_open( "armory" );
			else if ( page == "main" )
				self sd_menu_close();
			else
				self sd_menu_open( "main" );
			break;
	}
}

sd_menu_draw()
{
	self.sdHud = [];

	bg = newclienthudelem( self );
	bg.x = 0;
	bg.y = -10;
	bg.alignx = "center";
	bg.aligny = "top";
	bg.horzalign = "center";
	bg.vertalign = "middle";
	bg setshader( "black", 420, 60 + self.sdOptions.size * 22 );
	bg.alpha = 0.7;
	bg.sort = -1;
	bg.y = -150;
	self.sdHud[ 0 ] = bg;

	title = self createfontstring( "objective", 1.4 );
	title setpoint( "CENTER", "CENTER", 0, -135 );
	title.color = ( 1, 0.8, 0.3 );
	title settext( self.sdTitle );
	self.sdHud[ 1 ] = title;

	self.sdLines = [];
	for ( i = 0; i < self.sdOptions.size; i++ )
	{
		line = self createfontstring( "default", 1.3 );
		line setpoint( "CENTER", "CENTER", 0, -105 + i * 22 );
		line settext( self.sdOptions[ i ] );
		self.sdLines[ i ] = line;
	}

	self sd_menu_highlight();
}

sd_menu_highlight()
{
	for ( i = 0; i < self.sdLines.size; i++ )
	{
		if ( i == self.sdCursor )
		{
			self.sdLines[ i ].color = ( 1, 0.85, 0.3 );
			self.sdLines[ i ].fontscale = 1.45;
		}
		else
		{
			self.sdLines[ i ].color = ( 1, 1, 1 );
			self.sdLines[ i ].fontscale = 1.3;
		}
	}
}

sd_menu_destroy()
{
	if ( isdefined( self.sdHud ) )
	{
		foreach ( h in self.sdHud )
			h destroy();
	}
	if ( isdefined( self.sdLines ) )
	{
		foreach ( l in self.sdLines )
			l destroy();
	}
	self.sdHud = undefined;
	self.sdLines = undefined;
}

sd_menu_close()
{
	self sd_menu_destroy();
	self.sdMenu = undefined;
	if ( isalive( self ) )
		self freezecontrols( false );
}
