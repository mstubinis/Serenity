#pragma once
#ifndef GAME_CORE_H
#define GAME_CORE_H

#include "GameState.h"
#include <string>
#include <core/engine/Engine_Utils.h>

class HUD;
class Server;
class Client;
class Core final {
    private:
        HUD*                m_HUD;
        Server*             m_Server;
        Client*             m_Client;
        bool                m_Initalized;
        GameState::State    m_GameState;
    public:
        Core();
        ~Core();

        void startClient(const unsigned short& port, const std::string& ip = "127.0.0.1");
        void shutdownClient();

        void startServer(const unsigned short& port);
        void shutdownServer();

        void onResize(const uint& width, const uint& height);

        void enterMap(const std::string& mapFile);

        const GameState::State& gameState() const;

        void init();

        void update(const double& dt);
        void render();
};


#endif