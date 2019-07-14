#pragma once
#ifndef GAME_CORE_H
#define GAME_CORE_H

#include "GameState.h"

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
        GameState::State    m_GameStatePrevious;
    public:
        Core();
        ~Core();

        const GameState::State& gameState() const;

        void init();

        void update(const double& dt);
        void render();
};


#endif