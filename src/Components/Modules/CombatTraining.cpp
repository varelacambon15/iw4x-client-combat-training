#include "CombatTraining.hpp"
#include "Events.hpp"
#include "Menus.hpp"

namespace Components
{
  Dvar::Var CombatTraining::Active;
  Dvar::Var CombatTraining::CurrentPlaylist;
  Dvar::Var CombatTraining::NextMap;
  Dvar::Var CombatTraining::NextGametype;
  Dvar::Var CombatTraining::Quit;

  std::vector<std::string> CombatTraining::RecentMaps;

  namespace
  {
    // How many of the last maps played are left out when the next one is picked
    constexpr std::size_t RECENT_MAPS = 4;

    std::mt19937& Rng()
    {
      static std::mt19937 gen(std::random_device{}());
      return gen;
    }

    std::vector<std::string> Words(const std::string& text)
    {
      std::vector<std::string> out;
      for (auto& word : Utils::String::Split(text, ' '))
      {
        Utils::String::Trim(word);
        if (!word.empty())
        {
          out.emplace_back(word);
        }
      }
      return out;
    }
  }

  bool CombatTraining::IsActive()
  {
    return Active.get<bool>();
  }

  // combat_training/playlists.txt: one playlist per line,
  //   id | name | game modes | players | hardcore (0/1) | maps
  // A game mode written twice is picked twice as often. Lines starting with '#' are comments.
  std::vector<CombatTraining::Playlist> CombatTraining::LoadPlaylists()
  {
    std::vector<Playlist> result;

    FileSystem::File file("combat_training/playlists.txt");
    if (!file.exists())
    {
      Logger::PrintError(Game::CON_CHANNEL_ERROR, "Combat Training: combat_training/playlists.txt not found (is the mod loaded?)\n");
      return result;
    }

    for (auto& line : Utils::String::Split(file.getBuffer(), '\n'))
    {
      Utils::String::Replace(line, "\r", "");
      Utils::String::Trim(line);
      if (line.empty() || line[0] == '#')
      {
        continue;
      }

      auto fields = Utils::String::Split(line, '|');
      if (fields.size() < 6)
      {
        Logger::Warning(Game::CON_CHANNEL_SERVER, "Combat Training: ignoring playlist line '{}'\n", line);
        continue;
      }

      for (auto& field : fields)
      {
        Utils::String::Trim(field);
      }

      Playlist playlist;
      playlist.id = Utils::String::ToLower(fields[0]);
      playlist.name = fields[1];
      playlist.gametypes = Words(fields[2]);
      playlist.players = std::clamp(std::atoi(fields[3].data()), 2, 18);
      playlist.hardcore = fields[4] == "1";
      // Only the maps that are installed (DLC maps may be missing)
      for (const auto& map : Words(fields[5]))
      {
        if (Maps::CheckMapInstalled(map))
        {
          playlist.maps.emplace_back(map);
        }
      }

      if (playlist.id.empty() || playlist.gametypes.empty() || playlist.maps.empty())
      {
        Logger::Warning(Game::CON_CHANNEL_SERVER, "Combat Training: playlist '{}' has no modes or no maps\n", playlist.id);
        continue;
      }

      result.emplace_back(std::move(playlist));
    }

    return result;
  }

  bool CombatTraining::FindPlaylist(const std::string& id, Playlist& out)
  {
    const auto wanted = Utils::String::ToLower(id);
    for (auto& playlist : LoadPlaylists())
    {
      if (playlist.id == wanted)
      {
        out = std::move(playlist);
        return true;
      }
    }

    return false;
  }

  std::string CombatTraining::PickGametype(const Playlist& playlist)
  {
    std::uniform_int_distribution<std::size_t> dist(0, playlist.gametypes.size() - 1);
    return playlist.gametypes[dist(Rng())];
  }

  std::string CombatTraining::PickMap(const Playlist& playlist, const std::string& current)
  {
    std::vector<std::string> candidates;
    for (const auto& map : playlist.maps)
    {
      const auto recent = std::ranges::find(RecentMaps, map) != RecentMaps.end();
      if (map != current && !recent)
      {
        candidates.emplace_back(map);
      }
    }

    // A short playlist: anything but the map being played
    if (candidates.empty())
    {
      for (const auto& map : playlist.maps)
      {
        if (map != current)
        {
          candidates.emplace_back(map);
        }
      }
    }

    if (candidates.empty())
    {
      return playlist.maps.front();
    }

    std::uniform_int_distribution<std::size_t> dist(0, candidates.size() - 1);
    return candidates[dist(Rng())];
  }

  void CombatTraining::RememberMap(const std::string& map)
  {
    RecentMaps.emplace_back(map);
    while (RecentMaps.size() > RECENT_MAPS)
    {
      RecentMaps.erase(RecentMaps.begin());
    }
  }

  // The match after this one, chosen now so that the scripts can show it at the end of the match
  void CombatTraining::QueueNext(const Playlist& playlist, const std::string& current)
  {
    NextGametype.set(PickGametype(playlist));
    NextMap.set(PickMap(playlist, current));
  }

  // The player ended the match from the pause menu (the scripts set ct_quit): back to the main menu, not the next match
  bool CombatTraining::ConsumeQuit()
  {
    if (!Quit.get<bool>())
    {
      return false;
    }

    Quit.set(false);
    Logger::Print(Game::CON_CHANNEL_SERVER, "Combat Training: the match was ended, back to the menu\n");
    Stop();
    return true;
  }

  void CombatTraining::PrepareRotation()
  {
    if (!IsActive())
    {
      return;
    }

    Playlist playlist;
    if (!FindPlaylist(CurrentPlaylist.get<std::string>(), playlist))
    {
      Logger::PrintError(Game::CON_CHANNEL_ERROR, "Combat Training: playlist '{}' not found, stopping\n", CurrentPlaylist.get<std::string>());
      Active.set(false);
      return;
    }

    auto gametype = NextGametype.get<std::string>();
    auto map = NextMap.get<std::string>();
    const std::string current = (*Game::sv_mapname)->current.string;

    if (gametype.empty() || map.empty())
    {
      gametype = PickGametype(playlist);
      map = PickMap(playlist, current);
    }

    RememberMap(map);
    QueueNext(playlist, map);

    Logger::Print(Game::CON_CHANNEL_SERVER, "Combat Training: next match {} on {}\n", gametype, map);
    Game::Dvar_SetString(*Game::sv_mapRotationCurrent, std::format("gametype {} map {}", gametype, map).data());
  }

  void CombatTraining::Start(const std::string& id)
  {
    Playlist playlist;
    if (!FindPlaylist(id, playlist))
    {
      Logger::PrintError(Game::CON_CHANNEL_ERROR, "Combat Training: unknown playlist '{}'\n", id);
      return;
    }

    RecentMaps.clear();
    Quit.set(false);
    const auto gametype = PickGametype(playlist);
    const auto map = PickMap(playlist, "");
    RememberMap(map);

    Active.set(true);
    CurrentPlaylist.set(playlist.id);
    QueueNext(playlist, map);

    Logger::Print(Game::CON_CHANNEL_SERVER, "Combat Training: starting '{}' ({} on {})\n", playlist.name, gametype, map);

    // A local server that counts as an online, ranked game: the stock scripts then give experience, rank,
    // challenges and unlocks as on Xbox LIVE. The bots fill the match up to the playlist's size.
    std::string cmd;
    cmd += "set onlinegame 1;";
    cmd += "exec combat_training/match.cfg;"; // the online rules first (it runs default_xboxlive.cfg), then the playlist's
    cmd += "set scr_rankedmatch 1;";
    cmd += "set party_enable 0;";
    cmd += "set xblive_privateserver 0;";
    cmd += "set sv_randomBotNames 1;";
    cmd += std::format("set sv_maxclients {};", playlist.players);
    cmd += std::format("set party_maxplayers {};", playlist.players);
    cmd += std::format("set scr_hardcore {};", playlist.hardcore ? 1 : 0);
    cmd += std::format("set g_hardcore {};", playlist.hardcore ? 1 : 0);
    cmd += std::format("set ct_players {};", playlist.players);
    cmd += std::format("set ct_playlist_name \"{}\";", playlist.name);
    cmd += std::format("set g_gametype {};", gametype);
    cmd += std::format("map {}", map);

    Command::Execute(cmd, false);
  }

  void CombatTraining::Stop()
  {
    Active.set(false);
    NextMap.set("");
    NextGametype.set("");
    RecentMaps.clear();
    Command::Execute("disconnect", false);
  }

  CombatTraining::CombatTraining()
  {
    if (Dedicated::IsEnabled() || ZoneBuilder::IsEnabled())
    {
      return;
    }

    Events::OnDvarInit([]
    {
      Active = Dvar::Register<bool>("ct_active", false, Game::DVAR_NONE, "Combat Training is running");
      CurrentPlaylist = Dvar::Register<const char*>("ct_playlist", "", Game::DVAR_NONE, "Combat Training playlist being played");
      NextMap = Dvar::Register<const char*>("ct_nextmap", "", Game::DVAR_NONE, "Combat Training: map of the next match");
      NextGametype = Dvar::Register<const char*>("ct_nextgametype", "", Game::DVAR_NONE, "Combat Training: game mode of the next match");
      Quit = Dvar::Register<bool>("ct_quit", false, Game::DVAR_NONE, "Combat Training: the player ended the match, back to the menu instead of the next one");
    });

    // The playlist menu of the mod: a new menu, so it has to be registered to load in the main menu
    Menus::Add("ui_mp/ct_playlists.menu");

    Command::Add("ct_start", [](const Command::Params* params)
    {
      if (params->size() < 2)
      {
        Logger::Print("ct_start <playlist> : play a Combat Training playlist (combat_training/playlists.txt)\n");
        return;
      }

      Start(params->get(1));
    });

    Command::Add("ct_stop", []
    {
      Stop();
    });

    Command::Add("ct_playlists", []
    {
      for (const auto& playlist : LoadPlaylists())
      {
        Logger::Print("{} : {} ({} players, {} maps)\n", playlist.id, playlist.name, playlist.players, playlist.maps.size());
      }
    });

    // (the mod's main menu sets ct_active back to 0 when it opens: back in the menus, Combat Training is over)
  }
}
