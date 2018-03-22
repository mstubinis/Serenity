#pragma once
#ifndef ENGINE_SCENE_H
#define ENGINE_SCENE_H

#include "Engine_ResourceBasic.h"
#include "Components.h"
#include <unordered_map>
#include <glm/glm.hpp>

class Entity;
class Object;
class Camera;
class SunLight;
class SkyboxEmpty;
class LightProbe;

class Scene: public EngineResource{
    friend class LightProbe;
	friend class ::Engine::epriv::ComponentManager;
    private:
        SkyboxEmpty* m_Skybox;
		Camera* m_ActiveCamera;
    protected:
		std::vector<uint> m_Entities;

        std::unordered_map<std::string,Object*> m_Objects;
        std::vector<SunLight*> m_Lights;
        std::unordered_map<std::string,LightProbe*> m_LightProbes;

        glm::vec3 m_BackgroundColor;
    public:
        Scene(std::string name);
        virtual ~Scene();

		Entity* getEntity(uint entityID);
		void addEntity(Entity*);
		bool hasEntity(Entity*);
		bool hasEntity(uint entityID);

        virtual void update(float);

        std::unordered_map<std::string,Object*>& objects();
        std::vector<SunLight*>& lights();
		std::unordered_map<std::string,LightProbe*>& lightProbes();

        Object* getObject(std::string&);
		Camera* getActiveCamera();

        glm::vec3 getBackgroundColor();
        void setBackgroundColor(float,float,float);

        SkyboxEmpty* skybox() const;
        void setSkybox(SkyboxEmpty*);
        void centerSceneToObject(Object*);
		void setActiveCamera(Camera*);
};
#endif
