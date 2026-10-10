#include "SupplyDrops.hpp"
#include "Events.hpp"
#include "Menus.hpp"

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
    };

    // In the order of Create a Class
    constexpr WeaponInfo WEAPONS[] =
    {
      { "m4", "M4A1", "assault" }, { "famas", "FAMAS", "assault" }, { "scar", "SCAR-H", "assault" },
      { "tavor", "TAR-21", "assault" }, { "fal", "FAL", "assault" }, { "m16", "M16A4", "assault" },
      { "masada", "ACR", "assault" }, { "fn2000", "F2000", "assault" }, { "ak47", "AK-47", "assault" },
      { "ak47classic", "AK-47 Classic", "assault" },
      { "mp5k", "MP5K", "smg" }, { "ump45", "UMP45", "smg" }, { "kriss", "Vector", "smg" }, { "p90", "P90", "smg" },
      { "uzi", "Mini-Uzi", "smg" }, { "ak74u", "AK-74u", "smg" }, { "peacekeeper", "Peacekeeper", "smg" },
      { "sa80", "L86 LSW", "lmg" }, { "rpd", "RPD", "lmg" }, { "mg4", "MG4", "lmg" }, { "aug", "AUG HBAR", "lmg" },
      { "m240", "M240", "lmg" },
      { "cheytac", "Intervention", "sniper" }, { "barrett", "Barrett .50cal", "sniper" }, { "wa2000", "WA2000", "sniper" },
      { "m21", "M21 EBR", "sniper" }, { "m40a3", "M40A3", "sniper" }, { "dragunov", "Dragunov", "sniper" },
      { "pp2000", "PP2000", "mpistol" }, { "glock", "G18", "mpistol" }, { "beretta393", "M93 Raffica", "mpistol" },
      { "tmp", "TMP", "mpistol" },
      { "spas12", "SPAS-12", "shotgun" }, { "aa12", "AA-12", "shotgun" }, { "striker", "Striker", "shotgun" },
      { "ranger", "Ranger", "shotgun" }, { "m1014", "M1014", "shotgun" }, { "model1887", "Model 1887", "shotgun" },
      { "usp", "USP .45", "pistol" }, { "coltanaconda", ".44 Magnum", "pistol" }, { "beretta", "M9", "pistol" },
      { "deserteagle", "Desert Eagle", "pistol" },
    };

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

    // ------------------------------------------------------------------------------------------------------------
    // Settings: supply_drops/config.txt in the mod, "key = value" lines
    // ------------------------------------------------------------------------------------------------------------

    std::unordered_map<std::string, int> Config;

    void LoadConfig()
    {
      Config =
      {
        { "minutes_per_drop", 30 }, { "drops_per_level", 1 }, { "advanced_per_prestige", 2 }, { "first_win_of_day_drops", 1 },
        { "salvage_enlisted", 10 }, { "salvage_professional", 30 }, { "salvage_elite", 100 },
        { "match_credits_top", 15 }, { "match_credits_middle", 8 }, { "match_credits_bottom", 4 },
        { "price_drop", 120 }, { "price_advanced", 350 },
        { "items_per_drop", 3 }, { "bonus_credits_per_drop", 10 },
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
      const auto* weapon = FindWeapon(item.base);
      return std::format("{} '{}'", weapon ? weapon->name : item.base, item.name);
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
    void PublishCard(const std::string& p, const Item* item, const std::string& ref)
    {
      const auto* weapon = FindWeapon(item ? item->base : ref);
      SetDvar(p + "_ref", item ? item->base : ref);
      SetDvar(p + "_weapon", weapon ? Utils::String::ToUpper(weapon->name) : "");

      if (!item)
      {
        SetDvar(p + "_name", "Standard Issue");
        SetDvar(p + "_rarity", -1);
        SetDvar(p + "_rname", "BASE WEAPON");
        SetDvar(p + "_ddmg", 0);
        SetDvar(p + "_drng", 0);
        SetDvar(p + "_dmob", 0);
        SetDvar(p + "_mod0", "No changes to the weapon.");
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

      for (const auto& weapon : WEAPONS)
      {
        SetDvar(std::format("sd_own_{}", weapon.ref), CountFor(weapon.ref));
      }
    }

    // --- Opening -----------------------------------------------------------------------------------------------------

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

      SetDvar("sd_rv_ok", 1);
      SetDvar("sd_rv_count", static_cast<int>(items.size()));
      SetDvar("sd_rv_adv", advanced ? 1 : 0);
      SetDvar("sd_rv_title", advanced ? "ADVANCED SUPPLY DROP" : "SUPPLY DROP");
      SetDvar("sd_rv_best", best);
      SetDvar("sd_rv_bonus", std::format("+{} CREDITS", bonus));
      PublishMain();
    }

    void Buy(bool advanced)
    {
      Load();
      const auto price = Cfg(advanced ? "price_advanced" : "price_drop");
      if (Data.credits < price)
      {
        SetDvar("sd_msg", std::format("^1Not enough Credits^7 ({} / {})", Data.credits, price));
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
      SetDvar("sd_msg", std::format("Salvaged {}: ^3+{} Credits", Title(*it), credits));
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
      SetDvar("sd_msg", count ? std::format("Salvaged {} {} variants: ^3+{} Credits", count, Utils::String::ToLower(RARITY_NAMES[rarity]), credits)
        : std::string("Nothing to salvage"));
      PublishArmory();
    }

    // --- Create a Class: the variant of a weapon (sd_cac_ref) -----------------------------------------------------------

    std::vector<int> CacSlots; // item ids; 0 = no variant

    void PublishCac()
    {
      Load();
      const auto ref = Dvar::Var("sd_cac_ref").get<std::string>();
      const auto list = VariantsOf(ref);
      CacSlots = { 0 };
      for (const auto* item : list)
      {
        if (static_cast<int>(CacSlots.size()) >= CAC_ROWS) break;
        CacSlots.emplace_back(item->id);
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

        if (CacSlots[i] == 0)
        {
          SetDvar(p + "_name", "NO VARIANT");
          SetDvar(p + "_eq", equipped ? 0 : 1);
          continue;
        }

        const auto* item = FindItem(CacSlots[i]);
        SetDvar(p + "_name", std::format("{}{} ^7- {}", RARITY_COLORS[RarityIndex(item->rarity)], Utils::String::ToUpper(item->name), RARITY_NAMES[RarityIndex(item->rarity)]));
        SetDvar(p + "_eq", item->equipped ? 1 : 0);
      }

      SetDvar("sd_cv_count", static_cast<int>(CacSlots.size()));
      PublishCard("sd_cf", equipped, ref);
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
    Menus::Add("ui_mp/popup_sd_variant.menu");
    // Create a Class popups of the stock game, rebuilt with the variant step
    Menus::Add("ui_mp/popup_cac_lmg_primary.menu");
    Menus::Add("ui_mp/popup_cac_shotgun_secondary.menu");
    Menus::Add("ui_mp/popup_cac_machinepistols_secondary.menu");

    AddScriptFunctions();

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
      PublishCard("sd_cf", CacSlots[slot] ? FindItem(CacSlots[slot]) : nullptr, ref);
    });

    Command::Add("sd_cpick", [](const Command::Params* params) // sd_cpick <row>: the variant for the weapon in all classes
    {
      Load();
      const auto slot = ParamInt(params, 1, -1);
      const auto ref = Dvar::Var("sd_cac_ref").get<std::string>();
      if (slot < 0 || slot >= static_cast<int>(CacSlots.size()))
      {
        return;
      }

      if (CacSlots[slot] == 0)
      {
        for (auto& item : Data.items)
        {
          if (item.base == ref) item.equipped = false;
        }
        Save();
      }
      else if (auto* item = FindItem(CacSlots[slot]))
      {
        Equip(*item, true);
      }
      PublishMain();
    });
  }
}
