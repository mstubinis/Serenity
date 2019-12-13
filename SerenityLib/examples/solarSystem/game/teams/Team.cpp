#include "Team.h"
#include <string>

using namespace std;

Team::Team(const TeamNumber::Enum& thisTeamNumber) {
    m_TeamNumber = thisTeamNumber;
}
Team::Team(const TeamNumber::Enum& thisTeamNumber, vector<TeamNumber::Enum>& allyTeams, vector<TeamNumber::Enum>& enemyTeams) {
    m_TeamNumber = thisTeamNumber;

    for (auto& allyTeam : allyTeams)
        addAllyTeam(allyTeam);
    for (auto& enemyTeam : enemyTeams)
        addEnemyTeam(enemyTeam);
}
Team::~Team() {

}

const TeamNumber::Enum& Team::getTeamNumber() const {
    return m_TeamNumber;
}
const size_t Team::getNumberOfPlayersOnTeam() const {
    return m_TeamPlayers.size();
}
string Team::getTeamNumberAsString() const {
    return to_string(static_cast<unsigned int>(m_TeamNumber));
}
unordered_set<string>& Team::getPlayers() {
    return m_TeamPlayers;
}
unordered_set<TeamNumber::Enum>& Team::getAllyTeams() {
    return m_AllyTeams;
}
unordered_set<TeamNumber::Enum>& Team::getEnemyTeams() {
    return m_EnemyTeams;
}

const bool Team::isPlayerOnTeam(const string& playerMapKey) const {
    if (m_TeamPlayers.size() > 0 && m_TeamPlayers.count(playerMapKey))
        return true;
    return false;
}
const bool Team::addPlayerToTeam(const string& playerMapKey) {
    if (m_TeamPlayers.size() > 0 && m_TeamPlayers.count(playerMapKey))
        return false;
    m_TeamPlayers.insert(playerMapKey);
    return true;
}
const bool Team::addEnemyTeam(const TeamNumber::Enum& teamNumber) {
    if (m_EnemyTeams.size() > 0 && (teamNumber == m_TeamNumber || m_EnemyTeams.count(teamNumber))) {
        return false;
    }
    m_EnemyTeams.insert(teamNumber);
    return true;
}
const bool Team::addAllyTeam(const TeamNumber::Enum& teamNumber) {
    if (m_AllyTeams.size() > 0 && (teamNumber == m_TeamNumber || m_AllyTeams.count(teamNumber))) {
        return false;
    }
    m_AllyTeams.insert(teamNumber);
    return true;
}

const bool Team::isEnemyTeam(const TeamNumber::Enum& otherTeamNumber) const {
    if (m_TeamNumber == TeamNumber::Team_FFA || otherTeamNumber == TeamNumber::Team_FFA)
        return true;
    if (m_TeamNumber == otherTeamNumber)
        return false;
    return (m_EnemyTeams.count(otherTeamNumber)) ? true : false;
}
const bool Team::isAllyTeam(const TeamNumber::Enum& otherTeamNumber) const {
    if (m_TeamNumber == TeamNumber::Team_FFA || otherTeamNumber == TeamNumber::Team_FFA)
        return false;
    if (m_TeamNumber == otherTeamNumber)
        return true;
    return (m_AllyTeams.count(otherTeamNumber)) ? true : false;
}
const bool Team::isNeutralTeam(const TeamNumber::Enum& otherTeamNumber) const {
    return (!isEnemyTeam(otherTeamNumber) && !isAllyTeam(otherTeamNumber)) ? true : false;
}

const bool Team::isEnemyTeam(const Team& otherTeam) const { return isEnemyTeam(otherTeam.getTeamNumber()); }
const bool Team::isAllyTeam(const Team& otherTeam) const { return isAllyTeam(otherTeam.getTeamNumber()); }
const bool Team::isNeutralTeam(const Team& otherTeam) const { return isNeutralTeam(otherTeam.getTeamNumber()); }