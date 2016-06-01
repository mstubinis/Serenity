#pragma once
#ifndef GAMESKYBOX_H
#define GAMESKYBOX_H

#include "Skybox.h"

struct SkyboxSunFlare final{
    glm::vec3 position;
    glm::vec3 color;
    float scale;
    SkyboxSunFlare(){ position = glm::vec3(0); color = glm::vec3(0); scale = 0;}
    ~SkyboxSunFlare(){ position = glm::vec3(0); color = glm::vec3(0); scale = 0;}
};

class GameSkybox: public Skybox{
    private:
        std::vector<SkyboxSunFlare> m_SunFlares;
    public:
        GameSkybox(std::string name,unsigned int numSunFlares = 0,Scene* = nullptr);
        virtual ~GameSkybox();

        virtual void update();
        virtual void render(bool godsRays);
};

#endif
