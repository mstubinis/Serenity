#pragma once
#ifndef GAME_GAME_SKYBOX_H
#define GAME_GAME_SKYBOX_H

#include <core/engine/scene/Skybox.h>
#include <memory>

class GameSkybox: public Skybox{
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
        GameSkybox(const std::string* names, const uint& numSunFlares = 0);
        GameSkybox(const std::string& name, const uint& numSunFlares = 0);
        virtual ~GameSkybox();

        void update();
        void draw();
};
#endif