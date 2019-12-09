#pragma once
#ifndef GAME_NETWORKING_CLIENT_MAP_SPECIFIC_DATA_H
#define GAME_NETWORKING_CLIENT_MAP_SPECIFIC_DATA_H

class Map;
class Team;
class GameplayMode;
class Client;
class Core;
class Menu;
class ClientMapSpecificData final {
    friend class Client;
    friend class Core;
    friend class Menu;
    private:
        Client&           m_Client;
        GameplayMode*     m_GameplayMode;
        Team*             m_Team;
        Map*              m_Map;
        double            m_PingTimeHealthUpdate;
        double            m_PingTimePhysicsUpdate;
    public:
        ClientMapSpecificData(Client&);
        ~ClientMapSpecificData();

        void cleanup();

        void update(const double& dt);
};

#endif