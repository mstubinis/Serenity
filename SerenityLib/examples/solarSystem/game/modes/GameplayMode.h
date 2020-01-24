#pragma once
#ifndef GAME_GAMEPLAYMODE_H
#define GAME_GAMEPLAYMODE_H

#include <unordered_map>
#include <unordered_set>
#include "../teams/TeamIncludes.h"

struct GameplayModeType final {enum Mode {
    FFA,
    TeamDeathmatch,
    HomelandSecurity,
_TOTAL};};

class  Team;
struct PacketGameplayModeInfo;
class  ServerHostData;
class GameplayMode {
    friend class ServerHostData;
    protected:
        GameplayModeType::Mode                           m_GameplayModeType;
        unsigned int                                     m_MaxAmountOfPlayers;
        std::unordered_map<TeamNumber::Enum, Team>       m_Teams;
        std::unordered_set<std::string>                  m_AllowedShipClasses;
    public:
        static std::vector<std::string>                  GAMEPLAY_TYPE_ENUM_NAMES;
    public:
        GameplayMode();
        GameplayMode(const GameplayModeType::Mode& mode, const unsigned int MaxAmountOfPlayers);
        virtual ~GameplayMode();

        void clear();

        void setGameplayMode(const GameplayModeType::Mode& mode);
        void setMaxAmountOfPlayers(const unsigned int& maxPlayers);

        const bool addAllowedShipClass(const std::string& shipClass);

        std::unordered_map<TeamNumber::Enum, Team>& getTeams();
        Team* getTeam(const TeamNumber::Enum& teamNumberEnum);
        const GameplayModeType::Mode& getGameplayMode() const;
        const unsigned int& getMaxAmountOfPlayers() const;
        const bool addTeam(Team& team);

        const PacketGameplayModeInfo serialize() const;
        void deserialize(const PacketGameplayModeInfo&);

};


#endif