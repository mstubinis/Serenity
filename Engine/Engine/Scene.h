#pragma once
#ifndef ENGINE_SCENE_H
#define ENGINE_SCENE_H

#include "Engine_ResourceBasic.h"
#include "Engine_Renderer.h"
#include <unordered_map>
#include <glm/glm.hpp>

class Object;
class Camera;
class SunLight;
class SkyboxEmpty;
class LightProbe;

class Scene: public EngineResource{
	friend class LightProbe;
	friend class Engine::Renderer::Detail::RenderManagement;
    private:
        SkyboxEmpty* m_Skybox;
    protected:
        std::unordered_map<std::string,Object*> m_Objects;
        std::unordered_map<std::string,SunLight*> m_Lights;
		std::unordered_map<std::string,LightProbe*> m_LightProbes;

        glm::vec3 m_BackgroundColor;
    public:
        Scene(std::string name);
        virtual ~Scene();

        virtual void update(float);

        std::unordered_map<std::string,Object*>& objects();
        std::unordered_map<std::string,SunLight*>& lights();

        Object* getObject(std::string&);
        SunLight* getLight(std::string&);

        glm::vec3 getBackgroundColor();
        void setBackgroundColor(float,float,float);

        SkyboxEmpty* getSkybox() const;
        void setSkybox(SkyboxEmpty*);
        void centerSceneToObject(Object*);
        void renderSkybox();
};
#endif