#pragma once
#ifndef GAME_NETWORKING_CLIENT_MAP_SPECIFIC_DATA_H
#define GAME_NETWORKING_CLIENT_MAP_SPECIFIC_DATA_H

#include "../../modes/GameplayMode.h"

class Map;
class Team;
class Client;
class Core;
class Menu;
class ClientMapSpecificData final {
    friend class Client;
    friend class Core;
    friend class Menu;
    private:
        Client&           m_Client;
        GameplayMode      m_GameplayMode;
        Team*             m_Team;
        Map*              m_Map;
        double            m_PingTimeHealthUpdate;
        double            m_PingTimePhysicsUpdate;
    public:
        ClientMapSpecificData(Client&);
        ~ClientMapSpecificData();

        ClientMapSpecificData(const ClientMapSpecificData& other)                = delete;
        ClientMapSpecificData& operator=(const ClientMapSpecificData& other)     = delete;
        ClientMapSpecificData(ClientMapSpecificData&& other) noexcept            = delete;
        ClientMapSpecificData& operator=(ClientMapSpecificData&& other) noexcept = delete;

        bool operator==(const bool& rhs) const;
        explicit operator bool() const;

        Map& getMap();
        Team& getTeam();
        Client& getClient();
        GameplayMode& getGameplayMode();

        void cleanup();

        void update(const double& dt);
};

#endif