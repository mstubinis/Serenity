#include "Scene.h"
#include "Light.h"
#include "Engine_Resources.h"
#include "Camera.h"
#include "Skybox.h"
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/make_shared.hpp>

using namespace Engine;

Scene::Scene(std::string name,glm::vec3 ambientLightColor){
    m_Skybox = nullptr;
    if(Resources::getCurrentScene() == nullptr){
        Resources::Detail::ResourceManagement::m_CurrentScene = this;
    }
    m_AmbientLighting = ambientLightColor;
    m_BackgroundColor = glm::vec3(0,0,0);

    if(!exists(Resources::getActiveCameraPtr())){
        new Camera("Default_" + name,45.0f,1.0f,0.1f,100.0f,this);
        Resources::setActiveCamera("Default_" + name);
    }
    setName(name);
    Resources::Detail::ResourceManagement::_addToContainer(Resources::Detail::ResourceManagement::m_Scenes,name,boost::shared_ptr<Scene>(this));
}
void Scene::centerSceneToObject(Object* center){
    glm::v3 offset = -(center->getPosition());
    for(auto object:m_Objects){
        Object* obj = object.second;
        if(obj != center && obj->getParent() == nullptr){
            obj->setPosition(obj->getPosition() + offset);
        }
    }
    if(center->getParent() == nullptr)
        center->setPosition(0,0,0);
}
Scene::~Scene(){
    SAFE_DELETE(m_Skybox);
}
void Scene::update(float dt){
    for (auto it = m_Objects.cbegin(); it != m_Objects.cend();){
        if (it->second->isDestroyed()){
            Resources::Detail::ResourceManagement::_removeFromContainer(Resources::Detail::ResourceManagement::m_Objects,it->second->name());
            m_Objects.erase(it++);
        }
        else{
            it->second->update(dt);
            ++it;
        }
    }
    for (auto it = Resources::Detail::ResourceManagement::m_Cameras.cbegin(); it != Resources::Detail::ResourceManagement::m_Cameras.cend();){
        if (it->second->isDestroyed()){
            Resources::Detail::ResourceManagement::_removeFromContainer(Resources::Detail::ResourceManagement::m_Cameras,it->second->name());
        }
        else{
            if(it->second->getScene() == this){
                it->second->update(dt);
            }
            ++it;
        }
    }
    if(m_Skybox != nullptr) m_Skybox->update();
}
void Scene::setAmbientLightColor(glm::vec3& c){ setAmbientLightColor(c.r,c.g,c.b); }
void Scene::setAmbientLightColor(float r,float g,float b){ Engine::Math::setColor(m_AmbientLighting,r,g,b); }
void Scene::setBackgroundColor(float r, float g, float b){ Engine::Math::setColor(m_BackgroundColor,r,g,b); }
void Scene::renderSkybox(bool godsRays){ if(m_Skybox != nullptr) m_Skybox->draw(godsRays); }

glm::vec3 Scene::getAmbientLightColor(){ return m_AmbientLighting; }
glm::vec3 Scene::getBackgroundColor(){ return m_BackgroundColor; }

std::unordered_map<std::string,Object*>& Scene::objects() { return m_Objects; }
std::unordered_map<std::string,SunLight*>& Scene::lights() { return m_Lights; }

Object* Scene::getObject(std::string name){ return m_Objects.at(name); } //might need skey(name) instead of name in at()
SunLight* Scene::getLight(std::string name){ return m_Lights.at(name); } //might need skey(name) instead of name in at()

SkyboxEmpty* Scene::getSkybox() const { return m_Skybox; }
void Scene::setSkybox(SkyboxEmpty* s){ m_Skybox = s; }