#include "GameplayMode.h"
#include "../teams/Team.h"
#include "../Helper.h"
#include "../ships/Ships.h"
#include "../networking/packets/PacketGameplayModeInfo.h"

#include <iostream>


using namespace std;


std::vector<std::string> GameplayMode::GAMEPLAY_TYPE_ENUM_NAMES = []() {
    std::vector<std::string> ret;
    ret.resize(GameplayModeType::_TOTAL, "");

    ret[GameplayModeType::FFA]              = "Free for All";
    ret[GameplayModeType::TeamDeathmatch]   = "Team Deathmatch";
    ret[GameplayModeType::HomelandSecurity] = "Homeland Security";

    return ret;
}();


GameplayMode::GameplayMode() {
    setGameplayMode(GameplayModeType::FFA);
    setMaxAmountOfPlayers(0);
}
GameplayMode::GameplayMode(const GameplayModeType::Mode& mode, const unsigned int MaxAmountOfPlayers) {
    setGameplayMode(mode);
    setMaxAmountOfPlayers(MaxAmountOfPlayers);
}
GameplayMode::~GameplayMode() {
    clear();
}
void GameplayMode::clear() {
    m_MaxAmountOfPlayers = 0;
    m_AllowedShipClasses.clear();
}
unordered_map<TeamNumber::Enum, Team>& GameplayMode::getTeams() {
    return m_Teams;
}
void GameplayMode::setMaxAmountOfPlayers(const unsigned int& maxPlayers) {
    m_MaxAmountOfPlayers = maxPlayers;
}
void GameplayMode::setGameplayMode(const GameplayModeType::Mode& mode) {
    m_GameplayModeType = mode;
}
const GameplayModeType::Mode& GameplayMode::getGameplayMode() const {
    return m_GameplayModeType;
}
const unsigned int& GameplayMode::getMaxAmountOfPlayers() const {
    return m_MaxAmountOfPlayers;
}
const bool GameplayMode::addTeam(Team& team) {
    if (m_Teams.count(team.getTeamNumber())) {
        return false;
    }
    m_Teams.emplace(team.getTeamNumber(), team);
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
Team* GameplayMode::getTeam(const TeamNumber::Enum& teamNumberEnum) {
    if (m_Teams.count(teamNumberEnum)) {
        return &m_Teams.at(teamNumberEnum);
    }
    return nullptr;
}
const PacketGameplayModeInfo GameplayMode::serialize() const {
    PacketGameplayModeInfo res;

    res.gameplay_mode_type                  = static_cast<unsigned int>(m_GameplayModeType);
    res.gameplay_mode_team_sizes            = static_cast<unsigned int>(m_Teams.size());
    res.gameplay_mode_max_number_of_players = static_cast<unsigned int>(m_MaxAmountOfPlayers);

    for (auto& team_itr : m_Teams) {
        Team& team = const_cast<Team&>(team_itr.second);
        res.team_data += "," + team.getTeamNumberAsString();
        res.team_data += "," + to_string(team.getNumberOfPlayersOnTeam());

        res.team_data += "," + to_string(team.getAllyTeams().size());
        res.team_data += "," + to_string(team.getEnemyTeams().size());

        for (auto& ally_team_number_enum : team.getAllyTeams()) {
            res.team_data += "," + to_string(static_cast<unsigned int>(ally_team_number_enum));
        }
        for (auto& enemy_team_number_enum : team.getEnemyTeams()) {
            res.team_data += "," + to_string(static_cast<unsigned int>(enemy_team_number_enum));
        }
        for (auto& playerMapKey : team.getPlayers()) {
            res.team_data += "," + playerMapKey;
        }
    }
    res.team_data.erase(0, 1); //remove first ","


    res.allowed_ships += to_string(m_AllowedShipClasses.size());
    for (auto& ship_class : m_AllowedShipClasses) {
        res.allowed_ships += "," + ship_class;
    }
    return res;
}
void GameplayMode::deserialize(const PacketGameplayModeInfo& packet) {

    const auto list_team_data     = Helper::SeparateStringByCharacter(packet.team_data, ',');
    const auto list_allowed_ships = Helper::SeparateStringByCharacter(packet.allowed_ships, ',');
    m_GameplayModeType            = static_cast<GameplayModeType::Mode>(packet.gameplay_mode_type);
    m_MaxAmountOfPlayers          = static_cast<unsigned int>(packet.gameplay_mode_max_number_of_players);
    size_t start_index            = 0;

    TeamNumber::Enum    ally_team_number_enum;
    TeamNumber::Enum    enemy_team_number_enum;
    TeamNumber::Enum    team_number_enum;
    unsigned int        num_ally_teams;
    unsigned int        num_enemy_teams;
    unsigned int        num_players_on_team;
    string              player_map_key;
    for (unsigned int i = 0; i < packet.gameplay_mode_team_sizes; ++i) {

        unsigned int count  = 0;
        team_number_enum    = static_cast<TeamNumber::Enum>(stoi(list_team_data[start_index + 0]));
        num_players_on_team =                               stoi(list_team_data[start_index + 1]);
        num_ally_teams      =                               stoi(list_team_data[start_index + 2]);
        num_enemy_teams     =                               stoi(list_team_data[start_index + 3]);
        count += 4;

        //add team
        Team* newTeam = nullptr;
        if (!m_Teams.count(team_number_enum)) {
            newTeam = NEW Team(team_number_enum);
            m_Teams.emplace(team_number_enum, *newTeam);
            SAFE_DELETE(newTeam);
        }
        newTeam = &(m_Teams.at(team_number_enum));

        for (unsigned int j = 0; j < num_ally_teams; ++j) {
            ally_team_number_enum = static_cast<TeamNumber::Enum>(stoi(list_team_data[start_index + count]));
            newTeam->addAllyTeam(ally_team_number_enum);
            ++count;
        }
        for (unsigned int j = 0; j < num_enemy_teams; ++j) {
            enemy_team_number_enum = static_cast<TeamNumber::Enum>(stoi(list_team_data[start_index + count]));
            newTeam->addEnemyTeam(enemy_team_number_enum);
            ++count;
        }
        for (unsigned int j = 0; j < num_players_on_team; ++j) {
            player_map_key = list_team_data[start_index + count];
            newTeam->addPlayerToTeam(player_map_key);
            ++count;
        }
        start_index += count;
    }



    for (auto& ship_class : list_allowed_ships) {
        addAllowedShipClass(ship_class);
    }
}