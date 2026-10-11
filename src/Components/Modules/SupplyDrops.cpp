#include "SupplyDrops.hpp"
#include "Events.hpp"
#include "Menus.hpp"
#include "StructuredData.hpp"

#include "GSC/Script.hpp"

namespace Components
{
  namespace
  {
    // ------------------------------------------------------------------------------------------------------------
    // Tables
    // ------------------------------------------------------------------------------------------------------------

    struct WeaponInfo
    {
      const char* ref;      // as in mp/statsTable.csv and the weapon names (m4 -> m4_reflex_mp)
      const char* name;
      const char* category;
      const char* image;    // the picture of Create a Class
      int stats[5];         // accuracy, damage, range, fire rate, mobility, as the bars of Create a Class show them
    };

    // In the order of Create a Class
    constexpr WeaponInfo WEAPONS[] =
    {
      { "m4", "M4A1", "assault", "weapon_m4_short", { 75, 40, 75, 75, 66 } }, { "famas", "FAMAS", "assault", "weapon_famas", { 95, 40, 75, 90, 66 } }, { "scar", "SCAR-H", "assault", "weapon_scar_h", { 90, 75, 75, 35, 66 } },
      { "tavor", "TAR-21", "assault", "weapon_tavor", { 40, 75, 75, 55, 66 } }, { "fal", "FAL", "assault", "weapon_fnfal", { 90, 75, 75, 35, 66 } }, { "m16", "M16A4", "assault", "weapon_m16a4", { 85, 75, 75, 80, 66 } },
      { "masada", "ACR", "assault", "weapon_masada", { 95, 40, 75, 63, 66 } }, { "fn2000", "F2000", "assault", "weapon_fn2000", { 60, 40, 75, 82, 66 } }, { "ak47", "AK-47", "assault", "weapon_ak47", { 50, 75, 75, 50, 66 } },
      { "ak47classic", "AK-47 Classic", "assault", "weapon_ak47_classic", { 50, 75, 75, 50, 66 } },
      { "mp5k", "MP5K", "smg", "weapon_mp5k", { 70, 80, 40, 77, 100 } }, { "ump45", "UMP45", "smg", "weapon_ump45", { 85, 85, 40, 70, 100 } }, { "kriss", "Vector", "smg", "weapon_kriss", { 85, 50, 40, 95, 100 } }, { "p90", "P90", "smg", "weapon_p90", { 80, 75, 40, 90, 100 } },
      { "uzi", "Mini-Uzi", "smg", "weapon_mini_uzi", { 80, 75, 40, 85, 100 } }, { "ak74u", "AK-74u", "smg", "weapon_aks74u", { 85, 85, 40, 70, 100 } }, { "peacekeeper", "Peacekeeper", "smg", "weapon_peacekeeper", { 85, 85, 40, 70, 100 } },
      { "sa80", "L86 LSW", "lmg", "weapon_sa80", { 50, 75, 75, 75, 33 } }, { "rpd", "RPD", "lmg", "weapon_rpd", { 60, 75, 75, 50, 33 } }, { "mg4", "MG4", "lmg", "weapon_mg4", { 75, 40, 75, 78, 33 } }, { "aug", "AUG HBAR", "lmg", "weapon_aug", { 73, 75, 75, 50, 33 } },
      { "m240", "M240", "lmg", "weapon_m240", { 70, 40, 75, 80, 33 } },
      { "cheytac", "Intervention", "sniper", "weapon_cheytac", { 88, 90, 100, 20, 100 } }, { "barrett", "Barrett .50cal", "sniper", "weapon_barrett50cal", { 85, 90, 100, 70, 100 } }, { "wa2000", "WA2000", "sniper", "weapon_wa2000", { 87, 87, 100, 55, 100 } },
      { "m21", "M21 EBR", "sniper", "weapon_m14ebr", { 80, 75, 100, 60, 100 } }, { "m40a3", "M40A3", "sniper", "weapon_m40a3", { 88, 90, 100, 20, 100 } }, { "dragunov", "Dragunov", "sniper", "weapon_dragunov", { 88, 90, 100, 20, 100 } },
      { "pp2000", "PP2000", "mpistol", "weapon_pp2000", { 65, 70, 60, 73, 50 } }, { "glock", "G18", "mpistol", "weapon_glock", { 45, 60, 40, 90, 50 } }, { "beretta393", "M93 Raffica", "mpistol", "weapon_beretta393", { 80, 75, 60, 70, 50 } },
      { "tmp", "TMP", "mpistol", "weapon_tmp", { 85, 65, 60, 60, 50 } },
      { "spas12", "SPAS-12", "shotgun", "weapon_spas12", { 30, 75, 25, 55, 50 } }, { "aa12", "AA-12", "shotgun", "weapon_aa12", { 20, 77, 25, 90, 50 } }, { "striker", "Striker", "shotgun", "weapon_striker", { 25, 70, 35, 75, 50 } },
      { "ranger", "Ranger", "shotgun", "weapon_ranger", { 25, 85, 25, 25, 50 } }, { "m1014", "M1014", "shotgun", "weapon_m1014", { 25, 90, 33, 70, 50 } }, { "model1887", "Model 1887", "shotgun", "weapon_model1887", { 35, 80, 40, 5, 50 } },
      { "usp", "USP .45", "pistol", "weapon_usp_45", { 55, 30, 60, 85, 50 } }, { "coltanaconda", ".44 Magnum", "pistol", "weapon_colt_anaconda", { 27, 62, 72, 85, 50 } }, { "beretta", "M9", "pistol", "weapon_m9beretta", { 70, 30, 45, 85, 50 } },
      { "deserteagle", "Desert Eagle", "pistol", "weapon_desert_eagle", { 30, 60, 70, 85, 50 } },
    };

    // Weapons added by mods (supply_drops/weapons.txt): bought with COD Points, then chosen in Create a Class in place
    // of a stock weapon of their category. Only the ones whose weapon file is installed (weapons/mp/<ref>_mp) count.
    struct AddedWeapon
    {
      std::string ref;
      std::string name;
      std::string category;
      int price = 0;
      std::string image;
      int stats[5]{};
    };

    std::vector<AddedWeapon> AddedWeapons;
    bool AddedLoaded = false;

    // ref | name | category | price | image | accuracy damage range firerate mobility
    void LoadAddedWeapons()
    {
      AddedLoaded = true;
      AddedWeapons.clear();

      FileSystem::File file("supply_drops/weapons.txt");
      if (!file.exists())
      {
        return;
      }

      for (auto& line : Utils::String::Split(file.getBuffer(), '\n'))
      {
        Utils::String::Replace(line, "\r", "");
        Utils::String::Trim(line);
        if (line.empty() || line.starts_with("//") || line.starts_with("#"))
        {
          continue;
        }

        auto f = Utils::String::Split(line, '|');
        if (f.size() < 6)
        {
          continue;
        }
        for (auto& field : f)
        {
          Utils::String::Trim(field);
        }

        AddedWeapon weapon;
        weapon.ref = Utils::String::ToLower(f[0]);
        weapon.name = f[1];
        weapon.category = Utils::String::ToLower(f[2]);
        weapon.price = std::atoi(f[3].data());
        weapon.image = f[4];
        const auto values = Utils::String::Split(f[5], ' ');
        int n = 0;
        for (const auto& v : values)
        {
          if (!v.empty() && n < 5) weapon.stats[n++] = std::atoi(v.data());
        }

        if (!FileSystem::File(std::format("weapons/mp/{}_mp", weapon.ref)).exists())
        {
          continue; // not installed
        }
        AddedWeapons.emplace_back(std::move(weapon));
      }
    }

    const AddedWeapon* FindAdded(const std::string& ref)
    {
      if (!AddedLoaded) LoadAddedWeapons();
      for (const auto& weapon : AddedWeapons)
      {
        if (weapon.ref == ref) return &weapon;
      }
      return nullptr;
    }

    constexpr const char* NAMES[] =
    {
      "Ember", "Gale", "Fang", "Viper", "Onyx", "Tempest", "Rook", "Havoc", "Specter", "Talon", "Cinder", "Frost", "Nomad",
      "Warden", "Raptor", "Obsidian", "Monsoon", "Ironclad", "Banshee", "Sabre", "Vandal", "Lancer", "Harbinger", "Mirage",
      "Bulwark", "Jackal", "Eclipse", "Ravager", "Sentinel", "Thorn", "Cobalt", "Wraith", "Vortex", "Kestrel", "Bastion",
      "Riptide", "Outlaw", "Phantom", "Granite", "Hornet", "Requiem", "Atlas", "Siren", "Valkyrie", "Grim", "Nightfall",
      "Overdrive", "Solstice", "Brimstone", "Recluse", "Tundra", "Mamba", "Halcyon", "Ironside", "Dagger", "Hailstorm",
    };

    struct ModInfo
    {
      const char* type;
      const char* name;
    };

    constexpr ModInfo MODS[] =
    {
      { "dmg", "Damage" }, { "cqb", "Close Range Damage" }, { "lrg", "Long Range Damage" }, { "hs", "Headshot Damage" },
      { "mob", "Mobility" }, { "ammo", "Reserve Ammo" },
    };

    constexpr const char* RARITIES[] = { "enlisted", "professional", "elite" };
    constexpr const char* RARITY_NAMES[] = { "ENLISTED", "PROFESSIONAL", "ELITE" };
    constexpr const char* RARITY_COLORS[] = { "^2", "^5", "^3" };
    // The colour of each rarity in the menus, in percent (r, g, b), and of a base weapon
    constexpr int RARITY_RGB[][3] = { { 47, 86, 38 }, { 32, 64, 100 }, { 100, 74, 20 } };
    constexpr int BASE_RGB[3] = { 62, 62, 62 };

    constexpr const char* CONTRACT_TYPES[] =
    {
      "kills", "headshots", "wins", "matches", "kills_rifle", "kills_smg", "kills_mg", "kills_sniper", "kills_spread", "kills_pistol",
    };

    constexpr int ARMORY_ROWS = 10;
    constexpr int VARIANT_ROWS = 8;
    constexpr int CAC_ROWS = 10;
    constexpr int REVEAL_CARDS = 3;
    constexpr int CONTRACT_ROWS = 4;

    const WeaponInfo* FindWeapon(const std::string& ref)
    {
      for (const auto& weapon : WEAPONS)
      {
        if (ref == weapon.ref)
        {
          return &weapon;
        }
      }
      return nullptr;
    }

    std::string WeaponName(const std::string& ref)
    {
      if (const auto* weapon = FindWeapon(ref)) return weapon->name;
      if (const auto* added = FindAdded(ref)) return added->name;
      return ref;
    }


    const char* ModName(const std::string& type)
    {
      for (const auto& mod : MODS)
      {
        if (type == mod.type)
        {
          return mod.name;
        }
      }
      return "?";
    }

    int RarityIndex(const std::string& rarity)
    {
      for (int i = 0; i < 3; ++i)
      {
        if (rarity == RARITIES[i])
        {
          return i;
        }
      }
      return 0;
    }

    std::mt19937& Rng()
    {
      static std::mt19937 gen(std::random_device{}());
      return gen;
    }

    int Random(int low, int high) // both included
    {
      if (high < low)
      {
        return low;
      }
      std::uniform_int_distribution<int> dist(low, high);
      return dist(Rng());
    }

    std::string LocalTime(const char* format)
    {
      const auto now = std::time(nullptr);
      std::tm tm{};
      localtime_s(&tm, &now);
      char buffer[32]{};
      std::strftime(buffer, sizeof(buffer), format, &tm);
      return buffer;
    }

    void SetDvar(const std::string& name, const std::string& value)
    {
      Game::Dvar_SetCommand(name.data(), value.data());
    }

    void SetDvar(const std::string& name, int value)
    {
      SetDvar(name, std::to_string(value));
    }

    // <prefix>_cr, _cg, _cb: the colour the menus draw a rarity in (-1: a base weapon)
    void SetColorDvars(const std::string& prefix, int rarity)
    {
      const auto* rgb = rarity < 0 ? BASE_RGB : RARITY_RGB[std::clamp(rarity, 0, 2)];
      SetDvar(prefix + "_cr", rgb[0]);
      SetDvar(prefix + "_cg", rgb[1]);
      SetDvar(prefix + "_cb", rgb[2]);
    }

    // ------------------------------------------------------------------------------------------------------------
    // Settings: supply_drops/config.txt in the mod, "key = value" lines
    // ------------------------------------------------------------------------------------------------------------

    std::unordered_map<std::string, int> Config;

    void LoadConfig()
    {
      Config =
      {
        { "minutes_per_drop", 45 }, { "drops_per_level", 1 }, { "levels_per_drop", 5 }, { "advanced_per_prestige", 2 }, { "first_win_of_day_drops", 1 },
        { "salvage_enlisted", 20 }, { "salvage_professional", 50 }, { "salvage_elite", 150 },
        { "match_credits_top", 40 }, { "match_credits_middle", 25 }, { "match_credits_bottom", 15 },
        { "price_drop", 250 }, { "price_advanced", 600 },
        { "items_per_drop", 3 }, { "bonus_credits_per_drop", 15 },
        { "chance_elite", 8 }, { "chance_professional", 30 }, { "chance_elite_advanced", 25 }, { "chance_professional_advanced", 75 },
        { "advantages_enlisted", 1 }, { "advantages_professional", 2 }, { "advantages_elite", 3 },
        { "advantage_min_enlisted", 3 }, { "advantage_max_enlisted", 6 },
        { "advantage_min_professional", 5 }, { "advantage_max_professional", 10 },
        { "advantage_min_elite", 8 }, { "advantage_max_elite", 14 },
        { "drawback_chance_enlisted", 50 }, { "drawback_chance_professional", 100 }, { "drawback_chance_elite", 50 },
        { "drawback_min_enlisted", 2 }, { "drawback_max_enlisted", 4 },
        { "drawback_min_professional", 3 }, { "drawback_max_professional", 6 },
        { "drawback_min_elite", 3 }, { "drawback_max_elite", 6 },
        { "daily_contracts", 3 }, { "weekly_contract_advanced", 1 },
      };

      FileSystem::File file("supply_drops/config.txt");
      if (!file.exists())
      {
        return;
      }

      for (auto& line : Utils::String::Split(file.getBuffer(), '\n'))
      {
        Utils::String::Replace(line, "\r", "");
        if (const auto comment = line.find("//"); comment != std::string::npos)
        {
          line = line.substr(0, comment);
        }

        const auto eq = line.find('=');
        if (eq == std::string::npos)
        {
          continue;
        }

        auto key = line.substr(0, eq);
        auto value = line.substr(eq + 1);
        Utils::String::Trim(key);
        Utils::String::Trim(value);
        if (!key.empty() && !value.empty())
        {
          Config[Utils::String::ToLower(key)] = std::atoi(value.data());
        }
      }
    }

    int Cfg(const std::string& key)
    {
      if (Config.empty())
      {
        LoadConfig();
      }

      const auto it = Config.find(key);
      return it == Config.end() ? 0 : it->second;
    }

    // ------------------------------------------------------------------------------------------------------------
    // The profile
    // ------------------------------------------------------------------------------------------------------------

    struct Item
    {
      int id = 0;
      std::string base;
      std::string rarity;
      std::string name;
      bool equipped = false;
      std::vector<std::pair<std::string, int>> mods;

      [[nodiscard]] int Mod(const std::string& type) const
      {
        for (const auto& [t, v] : mods)
        {
          if (t == type)
          {
            return v;
          }
        }
        return 0;
      }
    };

    struct Contract
    {
      std::string type;
      int target = 1;
      int progress = 0;
      bool done = false;
    };

    struct Profile
    {
      int credits = 0;
      int drops = 1; // one to start with
      int advanced = 0;
      int playtime = 0;
      int nextid = 1;
      std::string winday;
      std::string day;
      std::string week;
      std::vector<Contract> daily;
      std::vector<Contract> weekly;
      std::vector<Item> items;
      std::vector<std::string> owned;                       // added weapons bought
      std::unordered_map<std::string, std::string> replace; // stock weapon -> added weapon used in its place
    };

    Profile Data;
    bool Loaded = false;

    const char* PROFILE_PATH = "players/supply_drops/profile.txt";

    Contract ParseContract(const std::string& text)
    {
      const auto f = Utils::String::Split(text, ',');
      Contract c;
      if (f.size() >= 4)
      {
        c.type = f[0];
        c.target = std::max(1, std::atoi(f[1].data()));
        c.progress = std::atoi(f[2].data());
        c.done = std::atoi(f[3].data()) != 0;
      }
      return c;
    }

    std::string ContractText(const Contract& c)
    {
      return std::format("{},{},{},{}", c.type, c.target, c.progress, c.done ? 1 : 0);
    }

    // id;base;rarity;name;equipped;type:value,type:value
    bool ParseItem(const std::string& text, Item& item)
    {
      const auto f = Utils::String::Split(text, ';');
      if (f.size() < 5 || !FindWeapon(f[1]))
      {
        return false;
      }

      item.id = std::atoi(f[0].data());
      item.base = f[1];
      item.rarity = RARITIES[RarityIndex(f[2])];
      item.name = f[3];
      item.equipped = std::atoi(f[4].data()) != 0;
      item.mods.clear();
      if (f.size() > 5)
      {
        for (const auto& m : Utils::String::Split(f[5], ','))
        {
          const auto tv = Utils::String::Split(m, ':');
          if (tv.size() == 2)
          {
            item.mods.emplace_back(tv[0], std::atoi(tv[1].data()));
          }
        }
      }
      return true;
    }

    std::string ItemText(const Item& item)
    {
      std::string mods;
      for (const auto& [type, value] : item.mods)
      {
        if (!mods.empty())
        {
          mods += ",";
        }
        mods += std::format("{}:{}", type, value);
      }
      return std::format("{};{};{};{};{};{}", item.id, item.base, item.rarity, item.name, item.equipped ? 1 : 0, mods);
    }

    void Save()
    {
      auto& p = Data;
      std::string text = std::format("credits={}\ndrops={}\nadvanced={}\nplaytime={}\nnextid={}\nwinday={}\nday={}\nweek={}\n",
        p.credits, p.drops, p.advanced, p.playtime, p.nextid, p.winday, p.day, p.week);

      for (const auto& c : p.daily)
      {
        text += "daily=" + ContractText(c) + "\n";
      }
      for (const auto& c : p.weekly)
      {
        text += "weekly=" + ContractText(c) + "\n";
      }
      for (const auto& item : p.items)
      {
        text += "item=" + ItemText(item) + "\n";
      }
      for (const auto& ref : p.owned)
      {
        text += "owned=" + ref + "\n";
      }
      for (const auto& [base, added] : p.replace)
      {
        text += "replace=" + base + ":" + added + "\n";
      }

      Utils::IO::WriteFile(PROFILE_PATH, text, false);
    }

    Contract NewContract(bool weekly)
    {
      Contract c;
      c.type = CONTRACT_TYPES[Random(0, static_cast<int>(std::size(CONTRACT_TYPES)) - 1)];

      if (c.type == "kills") c.target = 40;
      else if (c.type == "headshots") c.target = 10;
      else if (c.type == "wins") c.target = 3;
      else if (c.type == "matches") c.target = 4;
      else c.target = 20;

      if (weekly)
      {
        c.target *= 5;
      }
      return c;
    }

    // New daily contracts every day and a new weekly one every week
    void RefreshContracts()
    {
      bool changed = false;

      const auto today = LocalTime("%Y-%m-%d");
      if (Data.day != today || Data.daily.empty())
      {
        Data.day = today;
        Data.daily.clear();
        for (int i = 0; i < Cfg("daily_contracts"); ++i)
        {
          Data.daily.emplace_back(NewContract(false));
        }
        changed = true;
      }

      const auto week = LocalTime("%G-W%V");
      if (Data.week != week || Data.weekly.empty())
      {
        Data.week = week;
        Data.weekly.clear();
        Data.weekly.emplace_back(NewContract(true));
        changed = true;
      }

      if (changed)
      {
        Save();
      }
    }

    void Load()
    {
      if (Loaded)
      {
        return;
      }

      Loaded = true;
      Data = {};
      LoadConfig();
      LoadAddedWeapons();

      std::string text;
      if (Utils::IO::ReadFile(PROFILE_PATH, &text))
      {
        for (auto& line : Utils::String::Split(text, '\n'))
        {
          Utils::String::Replace(line, "\r", "");
          const auto eq = line.find('=');
          if (eq == std::string::npos)
          {
            continue;
          }

          const auto key = line.substr(0, eq);
          const auto value = line.substr(eq + 1);

          if (key == "credits") Data.credits = std::atoi(value.data());
          else if (key == "drops") Data.drops = std::atoi(value.data());
          else if (key == "advanced") Data.advanced = std::atoi(value.data());
          else if (key == "playtime") Data.playtime = std::atoi(value.data());
          else if (key == "nextid") Data.nextid = std::atoi(value.data());
          else if (key == "winday") Data.winday = value;
          else if (key == "day") Data.day = value;
          else if (key == "week") Data.week = value;
          else if (key == "daily") Data.daily.emplace_back(ParseContract(value));
          else if (key == "weekly") Data.weekly.emplace_back(ParseContract(value));
          else if (key == "owned") Data.owned.emplace_back(value);
          else if (key == "replace")
          {
            const auto colon = value.find(':');
            if (colon != std::string::npos) Data.replace[value.substr(0, colon)] = value.substr(colon + 1);
          }
          else if (key == "item")
          {
            Item item;
            if (ParseItem(value, item))
            {
              Data.nextid = std::max(Data.nextid, item.id + 1);
              Data.items.emplace_back(std::move(item));
            }
          }
        }
      }

      RefreshContracts();
    }

    Item* FindItem(int id)
    {
      for (auto& item : Data.items)
      {
        if (item.id == id)
        {
          return &item;
        }
      }
      return nullptr;
    }

    Item* EquippedFor(const std::string& base)
    {
      for (auto& item : Data.items)
      {
        if (item.equipped && item.base == base)
        {
          return &item;
        }
      }
      return nullptr;
    }

    int CountFor(const std::string& base)
    {
      return static_cast<int>(std::ranges::count_if(Data.items, [&](const Item& item) { return item.base == base; }));
    }

    // Equipped first, then the best rarity, then the newest
    std::vector<Item*> VariantsOf(const std::string& base)
    {
      std::vector<Item*> list;
      for (auto& item : Data.items)
      {
        if (item.base == base)
        {
          list.emplace_back(&item);
        }
      }

      std::ranges::sort(list, [](const Item* a, const Item* b)
      {
        if (a->equipped != b->equipped) return a->equipped;
        const auto ra = RarityIndex(a->rarity);
        const auto rb = RarityIndex(b->rarity);
        if (ra != rb) return ra > rb;
        return a->id > b->id;
      });

      return list;
    }

    // ------------------------------------------------------------------------------------------------------------
    // Variants
    // ------------------------------------------------------------------------------------------------------------

    std::string RollRarity(bool advanced)
    {
      const auto elite = Cfg(advanced ? "chance_elite_advanced" : "chance_elite");
      const auto professional = Cfg(advanced ? "chance_professional_advanced" : "chance_professional");
      const auto r = Random(0, 99);
      if (r < elite) return "elite";
      if (r < elite + professional) return "professional";
      return "enlisted";
    }

    Item NewVariant(const std::string& rarity)
    {
      Item item;
      item.id = Data.nextid++;
      item.base = WEAPONS[Random(0, static_cast<int>(std::size(WEAPONS)) - 1)].ref;
      item.rarity = rarity;
      item.name = NAMES[Random(0, static_cast<int>(std::size(NAMES)) - 1)];

      std::vector<std::string> pool;
      for (const auto& mod : MODS)
      {
        pool.emplace_back(mod.type);
      }

      // Advantages: different stats, each once
      const auto count = Cfg("advantages_" + rarity);
      for (int i = 0; i < count && !pool.empty(); ++i)
      {
        const auto pick = Random(0, static_cast<int>(pool.size()) - 1);
        item.mods.emplace_back(pool[pick], Random(Cfg("advantage_min_" + rarity), Cfg("advantage_max_" + rarity)));
        pool.erase(pool.begin() + pick);
      }

      // A drawback on one of the stats left
      if (!pool.empty() && Random(0, 99) < Cfg("drawback_chance_" + rarity))
      {
        const auto pick = Random(0, static_cast<int>(pool.size()) - 1);
        item.mods.emplace_back(pool[pick], -Random(Cfg("drawback_min_" + rarity), Cfg("drawback_max_" + rarity)));
      }

      return item;
    }

    std::string Title(const Item& item)
    {
      return std::format("{} '{}'", WeaponName(item.base), item.name);
    }

    int SalvageValue(const Item& item)
    {
      return Cfg("salvage_" + item.rarity);
    }

    // ------------------------------------------------------------------------------------------------------------
    // What the menus show
    // ------------------------------------------------------------------------------------------------------------

    // A weapon card: <prefix>_ref, _weapon, _name, _rarity (-1 none, 0..2), _rname, _ddmg/_drng/_dmob (how much the
    // stat bars grow), _mod0.._mod3, _equipped, _salvage, _id
    // The picture and the stock stats of a weapon, stock or added
    void PublishWeaponLook(const std::string& p, const std::string& ref)
    {
      const int* stats = nullptr;
      std::string image;
      if (const auto* weapon = FindWeapon(ref))
      {
        stats = weapon->stats;
        image = weapon->image;
      }
      else if (const auto* added = FindAdded(ref))
      {
        stats = added->stats;
        image = added->image;
      }

      SetDvar(p + "_img", image);
      for (int i = 0; i < 5; ++i)
      {
        SetDvar(std::format("{}_s{}", p, i), stats ? stats[i] : 0);
      }
    }

    // A weapon card: <prefix>_ref, _img, _s0.._s4 (the stock bars), _weapon, _name, _rarity (-1 none, 0..2) and its
    // colour, _rname, _ddmg/_drng/_dmob (how much the bars grow), _mod0.._mod3, _equipped, _salvage, _id.
    // With no variant: the weapon itself (an added weapon, or a stock one).
    void PublishCard(const std::string& p, const Item* item, const std::string& ref)
    {
      const auto weaponRef = item ? item->base : ref;
      SetDvar(p + "_ref", weaponRef);
      SetDvar(p + "_weapon", Utils::String::ToUpper(WeaponName(weaponRef)));
      PublishWeaponLook(p, weaponRef);

      if (!item)
      {
        const auto* added = FindAdded(weaponRef);
        SetDvar(p + "_name", added ? added->name : "Standard Issue");
        SetDvar(p + "_rarity", -1);
        SetColorDvars(p, added ? 2 : -1);
        SetDvar(p + "_rname", added ? "NEW WEAPON" : "BASE WEAPON");
        SetDvar(p + "_ddmg", 0);
        SetDvar(p + "_drng", 0);
        SetDvar(p + "_dmob", 0);
        SetDvar(p + "_mod0", added ? std::string("In Create a Class, in STORE WEAPONS.") : std::string("No changes to the weapon."));
        for (int i = 1; i < 4; ++i)
        {
          SetDvar(std::format("{}_mod{}", p, i), "");
        }
        SetDvar(p + "_equipped", "");
        SetDvar(p + "_salvage", "");
        SetDvar(p + "_id", 0);
        return;
      }

      const auto rarity = RarityIndex(item->rarity);
      SetDvar(p + "_name", item->name);
      SetDvar(p + "_rarity", rarity);
      SetColorDvars(p, rarity);
      SetDvar(p + "_rname", RARITY_NAMES[rarity]);
      // The bars are 0..100: one point of the variant is two of the bar, so that it can be seen
      SetDvar(p + "_ddmg", item->Mod("dmg") * 2);
      SetDvar(p + "_drng", item->Mod("lrg") * 2);
      SetDvar(p + "_dmob", item->Mod("mob") * 2);

      for (int i = 0; i < 4; ++i)
      {
        std::string line;
        if (i < static_cast<int>(item->mods.size()))
        {
          const auto& [type, value] = item->mods[i];
          line = value >= 0 ? std::format("^2+{}%^7  {}", value, ModName(type)) : std::format("^1{}%^7  {}", value, ModName(type));
        }
        SetDvar(std::format("{}_mod{}", p, i), line);
      }

      SetDvar(p + "_equipped", item->equipped ? "EQUIPPED" : "");
      SetDvar(p + "_salvage", std::format("{}", SalvageValue(*item)));
      SetDvar(p + "_id", item->id);
    }

    std::string ContractName(const Contract& c)
    {
      if (c.type == "kills") return std::format("Get {} kills", c.target);
      if (c.type == "headshots") return std::format("Get {} headshots", c.target);
      if (c.type == "wins") return std::format("Win {} matches", c.target);
      if (c.type == "matches") return std::format("Finish {} matches", c.target);
      if (c.type == "kills_rifle") return std::format("Get {} kills with assault rifles", c.target);
      if (c.type == "kills_smg") return std::format("Get {} kills with SMGs", c.target);
      if (c.type == "kills_mg") return std::format("Get {} kills with LMGs", c.target);
      if (c.type == "kills_sniper") return std::format("Get {} kills with sniper rifles", c.target);
      if (c.type == "kills_spread") return std::format("Get {} kills with shotguns", c.target);
      if (c.type == "kills_pistol") return std::format("Get {} kills with pistols", c.target);
      return c.type;
    }

    void PublishMain()
    {
      Load();
      RefreshContracts();

      SetDvar("sd_drops", Data.drops);
      SetDvar("sd_adv", Data.advanced);
      SetDvar("sd_credits", Data.credits);
      SetDvar("sd_items", static_cast<int>(Data.items.size()));
      SetDvar("sd_price_drop", Cfg("price_drop"));
      SetDvar("sd_price_adv", Cfg("price_advanced"));
      SetDvar("sd_next_drop", std::format("{} min", std::max(0, Cfg("minutes_per_drop") - Data.playtime)));

      std::vector<std::pair<const Contract*, bool>> contracts;
      for (const auto& c : Data.daily) contracts.emplace_back(&c, false);
      for (const auto& c : Data.weekly) contracts.emplace_back(&c, true);

      for (int i = 0; i < CONTRACT_ROWS; ++i)
      {
        const auto p = std::format("sd_c{}", i);
        if (i >= static_cast<int>(contracts.size()))
        {
          SetDvar(p + "_text", "");
          continue;
        }

        const auto& [c, weekly] = contracts[i];
        SetDvar(p + "_text", ContractName(*c));
        SetDvar(p + "_kind", weekly ? "WEEKLY" : "DAILY");
        SetDvar(p + "_reward", weekly ? std::format("{} Advanced Supply Drop", Cfg("weekly_contract_advanced")) : "1 Supply Drop");
        SetDvar(p + "_prog", c->done ? "COMPLETE" : std::format("{} / {}", c->progress, c->target));
        SetDvar(p + "_frac", c->done ? 100 : std::clamp(c->progress * 100 / std::max(1, c->target), 0, 100));
        SetDvar(p + "_done", c->done ? 1 : 0);
      }

      // Create a Class: a store weapon can be chosen once bought
      if (!AddedLoaded) LoadAddedWeapons();
      for (const auto& added : AddedWeapons)
      {
        SetDvar(std::format("sd_owned_{}", added.ref), std::ranges::find(Data.owned, added.ref) != Data.owned.end() ? 1 : 0);
      }

      for (const auto& weapon : WEAPONS)
      {
        SetDvar(std::format("sd_own_{}", weapon.ref), CountFor(weapon.ref));
      }
    }

    // --- Opening -----------------------------------------------------------------------------------------------------

    // --- The reveal animation: timed here, read by sd_reveal.menu ------------------------------------------------------

    constexpr int T_OPEN = 1300;      // the lid comes off
    constexpr int T_CARDS = 1900;     // the first card
    constexpr int T_CARD_STEP = 650;  // between cards

    bool RevealActive = false;
    int RevealStart = 0;
    int RevealCount = 0;
    int RevealEnd = 0;
    bool RevealElite[REVEAL_CARDS]{};

    float Clamp01(float x)
    {
      return std::clamp(x, 0.0f, 1.0f);
    }

    int Pct(float x)
    {
      return static_cast<int>(Clamp01(x) * 100.0f + 0.5f);
    }

    void StartReveal(int count)
    {
      RevealActive = true;
      RevealStart = Game::Sys_Milliseconds();
      RevealCount = count;
      RevealEnd = T_CARDS + T_CARD_STEP * (count - 1) + 500;
    }

    void UpdateReveal()
    {
      if (!RevealActive)
      {
        return;
      }

      const auto t = Game::Sys_Milliseconds() - RevealStart;
      const auto tf = static_cast<float>(t);

      const auto shake = t < T_OPEN ? std::sin(tf * 0.07f) * 4.0f * tf / T_OPEN : 0.0f;
      SetDvar("sd_rv_shake", static_cast<int>(shake * 10.0f));
      SetDvar("sd_rv_lid", t > T_OPEN ? static_cast<int>((tf - T_OPEN) * 0.35f) : 0);
      SetDvar("sd_rv_crate_a", Pct((T_CARDS - tf) / 300.0f));
      SetDvar("sd_rv_glow", Pct(Clamp01((tf - T_OPEN) / 500.0f) * 0.12f));

      const auto beamGrow = Clamp01((tf - T_OPEN + 200.0f) / 400.0f);
      SetDvar("sd_rv_beam_x", static_cast<int>(-4.0f - 40.0f * beamGrow));
      SetDvar("sd_rv_beam_w", static_cast<int>(8.0f + 80.0f * beamGrow));
      SetDvar("sd_rv_beam_a", Pct(0.7f * Clamp01((tf - T_OPEN + 200.0f) / 300.0f) * Clamp01((T_CARDS - tf) / 300.0f)));
      SetDvar("sd_rv_flash", t > T_OPEN + 250 ? Pct(0.85f * (1.0f - (tf - T_OPEN - 250.0f) / 650.0f)) : 0);

      for (int i = 0; i < REVEAL_CARDS; ++i)
      {
        const auto p = std::format("sd_rv{}", i);
        const auto since = tf - static_cast<float>(T_CARDS + i * T_CARD_STEP);
        const auto a = i < RevealCount ? Clamp01(since / 350.0f) : 0.0f;
        SetDvar(p + "_a", Pct(a));
        SetDvar(p + "_dy", static_cast<int>(40.0f * (1.0f - a)));
        SetDvar(p + "_fl", i < RevealCount && since > 0 ? Pct((RevealElite[i] ? 0.7f : 0.35f) * (1.0f - since / 900.0f)) : 0);
      }

      const auto done = t > RevealEnd;
      SetDvar("sd_rv_done", done ? 1 : 0);
      if (t > RevealEnd + 1000)
      {
        RevealActive = false; // everything is in place: no more updates
      }
    }

    void OpenDrop(bool advanced)
    {
      Load();
      auto& count = advanced ? Data.advanced : Data.drops;
      if (count <= 0)
      {
        SetDvar("sd_rv_ok", 0);
        return;
      }
      --count;

      std::vector<Item> items;
      const auto amount = std::clamp(Cfg("items_per_drop"), 1, REVEAL_CARDS);
      for (int i = 0; i < amount; ++i)
      {
        auto rarity = RollRarity(advanced);
        if (advanced && i == 0 && rarity == "enlisted")
        {
          rarity = "professional"; // an Advanced Supply Drop has one Professional or better
        }
        items.emplace_back(NewVariant(rarity));
      }

      // The best one last, as the big reveal
      std::ranges::stable_sort(items, [](const Item& a, const Item& b) { return RarityIndex(a.rarity) < RarityIndex(b.rarity); });

      const auto bonus = Cfg("bonus_credits_per_drop");
      Data.credits += bonus;

      int best = 0;
      for (int i = 0; i < REVEAL_CARDS; ++i)
      {
        const auto p = std::format("sd_rv{}", i);
        if (i < static_cast<int>(items.size()))
        {
          Data.items.emplace_back(items[i]);
          PublishCard(p, &items[i], "");
          SetDvar(p + "_on", 1);
          best = std::max(best, RarityIndex(items[i].rarity));
        }
        else
        {
          SetDvar(p + "_on", 0);
        }
      }

      Save();

      for (int i = 0; i < REVEAL_CARDS; ++i)
      {
        RevealElite[i] = i < static_cast<int>(items.size()) && items[i].rarity == "elite";
      }
      StartReveal(static_cast<int>(items.size()));
      UpdateReveal();

      SetDvar("sd_rv_ok", 1);
      SetDvar("sd_rv_count", static_cast<int>(items.size()));
      SetDvar("sd_rv_adv", advanced ? 1 : 0);
      SetDvar("sd_rv_title", advanced ? "ADVANCED SUPPLY DROP" : "SUPPLY DROP");
      SetDvar("sd_rv_best", best);
      SetColorDvars("sd_rv_best", best);
      SetDvar("sd_rv_bonus", std::format("+{} COD POINTS", bonus));
      PublishMain();
    }

    void Buy(bool advanced)
    {
      Load();
      const auto price = Cfg(advanced ? "price_advanced" : "price_drop");
      if (Data.credits < price)
      {
        SetDvar("sd_msg", std::format("^1Not enough COD Points^7 ({} / {})", Data.credits, price));
        return;
      }

      Data.credits -= price;
      (advanced ? Data.advanced : Data.drops)++;
      Save();
      SetDvar("sd_msg", advanced ? "^2Purchased:^7 Advanced Supply Drop" : "^2Purchased:^7 Supply Drop");
      PublishMain();
    }

    // --- Armory: the weapons you have variants for, then the variants of one ------------------------------------------

    int ArmoryPage = 0;
    int ArmoryFocus = 0;
    std::vector<std::string> ArmorySlots;   // weapon refs on the page
    std::string VariantWeapon;
    int VariantPage = 0;
    int VariantFocus = 0;
    std::vector<int> VariantSlots;          // item ids on the page

    // Page + change, wrapping around
    int TurnPage(int page, int change, int pages)
    {
      return ((page + change) % pages + pages) % pages;
    }

    // The card of an Armory row: the weapon with the variant it has equipped
    void PublishArmoryFocus()
    {
      if (ArmoryFocus < 0 || ArmoryFocus >= static_cast<int>(ArmorySlots.size()))
      {
        ArmoryFocus = 0;
      }
      if (ArmorySlots.empty())
      {
        return;
      }
      const auto& ref = ArmorySlots[ArmoryFocus];
      PublishCard("sd_af", EquippedFor(ref), ref);
    }

    void PublishArmory()
    {
      Load();

      std::vector<const WeaponInfo*> owned;
      for (const auto& weapon : WEAPONS)
      {
        if (CountFor(weapon.ref) > 0)
        {
          owned.emplace_back(&weapon);
        }
      }

      const auto pages = std::max(1, static_cast<int>((owned.size() + ARMORY_ROWS - 1) / ARMORY_ROWS));
      ArmoryPage = TurnPage(ArmoryPage, 0, pages);
      ArmorySlots.clear();

      for (int i = 0; i < ARMORY_ROWS; ++i)
      {
        const auto p = std::format("sd_aw{}", i);
        const auto index = ArmoryPage * ARMORY_ROWS + i;
        if (index >= static_cast<int>(owned.size()))
        {
          SetDvar(p + "_name", "");
          continue;
        }

        const auto* weapon = owned[index];
        ArmorySlots.emplace_back(weapon->ref);
        const auto* equipped = EquippedFor(weapon->ref);
        SetDvar(p + "_name", Utils::String::ToUpper(weapon->name));
        SetDvar(p + "_ref", weapon->ref);
        SetDvar(p + "_count", std::format("{}", CountFor(weapon->ref)));
        SetDvar(p + "_eq", equipped ? std::format("{}{}", RARITY_COLORS[RarityIndex(equipped->rarity)], equipped->name) : "^7-");
      }

      SetDvar("sd_aw_count", static_cast<int>(owned.size()));
      SetDvar("sd_aw_pages", pages);
      SetDvar("sd_aw_page", std::format("{} / {}", ArmoryPage + 1, pages));
      PublishArmoryFocus();
      PublishMain();
    }

    void PublishVariants()
    {
      Load();
      const auto list = VariantsOf(VariantWeapon);
      const auto pages = std::max(1, static_cast<int>((list.size() + VARIANT_ROWS - 1) / VARIANT_ROWS));
      VariantPage = TurnPage(VariantPage, 0, pages);
      VariantSlots.clear();

      for (int i = 0; i < VARIANT_ROWS; ++i)
      {
        const auto p = std::format("sd_av{}", i);
        const auto index = VariantPage * VARIANT_ROWS + i;
        if (index >= static_cast<int>(list.size()))
        {
          SetDvar(p + "_name", "");
          continue;
        }

        const auto* item = list[index];
        VariantSlots.emplace_back(item->id);
        SetDvar(p + "_name", std::format("{}{}", RARITY_COLORS[RarityIndex(item->rarity)], item->name));
        SetDvar(p + "_rname", RARITY_NAMES[RarityIndex(item->rarity)]);
        SetDvar(p + "_eq", item->equipped ? 1 : 0);
      }

      const auto* weapon = FindWeapon(VariantWeapon);
      SetDvar("sd_av_weapon", weapon ? Utils::String::ToUpper(weapon->name) : "");
      SetDvar("sd_av_ref", VariantWeapon);
      SetDvar("sd_av_count", static_cast<int>(list.size()));
      SetDvar("sd_av_pages", pages);
      SetDvar("sd_av_page", std::format("{} / {}", VariantPage + 1, pages));
      if (list.empty())
      {
        PublishCard("sd_f", nullptr, VariantWeapon);
      }
    }

    Item* VariantAt(int slot)
    {
      if (slot < 0 || slot >= static_cast<int>(VariantSlots.size()))
      {
        return nullptr;
      }
      return FindItem(VariantSlots[slot]);
    }

    void Equip(Item& item, bool equip)
    {
      for (auto& other : Data.items)
      {
        if (other.base == item.base)
        {
          other.equipped = false;
        }
      }
      item.equipped = equip;
      Save();
    }

    void Salvage(int id)
    {
      const auto it = std::ranges::find_if(Data.items, [&](const Item& item) { return item.id == id; });
      if (it == Data.items.end())
      {
        return;
      }

      const auto credits = SalvageValue(*it);
      SetDvar("sd_msg", std::format("Salvaged {}: ^3+{} CP", Title(*it), credits));
      Data.credits += credits;
      Data.items.erase(it);
      Save();
    }

    // Salvages every variant of a rarity that is not equipped
    void SalvageAll(int rarity)
    {
      Load();
      int count = 0;
      int credits = 0;
      std::erase_if(Data.items, [&](const Item& item)
      {
        if (item.equipped || RarityIndex(item.rarity) != rarity)
        {
          return false;
        }
        ++count;
        credits += SalvageValue(item);
        return true;
      });

      Data.credits += credits;
      Save();
      SetDvar("sd_msg", count ? std::format("Salvaged {} {} variants: ^3+{} CP", count, Utils::String::ToLower(RARITY_NAMES[rarity]), credits)
        : std::string("Nothing to salvage"));
      PublishArmory();
    }

    // --- Create a Class: the variant of a weapon (sd_cac_ref) -----------------------------------------------------------

    // A row of the Create a Class step: a variant (id), an added weapon bought (added), or neither (no variant)
    struct CacSlot
    {
      int id = 0;
      std::string added;
    };
    std::vector<CacSlot> CacSlots;

    bool Owns(const std::string& added)
    {
      return std::ranges::find(Data.owned, added) != Data.owned.end();
    }

    std::string ReplacementFor(const std::string& base)
    {
      const auto it = Data.replace.find(base);
      if (it == Data.replace.end() || !Owns(it->second) || !FindAdded(it->second))
      {
        return {};
      }
      return it->second;
    }

    void PublishCac()
    {
      Load();
      const auto ref = Dvar::Var("sd_cac_ref").get<std::string>();
      const auto replacement = ReplacementFor(ref);

      CacSlots = { CacSlot{} };
      for (const auto* item : VariantsOf(ref))
      {
        if (static_cast<int>(CacSlots.size()) >= CAC_ROWS) break;
        CacSlots.push_back({ item->id, {} });
      }

      const auto* equipped = EquippedFor(ref);
      for (int i = 0; i < CAC_ROWS; ++i)
      {
        const auto p = std::format("sd_cv{}", i);
        if (i >= static_cast<int>(CacSlots.size()))
        {
          SetDvar(p + "_name", "");
          continue;
        }

        const auto& slot = CacSlots[i];
        if (!slot.added.empty())
        {
          SetDvar(p + "_name", std::format("^3{} ^7- IN ITS PLACE", Utils::String::ToUpper(WeaponName(slot.added))));
          SetDvar(p + "_eq", replacement == slot.added ? 1 : 0);
          continue;
        }

        if (slot.id == 0)
        {
          SetDvar(p + "_name", "NO VARIANT");
          SetDvar(p + "_eq", (equipped || !replacement.empty()) ? 0 : 1);
          continue;
        }

        const auto* item = FindItem(slot.id);
        SetDvar(p + "_name", std::format("{}{} ^7- {}", RARITY_COLORS[RarityIndex(item->rarity)], Utils::String::ToUpper(item->name), RARITY_NAMES[RarityIndex(item->rarity)]));
        SetDvar(p + "_eq", (item->equipped && replacement.empty()) ? 1 : 0);
      }

      SetDvar("sd_cv_count", static_cast<int>(CacSlots.size()));
      if (!replacement.empty())
      {
        PublishCard("sd_cf", nullptr, replacement);
      }
      else
      {
        PublishCard("sd_cf", equipped, ref);
      }
    }

    // --- Store: the added weapons -------------------------------------------------------------------------------------

    constexpr int STORE_ROWS = 10;
    int StorePage = 0;
    int StoreFocus = 0;

    void PublishStore()
    {
      Load();
      if (!AddedLoaded) LoadAddedWeapons();
      const auto pages = std::max(1, static_cast<int>((AddedWeapons.size() + STORE_ROWS - 1) / STORE_ROWS));
      StorePage = ((StorePage % pages) + pages) % pages;

      for (int i = 0; i < STORE_ROWS; ++i)
      {
        const auto p = std::format("sd_sw{}", i);
        const auto index = StorePage * STORE_ROWS + i;
        if (index >= static_cast<int>(AddedWeapons.size()))
        {
          SetDvar(p + "_name", "");
          continue;
        }
        const auto& weapon = AddedWeapons[index];
        SetDvar(p + "_name", Utils::String::ToUpper(weapon.name));
        SetDvar(p + "_tag", Owns(weapon.ref) ? "^2OWNED" : std::format("{} CP", weapon.price));
      }

      SetDvar("sd_sw_count", static_cast<int>(AddedWeapons.size()));
      SetDvar("sd_sw_pages", pages);
      SetDvar("sd_sw_page", std::format("{} / {}", StorePage + 1, pages));
      PublishMain();
    }

    const AddedWeapon* StoreAt(int slot)
    {
      const auto index = StorePage * STORE_ROWS + slot;
      if (slot < 0 || slot >= STORE_ROWS || index >= static_cast<int>(AddedWeapons.size())) return nullptr;
      return &AddedWeapons[index];
    }

    void PublishStoreFocus()
    {
      const auto* weapon = StoreAt(StoreFocus);
      if (!weapon) return;
      PublishCard("sd_sf", nullptr, weapon->ref);
      SetDvar("sd_sf_price", weapon->price);
      SetDvar("sd_sf_owned", Owns(weapon->ref) ? 1 : 0);
      SetDvar("sd_sf_category", weapon->category == "sniper" ? "SNIPER RIFLE" : Utils::String::ToUpper(weapon->category));
    }

    // ------------------------------------------------------------------------------------------------------------
    // In a match
    // ------------------------------------------------------------------------------------------------------------

    std::string Award(int drops, int advanced)
    {
      Load();
      drops = std::max(0, drops);
      advanced = std::max(0, advanced);
      Data.drops += drops;
      Data.advanced += advanced;
      Save();

      std::string text;
      if (drops > 0) text += std::format("+{} Supply Drop", drops);
      if (advanced > 0) text += std::format("{}+{} Advanced Supply Drop", text.empty() ? "" : ", ", advanced);
      return text;
    }

    // Progress on the contracts; returns what was earned ("" if nothing)
    std::string Progress(const std::string& type, int amount)
    {
      Load();
      RefreshContracts();

      std::string earned;
      auto step = [&](std::vector<Contract>& list, bool weekly)
      {
        for (auto& c : list)
        {
          if (c.type != type || c.done)
          {
            continue;
          }

          c.progress += amount;
          if (c.progress >= c.target)
          {
            c.progress = c.target;
            c.done = true;
            const auto reward = weekly ? Award(0, Cfg("weekly_contract_advanced")) : Award(1, 0);
            earned += std::format("{}{} contract complete: {}", earned.empty() ? "" : "\n", weekly ? "Weekly" : "Daily", reward);
          }
        }
      };

      step(Data.daily, false);
      step(Data.weekly, true);
      Save();
      return earned;
    }

    void AddScriptFunctions()
    {
      GSC::Script::AddFunction("SDMod", [] // gsc: SDMod(<weapon ref>, <type>) -> percent of the equipped variant, 0 if none
      {
        Load();
        const std::string ref = Game::Scr_GetString(0);
        const std::string type = Game::Scr_GetString(1);
        const auto* item = EquippedFor(ref);
        Game::Scr_AddInt(item ? item->Mod(type) : 0);
      });

      GSC::Script::AddFunction("SDOwns", [] // gsc: SDOwns(<weapon ref>) -> 1 if you have a variant of it (it then counts as unlocked)
      {
        Load();
        Game::Scr_AddInt(CountFor(Game::Scr_GetString(0)) > 0 ? 1 : 0);
      });

      GSC::Script::AddFunction("SDReplacement", [] // gsc: SDReplacement(<stock weapon ref>) -> the added weapon used in its place, "" if none
      {
        Load();
        const auto text = ReplacementFor(Game::Scr_GetString(0));
        Game::Scr_AddString(text.data());
      });

      GSC::Script::AddFunction("SDOwnsWeapon", [] // gsc: SDOwnsWeapon(<store weapon ref>) -> 1 if bought
      {
        Load();
        Game::Scr_AddInt(Owns(Game::Scr_GetString(0)) ? 1 : 0);
      });

      GSC::Script::AddFunction("SDAddedWeapons", [] // gsc: SDAddedWeapons() -> the installed added weapons, "hdr ax50 ..."
      {
        LoadAddedWeapons();
        std::string text;
        for (const auto& weapon : AddedWeapons)
        {
          text += (text.empty() ? "" : " ") + weapon.ref;
        }
        Game::Scr_AddString(text.data());
      });

      GSC::Script::AddFunction("SDVariantName", [] // gsc: SDVariantName(<weapon ref>) -> "M4A1 'Ember' (Elite)" or ""
      {
        Load();
        const std::string ref = Game::Scr_GetString(0);
        const auto* item = EquippedFor(ref);
        const auto text = item ? std::format("{}{}^7 ({})", RARITY_COLORS[RarityIndex(item->rarity)], Title(*item), RARITY_NAMES[RarityIndex(item->rarity)]) : std::string();
        Game::Scr_AddString(text.data());
      });

      GSC::Script::AddFunction("SDAward", [] // gsc: SDAward(<drops>, <advanced drops>) -> "+1 Supply Drop"
      {
        const auto text = Award(Game::Scr_GetInt(0), Game::Scr_GetInt(1));
        Game::Scr_AddString(text.data());
      });

      GSC::Script::AddFunction("SDAddCredits", [] // gsc: SDAddCredits(<credits>)
      {
        Load();
        Data.credits += Game::Scr_GetInt(0);
        Save();
      });

      GSC::Script::AddFunction("SDProgress", [] // gsc: SDProgress(<contract type>, <amount>) -> what was earned, "" if nothing
      {
        const auto text = Progress(Game::Scr_GetString(0), Game::Scr_GetInt(1));
        Game::Scr_AddString(text.data());
      });

      GSC::Script::AddFunction("SDMinutePlayed", [] // gsc: SDMinutePlayed() -> 1 when it earned a Supply Drop
      {
        Load();
        ++Data.playtime;
        auto earned = 0;
        if (Data.playtime >= std::max(1, Cfg("minutes_per_drop")))
        {
          Data.playtime = 0;
          ++Data.drops;
          earned = 1;
        }
        Save();
        Game::Scr_AddInt(earned);
      });

      GSC::Script::AddFunction("SDFirstWin", [] // gsc: SDFirstWin() -> drops earned by the first win of the day (0 if not the first)
      {
        Load();
        const auto today = LocalTime("%Y-%m-%d");
        auto earned = 0;
        if (Data.winday != today)
        {
          Data.winday = today;
          earned = Cfg("first_win_of_day_drops");
          Data.drops += earned;
          Save();
        }
        Game::Scr_AddInt(earned);
      });

      GSC::Script::AddFunction("SDConfig", [] // gsc: SDConfig(<setting>) -> its value (supply_drops/config.txt)
      {
        Game::Scr_AddInt(Cfg(Utils::String::ToLower(Game::Scr_GetString(0))));
      });

      GSC::Script::AddFunction("SDCount", [] // gsc: SDCount() -> Supply Drops + Advanced Supply Drops waiting to be opened
      {
        Load();
        Game::Scr_AddInt(Data.drops + Data.advanced);
      });
    }

    int ParamInt(const Command::Params* params, int index, int fallback = 0)
    {
      return params->size() > index ? std::atoi(params->get(index)) : fallback;
    }
  }

  SupplyDrops::SupplyDrops()
  {
    if (Dedicated::IsEnabled() || ZoneBuilder::IsEnabled())
    {
      return;
    }

    // The menus of Advanced Combat Training (new menus: they have to be registered to load in the main menu)
    Menus::Add("ui_mp/sd_main.menu");
    Menus::Add("ui_mp/sd_reveal.menu");
    Menus::Add("ui_mp/sd_armory.menu");
    Menus::Add("ui_mp/sd_variants.menu");
    Menus::Add("ui_mp/sd_item_popup.menu");
    Menus::Add("ui_mp/sd_store.menu");
    Menus::Add("ui_mp/sd_buy_popup.menu");
    Menus::Add("ui_mp/popup_cac_store_primary.menu");
    Menus::Add("ui_mp/popup_sd_variant.menu");
    // Create a Class popups of the stock game, rebuilt with the variant step
    Menus::Add("ui_mp/popup_cac_lmg_primary.menu");
    Menus::Add("ui_mp/popup_cac_shotgun_secondary.menu");
    Menus::Add("ui_mp/popup_cac_machinepistols_secondary.menu");

    AddScriptFunctions();

    // Create a Class saves a class's weapon as one of a closed list of the stats (mp/playerdata.def). As IW4x does for
    // its own weapons, the store weapons are added to that list (within the room the list keeps for new entries), so
    // that a class can hold them. Only with Advanced Combat Training loaded; the stats layout does not change.
    AssetHandler::OnLoad([](Game::XAssetType type, Game::XAssetHeader asset, const std::string& filename, bool* /*restrict*/)
    {
      if (type != Game::ASSET_TYPE_STRUCTURED_DATA_DEF || filename != "mp/playerdata.def" || !asset.structuredDataDefSet)
      {
        return;
      }

      const std::string fsGame = (*Game::fs_gameDirVar) ? (*Game::fs_gameDirVar)->current.string : "";
      if (fsGame.find("advanced_combat_training") == std::string::npos)
      {
        return;
      }

      FileSystem::File file("supply_drops/weapons.txt", Game::FsThread::FS_THREAD_DATABASE);
      if (!file.exists())
      {
        return;
      }

      auto* def = &asset.structuredDataDefSet->defs[0];
      auto* weapons = &def->enums[StructuredData::PlayerDataType::WEAPONS];

      std::vector<std::string> entries;
      for (auto& line : Utils::String::Split(file.getBuffer(), '\n'))
      {
        Utils::String::Replace(line, "\r", "");
        Utils::String::Trim(line);
        if (line.empty() || line.starts_with("//") || line.starts_with("#")) continue;
        auto ref = Utils::String::Split(line, '|')[0];
        Utils::String::Trim(ref);
        ref = Utils::String::ToLower(ref);

        bool present = false;
        for (int i = 0; i < weapons->entryCount; ++i)
        {
          if (ref == weapons->entries[i].string) present = true;
        }
        if (!present && !ref.empty()) entries.emplace_back(ref);
      }

      if (entries.empty())
      {
        return;
      }
      if (weapons->entryCount + static_cast<int>(entries.size()) > weapons->reservedEntryCount)
      {
        Logger::PrintError(Game::CON_CHANNEL_ERROR, "Supply Drops: no room for {} more weapons in the player data\n", entries.size());
        return;
      }

      StructuredData::PatchPlayerDataEnum(def, StructuredData::PlayerDataType::WEAPONS, entries);
      Logger::Print("Supply Drops: {} store weapons added to the class data\n", entries.size());
    });

    Command::Add("sd_refresh", []
    {
      Loaded = false; // the settings or the save file may have been changed by hand
      Config.clear();
      PublishMain();
      SetDvar("sd_msg", "");
    });

    Command::Add("sd_open", [](const Command::Params* params)
    {
      OpenDrop(ParamInt(params, 1) != 0);
    });

    Command::Add("sd_reveal_skip", [] // straight to the end of the reveal
    {
      RevealStart = Game::Sys_Milliseconds() - RevealEnd - 1;
      RevealActive = true;
      UpdateReveal();
    });

    // The reveal animation, every frame while it plays
    Scheduler::Loop(UpdateReveal, Scheduler::Pipeline::CLIENT);

    Command::Add("sd_buy", [](const Command::Params* params)
    {
      Buy(ParamInt(params, 1) != 0);
    });

    Command::Add("sd_armory", [](const Command::Params* params) // sd_armory [page change: 1 / -1 / 0]
    {
      if (params->size() < 2)
      {
        ArmoryPage = 0;
        ArmoryFocus = 0;
        SetDvar("sd_msg", "");
      }
      else
      {
        ArmoryPage += ParamInt(params, 1);
      }
      PublishArmory();
    });

    Command::Add("sd_afocus", [](const Command::Params* params) // sd_afocus <armory row>
    {
      ArmoryFocus = ParamInt(params, 1);
      PublishArmoryFocus();
    });

    Command::Add("sd_weapon", [](const Command::Params* params) // sd_weapon <armory row>
    {
      const auto slot = ParamInt(params, 1, -1);
      if (slot < 0 || slot >= static_cast<int>(ArmorySlots.size()))
      {
        return;
      }
      VariantWeapon = ArmorySlots[slot];
      VariantPage = 0;
      VariantFocus = 0;
      SetDvar("sd_msg", "");
      PublishVariants();
      PublishCard("sd_f", VariantAt(0), VariantWeapon);
    });

    Command::Add("sd_variants", [](const Command::Params* params) // sd_variants <page change>
    {
      VariantPage += ParamInt(params, 1);
      PublishVariants();
      PublishCard("sd_f", VariantAt(0), VariantWeapon);
    });

    Command::Add("sd_focus", [](const Command::Params* params) // sd_focus <variant row>
    {
      VariantFocus = ParamInt(params, 1, -1);
      PublishCard("sd_f", VariantAt(VariantFocus), VariantWeapon);
    });

    Command::Add("sd_equip", [](const Command::Params* params) // sd_equip [variant row, the one in focus if none]: equips it, or unequips it
    {
      const auto slot = ParamInt(params, 1, VariantFocus);
      if (auto* item = VariantAt(slot))
      {
        const auto equip = !item->equipped;
        const auto title = Title(*item);
        Equip(*item, equip);
        SetDvar("sd_msg", equip ? std::format("^2Equipped:^7 {}", title) : std::format("Unequipped: {}", title));
        PublishVariants();
        PublishCard("sd_f", VariantAt(slot), VariantWeapon);
        PublishMain();
      }
    });

    Command::Add("sd_salvage", [](const Command::Params* params) // sd_salvage [variant row, the one in focus if none]
    {
      const auto slot = ParamInt(params, 1, VariantFocus);
      if (const auto* item = VariantAt(slot))
      {
        Salvage(item->id);
        PublishVariants();
        PublishCard("sd_f", VariantAt(std::min(slot, static_cast<int>(VariantSlots.size()) - 1)), VariantWeapon);
        PublishMain();
      }
    });

    Command::Add("sd_salvage_all", [](const Command::Params* params) // sd_salvage_all <rarity 0..2>
    {
      SalvageAll(std::clamp(ParamInt(params, 1), 0, 2));
    });

    Command::Add("sd_cac", [] // the variants of sd_cac_ref, for Create a Class
    {
      PublishCac();
    });

    Command::Add("sd_cfocus", [](const Command::Params* params) // sd_cfocus <row>
    {
      const auto slot = ParamInt(params, 1, -1);
      const auto ref = Dvar::Var("sd_cac_ref").get<std::string>();
      if (slot < 0 || slot >= static_cast<int>(CacSlots.size()))
      {
        return;
      }
      const auto& row = CacSlots[slot];
      if (!row.added.empty())
      {
        PublishCard("sd_cf", nullptr, row.added);
      }
      else
      {
        PublishCard("sd_cf", row.id ? FindItem(row.id) : nullptr, ref);
      }
    });

    Command::Add("sd_cpick", [](const Command::Params* params) // sd_cpick <row>: for the weapon in all classes
    {
      Load();
      const auto slot = ParamInt(params, 1, -1);
      const auto ref = Dvar::Var("sd_cac_ref").get<std::string>();
      if (slot < 0 || slot >= static_cast<int>(CacSlots.size()))
      {
        return;
      }

      const auto row = CacSlots[slot];
      if (!row.added.empty())
      {
        Data.replace[ref] = row.added; // the added weapon in its place; its variants stay as they were
        Save();
      }
      else
      {
        Data.replace.erase(ref);
        if (row.id == 0)
        {
          for (auto& item : Data.items)
          {
            if (item.base == ref) item.equipped = false;
          }
          Save();
        }
        else if (auto* item = FindItem(row.id))
        {
          Equip(*item, true);
        }
        else
        {
          Save();
        }
      }
      PublishMain();
    });

    Command::Add("sd_store", [](const Command::Params* params) // sd_store [page change]
    {
      if (params->size() < 2)
      {
        StorePage = 0;
        StoreFocus = 0;
        SetDvar("sd_msg", "");
        LoadAddedWeapons();
      }
      else
      {
        StorePage += ParamInt(params, 1);
      }
      PublishStore();
      PublishStoreFocus();
    });

    Command::Add("sd_sfocus", [](const Command::Params* params) // sd_sfocus <row>
    {
      StoreFocus = ParamInt(params, 1);
      PublishStoreFocus();
    });

    Command::Add("sd_sbuy", [] // buys the weapon in focus in the Store
    {
      Load();
      const auto* weapon = StoreAt(StoreFocus);
      if (!weapon || Owns(weapon->ref))
      {
        return;
      }
      if (Data.credits < weapon->price)
      {
        SetDvar("sd_msg", std::format("^1Not enough COD Points^7 ({} / {})", Data.credits, weapon->price));
        return;
      }
      Data.credits -= weapon->price;
      Data.owned.emplace_back(weapon->ref);
      Save();
      SetDvar("sd_msg", std::format("^2Purchased:^7 {}. Choose it in Create a Class, in STORE WEAPONS.", weapon->name));
      PublishStore();
      PublishStoreFocus();
    });
  }
}
