#pragma once
#ifndef GAME_HUD_H
#define GAME_HUD_H

#include <glm/fwd.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <core/engine/resources/Engine_Resources.h>

#include "GameState.h"

class  Font;
class  Ship;
struct Entity;
class HUD final{
    private:
        glm::vec3            m_Color;
        Handle               m_Font;
        bool                 m_Active;
        GameState::State&    m_GameState;
        GameState::State&    m_GameStatePrevious;


        void update_game(const double& dt);
        void update_main_menu(const double& dt);
        void update_host_server_map_and_ship(const double& dt);
        void update_host_server_port_and_name(const double& dt);
        void update_join_server_port_and_name_and_ip(const double& dt);
        void update_join_server_server_info(const double& dt);

        void render_game();
        void render_main_menu();
        void render_host_server_map_and_ship();
        void render_host_server_port_and_name();
        void render_join_server_port_and_name_and_ip();
        void render_join_server_server_info();

    public:
        HUD(GameState::State& current, GameState::State& previous);
        ~HUD();

        void update(const double& dt);
        void render();
};
#endif