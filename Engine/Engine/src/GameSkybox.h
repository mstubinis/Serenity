#pragma once
#ifndef GAME_GAMESKYBOX_H_INCLUDE_GUARD
#define GAME_GAMESKYBOX_H_INCLUDE_GUARD

#include "core/Skybox.h"
#include <memory>

class GameSkybox: public Skybox{
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
        GameSkybox(std::string* names,uint numSunFlares = 0,Scene* = nullptr);
        GameSkybox(std::string name,uint numSunFlares = 0,Scene* = nullptr);
        virtual ~GameSkybox();

        void update();
        void draw();
};
#endif