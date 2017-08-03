#include "Scene.h"
#include "Light.h"
#include "Engine_Resources.h"
#include "Camera.h"
#include "Skybox.h"
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/make_shared.hpp>

using namespace Engine;
using namespace std;

Scene::Scene(string name){
    m_Skybox = nullptr;
    if(Resources::getCurrentScene() == nullptr){
        Resources::Detail::ResourceManagement::m_CurrentScene = this;
    }
    m_BackgroundColor = glm::vec3(0,0,0);
    if(!exists(Resources::getActiveCameraPtr())){
        new Camera("Default_" + name,45.0f,1.0f,0.1f,100.0f,this);
        Resources::setActiveCamera("Default_" + name);
    }
    setName(name);
    Resources::Detail::ResourceManagement::_addToContainer(Resources::Detail::ResourceManagement::m_Scenes,name,boost::shared_ptr<Scene>(this));
}
void Scene::centerSceneToObject(Object* center){
    glm::vec3 offset = -(center->getPosition());
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
void Scene::setBackgroundColor(float r, float g, float b){ Engine::Math::setColor(m_BackgroundColor,r,g,b); }
void Scene::renderSkybox(bool godsRays){ if(m_Skybox != nullptr) m_Skybox->draw(godsRays); }

glm::vec3 Scene::getBackgroundColor(){ return m_BackgroundColor; }

unordered_map<string,Object*>& Scene::objects() { return m_Objects; }
unordered_map<string,SunLight*>& Scene::lights() { return m_Lights; }

Object* Scene::getObject(string& name){ return m_Objects.at(name); } //might need skey(name) instead of name in at()
SunLight* Scene::getLight(string& name){ return m_Lights.at(name); } //might need skey(name) instead of name in at()

SkyboxEmpty* Scene::getSkybox() const { return m_Skybox; }
void Scene::setSkybox(SkyboxEmpty* s){ m_Skybox = s; }
