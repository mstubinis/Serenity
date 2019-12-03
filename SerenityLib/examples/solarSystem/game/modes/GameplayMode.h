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
};};

class  Team;
struct PacketGameplayModeInfo;
class GameplayMode {
    protected:
        GameplayModeType::Mode                           m_GameplayModeType;
        unsigned int                                     m_MaxAmountOfPlayers;
        std::unordered_map<TeamNumber::Enum, Team*>      m_Teams;
        std::unordered_set<std::string>                  m_AllowedShipClasses;
    public:
        GameplayMode();
        GameplayMode(const GameplayModeType::Mode& mode, const unsigned int MaxAmountOfPlayers);
        virtual ~GameplayMode();

        const bool addAllowedShipClass(const std::string& shipClass);

        std::unordered_map<TeamNumber::Enum, Team*>& getTeams();
        const GameplayModeType::Mode& getGameplayModeType() const;
        const unsigned int& getMaxAmountOfPlayers() const;
        const bool addTeam(Team& team);

        const PacketGameplayModeInfo serialize() const;
        void deserialize(const PacketGameplayModeInfo&);

};


#endif