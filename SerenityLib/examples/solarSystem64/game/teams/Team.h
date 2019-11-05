#pragma once
#ifndef GAME_TEAM_H
#define GAME_TEAM_H

#include "TeamIncludes.h"
#include <unordered_set>

class Team {
    private:
        TeamNumber::Enum                       m_TeamNumber;
        std::unordered_set<TeamNumber::Enum>   m_AllyTeams;
        std::unordered_set<TeamNumber::Enum>   m_EnemyTeams;
        std::unordered_set<std::string>        m_TeamPlayers;
    public:
        Team(const TeamNumber::Enum& thisTeamNumber);
        Team(const TeamNumber::Enum& thisTeamNumber, std::vector<TeamNumber::Enum>& allyTeams, std::vector<TeamNumber::Enum>& enemyTeams);
        ~Team();

        const TeamNumber::Enum& getTeamNumber() const;
        std::string getTeamNumberAsString() const;
        const unsigned int getNumberOfPlayersOnTeam() const;

        std::unordered_set<std::string>& getPlayers();
        std::unordered_set<TeamNumber::Enum>&   getAllyTeams();
        std::unordered_set<TeamNumber::Enum>&   getEnemyTeams();

        const bool addEnemyTeam(const TeamNumber::Enum& teamNumber);
        const bool addAllyTeam(const TeamNumber::Enum& teamNumber);

        const bool isEnemyTeam(const TeamNumber::Enum& otherTeamNumber) const;
        const bool isAllyTeam(const TeamNumber::Enum& otherTeamNumber) const;
        const bool isNeutralTeam(const TeamNumber::Enum& otherTeamNumber) const;

        const bool isEnemyTeam(const Team& otherTeam) const;
        const bool isAllyTeam(const Team& otherTeam) const;
        const bool isNeutralTeam(const Team& otherTeam) const;


        const bool isPlayerOnTeam(const std::string& playerName) const;
        const bool addPlayerToTeam(const std::string& playerName);
};


#endif