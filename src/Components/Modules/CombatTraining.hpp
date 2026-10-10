#pragma once

namespace Components
{
  // Combat Training: the online multiplayer experience, played offline against bots.
  //
  // A playlist (combat_training/playlists.txt in the loaded mod) names the game modes, the maps and the size of the
  // match. `ct_start <playlist>` starts a local ranked server on one of its maps, and every map rotation after that
  // picks the next mode and map from the same playlist, never one of the last few maps played. The scripts of the mod
  // (scripts/mp/combat_training.gsc) fill the match with bots and give the experience, rank and unlocks.
  class CombatTraining : public Component
  {
  public:
    CombatTraining();

    [[nodiscard]] static bool IsActive();

    // Called by MapRotation before it rotates: queues the next mode and map of the playlist.
    static void PrepareRotation();

  private:
    struct Playlist
    {
      std::string id;
      std::string name;
      std::vector<std::string> gametypes;
      std::vector<std::string> maps;
      int players = 12;
      bool hardcore = false;
    };

    static Dvar::Var Active;
    static Dvar::Var CurrentPlaylist;
    static Dvar::Var NextMap;
    static Dvar::Var NextGametype;

    static std::vector<std::string> RecentMaps;

    static std::vector<Playlist> LoadPlaylists();
    static bool FindPlaylist(const std::string& id, Playlist& out);

    static std::string PickGametype(const Playlist& playlist);
    static std::string PickMap(const Playlist& playlist, const std::string& current);
    static void QueueNext(const Playlist& playlist, const std::string& current);
    static void RememberMap(const std::string& map);

    static void AddSupplyDropFunctions();

    static void Start(const std::string& id);
    static void Stop();
  };
}
