#include "GameplayMode.h"
#include "../teams/Team.h"
#include "../Helper.h"
#include "../ships/Ships.h"

#include <iostream>


using namespace std;

GameplayMode::GameplayMode() {
    m_GameplayModeType = GameplayModeType::FFA;
    m_MaxAmountOfPlayers = 0;
}
GameplayMode::GameplayMode(const GameplayModeType::Mode& mode, const unsigned int MaxAmountOfPlayers) {
    m_GameplayModeType = mode;
    m_MaxAmountOfPlayers = MaxAmountOfPlayers;
}
GameplayMode::~GameplayMode() {

}
unordered_map<TeamNumber::Enum, Team*>& GameplayMode::getTeams() {
    return m_Teams;
}
const GameplayModeType::Mode& GameplayMode::getGameplayModeType() const {
    return m_GameplayModeType;
}
const unsigned int& GameplayMode::getMaxAmountOfPlayers() const {
    return m_MaxAmountOfPlayers;
}
const bool GameplayMode::addTeam(Team& team) {
    if (m_Teams.count(team.getTeamNumber())) {
        return false;
    }
    m_Teams.emplace(team.getTeamNumber(), &team);
    return true;
}

const bool GameplayMode::addAllowedShipClass(const string& shipClass) {
    if (!Ships::Database.count(shipClass))
        return false;
    if (m_AllowedShipClasses.count(shipClass))
        return false;
    m_AllowedShipClasses.insert(shipClass);
    return true;
}

const string GameplayMode::serialize() const {
    string res = "";

    res +=       to_string(m_GameplayModeType);    //[0]
    res += "," + to_string(m_MaxAmountOfPlayers);  //[1]
    res += "," + to_string(m_Teams.size());        //[2]
    for (auto& team_itr : m_Teams) {
        auto& team = *team_itr.second;
        res += "," + team.getTeamNumberAsString();
        res += "," + to_string(team.getNumberOfPlayersOnTeam());

        res += "," + to_string(team.getAllyTeams().size());
        res += "," + to_string(team.getEnemyTeams().size());

        for (auto& ally_team : team.getAllyTeams()) {
            res += "," + to_string(static_cast<unsigned int>(ally_team));
        }
        for (auto& enemy_team : team.getEnemyTeams()) {
            res += "," + to_string(static_cast<unsigned int>(enemy_team));
        }
        for (auto& playerMapKey : team.getPlayers()) {
            res += "," + playerMapKey;
        }
    }
    res += "," + to_string(m_AllowedShipClasses.size());
    for (auto& ship_class : m_AllowedShipClasses) {
        res += "," + ship_class;
    }
    return res;
}
void GameplayMode::deserialize(const string& input) {
    auto list                  = Helper::SeparateStringByCharacter(input, ',');
    m_GameplayModeType         = static_cast<GameplayModeType::Mode>(stoi(list[0]));
    m_MaxAmountOfPlayers       = static_cast<unsigned int>(stoi(list[1]));

    unsigned int team_size     = static_cast<unsigned int>(stoi(list[2]));
    unsigned int start_index   = 3;

    TeamNumber::Enum    ally_team;
    TeamNumber::Enum    enemy_team;
    TeamNumber::Enum    team_number;
    unsigned int        num_ally_teams;
    unsigned int        num_enemy_teams;
    unsigned int        num_players_on_team;
    string              player_map_key;
    for (unsigned int i = 0; i < team_size; ++i) {

        unsigned int count = 0;
        team_number = static_cast<TeamNumber::Enum>(stoi(list[start_index]));
        num_players_on_team = stoi(list[start_index + 1]);
        num_ally_teams      = stoi(list[start_index + 2]);
        num_enemy_teams     = stoi(list[start_index + 3]);
        count += 4;


        //add team
        Team* newTeam = nullptr;
        if (!m_Teams.count(team_number)) {
            newTeam = new Team(team_number);
            m_Teams.emplace(team_number, newTeam);
        }else{
            newTeam = m_Teams.at(team_number);
        }

        for (unsigned int j = 0; j < num_ally_teams; ++j) {
            ally_team = static_cast<TeamNumber::Enum>(stoi(list[start_index + count]));
            newTeam->addAllyTeam(ally_team);
            ++count;
        }
        for (unsigned int j = 0; j < num_enemy_teams; ++j) {
            enemy_team = static_cast<TeamNumber::Enum>(stoi(list[start_index + count]));
            newTeam->addEnemyTeam(enemy_team);
            ++count;
        }
        for (unsigned int j = 0; j < num_players_on_team; ++j) {
            player_map_key = list[start_index + count];
            newTeam->addPlayerToTeam(player_map_key);
            ++count;
        }
        start_index += count;
    }
    int amount_of_ship_classes = stoi(list[start_index]);
    if (amount_of_ship_classes > 0) {
        for (int i = 0; i < amount_of_ship_classes; ++i) {
            const auto ship_class = list[start_index + i];
            addAllowedShipClass(ship_class);
            ++start_index;
        }
    }
}