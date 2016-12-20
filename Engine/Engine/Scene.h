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
        std::unordered_map<skey,Object*,skh,skef> m_Objects;
        std::unordered_map<skey,SunLight*,skh,skef> m_Lights;

        glm::vec3 m_AmbientLighting;
        glm::vec3 m_BackgroundColor;
    public:
        Scene(std::string name,glm::vec3 = glm::vec3(0.025f,0.025f,0.025f));
        virtual ~Scene();

        std::unordered_map<skey,Object*,skh,skef>& getObjects(){ return m_Objects; }
        std::unordered_map<skey,SunLight*,skh,skef>& getLights(){ return m_Lights; }

        Object* getObject(std::string name){ return m_Objects[skey(name)]; }
        SunLight* getLight(std::string name){ return m_Lights[skey(name)]; }

        virtual void update(float);
        glm::vec3 getAmbientLightColor(){ return m_AmbientLighting; }
        glm::vec3 getBackgroundColor(){ return m_BackgroundColor; }
        void setAmbientLightColor(glm::vec3);
        void setAmbientLightColor(float,float,float);
        void setBackgroundColor(float,float,float);

        SkyboxEmpty* getSkybox() const { return m_Skybox; }
        void setSkybox(SkyboxEmpty* s){ m_Skybox = s; }
        void centerSceneToObject(Object*);
        void renderSkybox(bool);
};
#endif