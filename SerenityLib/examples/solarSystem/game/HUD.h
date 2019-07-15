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
class  Button;
class  Core;
struct Entity;

struct ButtonHost_OnClick;
struct ButtonJoin_OnClick;
struct ButtonBack_OnClick;
struct ButtonNext_OnClick;

class HUD final{
    friend struct ButtonHost_OnClick;
    friend struct ButtonJoin_OnClick;
    friend struct ButtonBack_OnClick;
    friend struct ButtonNext_OnClick;
    private:
        glm::vec3            m_Color;
        Handle               m_FontHandle;
        Font*                m_Font;
        bool                 m_Active;
        GameState::State&    m_GameState;
        Core&                m_Core;


        Button*              m_ButtonHost;
        Button*              m_ButtonJoin;

        Button*              m_Back;
        Button*              m_Next;

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
        HUD(GameState::State& current, Core& core);
        ~HUD();

        void onResize(const uint& width, const uint& height);

        void update(const double& dt);
        void render();
};
#endif