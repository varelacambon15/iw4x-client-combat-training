#pragma once

namespace Components
{
  // Supply Drops (Advanced Combat Training): Supply Drops with weapon variants, as in Advanced Warfare.
  //
  // Everything lives in the main menu: opening drops (with their reveal), the Armory, the Store and the Contracts,
  // and a variant step in Create a Class. The menus of the mod (ui_mp/sd_*.menu) run the sd_* commands below and
  // show the sd_* dvars they set. In a match, the scripts of the mod (scripts/mp/sd_supply_drops.gsc) apply the
  // equipped variants and give the drops, credits and contract progress through the SD* script functions.
  //
  // The save file is players/supply_drops/profile.txt in the game folder; the settings are supply_drops/config.txt
  // in the mod.
  class SupplyDrops : public Component
  {
  public:
    SupplyDrops();
  };
}
