#pragma once
#ifndef GAME_CORE_H
#define GAME_CORE_H

#include "GameState.h"
#include <string>
#include <core/engine/utils/Utils.h>

class  Menu;
class  Map;
class  Server;
class  Client;
class  EntityWrapper;
class  ShipSystemSensors;
class  SensorStatusDisplay;
class  Team;
class  GameplayMode;
class  ClientMapSpecificData;
class Core final {
    friend class  Menu;
    friend class  Map;
    friend class  Client;
    friend class  Server;
    friend class  ShipSystemSensors;
    friend class  SensorStatusDisplay;
    friend class  ClientMapSpecificData;
    private:
        EntityWrapper*      m_ChosenShip;
        Menu*               m_Menu;
        Server*             m_Server;
        Client*             m_Client;
        bool                m_Initalized;
        double              m_GameTime;
        GameState::State    m_GameState;
    public:
        Core();
        ~Core();

        Server* getServer();
        Client* getClient();

        void startClient(GameplayMode* mode, Team* team, const unsigned short& port, const std::string& name, const std::string& ip = "127.0.0.1");
        void shutdownClient(const bool& serverShutdownFirst = false);

        void requestValidation(const std::string& name);

        void startServer(const unsigned short& port, const std::string& mapname);
        void shutdownServer();

        void onResize(const uint& width, const uint& height);

        void enterMap(Team& playerTeam, const std::string& mapFile, const std::string& playership, const std::string& playername, const float& x, const float& y, const float& z);

        const GameState::State& gameState() const;

        void init();

        void update(const double& dt);
        void render();
};
#endif