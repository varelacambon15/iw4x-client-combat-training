/*
	Supply Drops: settings. Change the numbers here to tune the pace and the economy.
	(After a change, the next match uses the new values.)
*/

// --- Earning drops ------------------------------------------------------------------------------------------------
minutes_per_drop() { return 30; }          // minutes played in matches for a Supply Drop
drops_per_level() { return 1; }            // Supply Drops for each level gained
advanced_per_prestige() { return 2; }      // Advanced Supply Drops for each prestige
first_win_of_day_drops() { return 1; }     // Supply Drops for the first win of the day

// --- Credits ------------------------------------------------------------------------------------------------------
salvage_credits( rarity )                  // credits for salvaging a variant
{
	switch ( rarity )
	{
		case "elite": return 100;
		case "professional": return 30;
	}
	return 10;
}
match_credits( place )                     // credits at the end of a match; place: 0 = first ... 1 = last
{
	if ( place <= 0.15 ) return 15;
	if ( place <= 0.5 ) return 8;
	return 4;
}
price_drop() { return 120; }               // a Supply Drop in the Store
price_advanced() { return 350; }           // an Advanced Supply Drop in the Store

// --- What comes out of a drop -------------------------------------------------------------------------------------
items_per_drop() { return 2; }             // variants in each drop
bonus_credits_per_drop() { return 10; }    // plus these credits

// Chance of each rarity, in percent. An Advanced drop guarantees Professional or better in its first item.
chance_elite( advanced ) { if ( advanced ) return 25; return 8; }
chance_professional( advanced ) { if ( advanced ) return 75; return 30; }

// Each rarity: how many advantages, their size (percent), and the chance and size of a drawback
advantages( rarity ) { switch ( rarity ) { case "elite": return 3; case "professional": return 2; } return 1; }
advantage_min( rarity ) { switch ( rarity ) { case "elite": return 8; case "professional": return 5; } return 3; }
advantage_max( rarity ) { switch ( rarity ) { case "elite": return 14; case "professional": return 10; } return 6; }
drawback_chance( rarity ) { switch ( rarity ) { case "elite": return 50; case "professional": return 100; } return 50; }
drawback_min( rarity ) { switch ( rarity ) { case "elite": return 3; case "professional": return 3; } return 2; }
drawback_max( rarity ) { switch ( rarity ) { case "elite": return 6; case "professional": return 6; } return 4; }

// --- Contracts ----------------------------------------------------------------------------------------------------
daily_contracts() { return 3; }            // each gives a Supply Drop
weekly_contract_advanced() { return 1; }   // the weekly contract gives this many Advanced Supply Drops
