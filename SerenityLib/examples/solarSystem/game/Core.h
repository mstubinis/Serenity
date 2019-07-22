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
    friend class HUD;
    friend class Client;
    friend class Server;
    private:
        HUD*                m_HUD;
        Server*             m_Server;
        Client*             m_Client;
        bool                m_Initalized;
        GameState::State    m_GameState;
    public:
        Core();
        ~Core();

        Server* getServer();
        Client* getClient();

        void startClient(const unsigned short& port, const std::string& name, const std::string& ip = "127.0.0.1");
        void shutdownClient(const bool& serverShutdownFirst = false);

        void requestValidation(const std::string& name);

        void startServer(const unsigned short& port, const std::string& mapname);
        void shutdownServer();

        void onResize(const uint& width, const uint& height);

        void enterMap(const std::string& mapFile);

        const GameState::State& gameState() const;

        void init();

        void update(const double& dt);
        void render();
};


#endif