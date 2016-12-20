#include "Scene.h"
#include "Light.h"
#include "Engine_Resources.h"
#include "Camera.h"
#include "Skybox.h"
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

using namespace Engine;
using namespace boost;

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

    Resources::Detail::ResourceManagement::m_Scenes[name] = shared_ptr<Scene>(this);
	setName(name);
}
void Scene::centerSceneToObject(Object* center){
    glm::v3 offset = -(center->getPosition());
    Scene* s =  Resources::getCurrentScene();
    for(auto object:s->getObjects()){
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
            std::string name = it->second->name();
            Resources::getObjectPtr(name).reset();
            Resources::Detail::ResourceManagement::m_Objects.erase(name);
            m_Objects.erase(it++);
        }
        else{
            it->second->update(dt);
            ++it;
        }
    }
    for (auto it = Resources::Detail::ResourceManagement::m_Cameras.cbegin(); it != Resources::Detail::ResourceManagement::m_Cameras.cend();){
        if (it->second->isDestroyed()){
            std::string name = it->second->name();
            Resources::getCameraPtr(name).reset();
            Resources::Detail::ResourceManagement::m_Cameras.erase(it++);
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
void Scene::setAmbientLightColor(glm::vec3 c){ setAmbientLightColor(c.r,c.g,c.b); }
void Scene::setAmbientLightColor(float r,float g,float b){ Engine::Math::setColor(m_AmbientLighting,r,g,b); }
void Scene::setBackgroundColor(float r, float g, float b){ Engine::Math::setColor(m_BackgroundColor,r,g,b); }
void Scene::renderSkybox(bool godsRays){ if(m_Skybox != nullptr) m_Skybox->render(godsRays); }