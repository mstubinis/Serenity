#pragma once
#ifndef ENGINE_SCENE_H
#define ENGINE_SCENE_H

#include "Engine_ResourceBasic.h"
#include "Engine_Utils.h"
#include "Components.h"
#include <unordered_map>
#include <glm/glm.hpp>
#include <iostream>

class Entity;
class Camera;
class SunLight;
class SkyboxEmpty;
class LightProbe;

class Scene: public EngineResource, public EventObserver{
    friend class LightProbe;
    friend class ::Engine::epriv::ComponentManager;
    private:
        SkyboxEmpty* m_Skybox;
        Camera* m_ActiveCamera;
    protected:
        std::vector<uint> m_Entities;

        std::vector<SunLight*> m_Lights;
        std::unordered_map<std::string,LightProbe*> m_LightProbes;

        glm::vec3 m_BackgroundColor;
    public:
        Scene(std::string name);
        virtual ~Scene();

        Entity* getEntity(uint entityID);
        uint addEntity(Entity*);
        void removeEntity(Entity*,bool immediate = false);
        void removeEntity(uint id,bool immediate = false);
        bool hasEntity(Entity*);
        bool hasEntity(uint entityID);

        virtual void update(const float& dt);

        std::vector<uint>& entities();
        std::vector<SunLight*>& lights();
        std::unordered_map<std::string,LightProbe*>& lightProbes();

        Camera* getActiveCamera();

        glm::vec3 getBackgroundColor();
        void setBackgroundColor(float,float,float);

        SkyboxEmpty* skybox() const;
        void setSkybox(SkyboxEmpty*);
        void centerSceneToObject(Entity*);
        void setActiveCamera(Camera*);
};
#endif
