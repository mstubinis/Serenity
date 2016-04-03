#ifndef GAMESKYBOX_H
#define GAMESKYBOX_H

#include "Skybox.h"

struct SkyboxSunFlare final{
    glm::vec3 position;
    glm::vec3 color;
    float scale;
};

class GameSkybox: public Skybox{
    private:
        std::vector<SkyboxSunFlare> m_SunFlares;
    public:
        GameSkybox(std::string name,unsigned int numSunFlares = 0,Scene* = nullptr);
        virtual ~GameSkybox();

        virtual void update();
        virtual void render();
};

#endif
