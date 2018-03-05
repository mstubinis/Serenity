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
        std::unordered_map<std::string,SunLight*> m_Lights;
        std::unordered_map<std::string,LightProbe*> m_LightProbes;
		std::unordered_map<std::string,Camera*> m_Cameras;

        glm::vec3 m_BackgroundColor;
    public:
        Scene(std::string name);
        virtual ~Scene();

		void addEntity(Entity*);

        virtual void update(float);

        std::unordered_map<std::string,Object*>& objects();
        std::unordered_map<std::string,SunLight*>& lights();
        std::unordered_map<std::string,Camera*>& cameras();
		std::unordered_map<std::string,LightProbe*>& lightProbes();

        Object* getObject(std::string&);
        SunLight* getLight(std::string&);
        Camera* getCamera(std::string&);
		Camera* getActiveCamera();

        glm::vec3 getBackgroundColor();
        void setBackgroundColor(float,float,float);

        SkyboxEmpty* skybox() const;
        void setSkybox(SkyboxEmpty*);
        void centerSceneToObject(Object*);
		void setActiveCamera(Camera*);
		void setActiveCamera(std::string);
};
#endif
