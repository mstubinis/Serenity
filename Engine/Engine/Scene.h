#pragma once
#ifndef ENGINE_SCENE_H
#define ENGINE_SCENE_H

#include "Engine_ResourceBasic.h"

#include <unordered_map>
#include <glm/glm.hpp>

class Object;
class Camera;
class SunLight;
class SkyboxEmpty;

class Scene: public EngineResource{
    private:
        SkyboxEmpty* m_Skybox;
    protected:
        std::unordered_map<std::string,Object*> m_Objects;
        std::unordered_map<std::string,SunLight*> m_Lights;

        glm::vec3 m_AmbientLighting;
        glm::vec3 m_BackgroundColor;
    public:
        Scene(std::string name,glm::vec3 = glm::vec3(0.025f,0.025f,0.025f));
        virtual ~Scene();

		virtual void update(float);

        std::unordered_map<std::string,Object*>& objects();
        std::unordered_map<std::string,SunLight*>& lights();

        Object* getObject(std::string);
        SunLight* getLight(std::string);

        glm::vec3 getAmbientLightColor();
        glm::vec3 getBackgroundColor();
        void setAmbientLightColor(glm::vec3&);
        void setAmbientLightColor(float,float,float);
        void setBackgroundColor(float,float,float);

        SkyboxEmpty* getSkybox() const;
        void setSkybox(SkyboxEmpty*);
        void centerSceneToObject(Object*);
        void renderSkybox(bool);
};
#endif