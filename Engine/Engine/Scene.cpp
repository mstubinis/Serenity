#include "Engine.h"
#include "Scene.h"
#include "Light.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "Camera.h"
#include "Skybox.h"
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/make_shared.hpp>

#include "Object.h"

using namespace Engine;
using namespace std;

Scene::Scene(string name){
    m_Skybox = nullptr;
	m_ActiveCamera = nullptr;
    m_BackgroundColor = glm::vec3(0.0f);
	name = epriv::Core::m_Engine->m_ResourceManager->_buildSceneName(name);
    setName(name);

	epriv::Core::m_Engine->m_ResourceManager->_addScene(this);

    if(Resources::getCurrentScene() == nullptr){
		Resources::setCurrentScene(this);
    }
}
void Scene::addEntity(Entity* entity){
	for(auto entityInScene:m_Entities){if (entity->m_ID == entityInScene) return; } //rethink this maybe use a fixed size array?
	Handle entityHandle = epriv::Core::m_Engine->m_ResourceManager->_addEntity(entity,EntityType::Basic);
	uint entityID = entityHandle.index;
	m_Entities.push_back(entityID);
}
Camera* Scene::getActiveCamera(){ return m_ActiveCamera; }
void Scene::setActiveCamera(Camera* c){
	m_ActiveCamera = c;
}
void Scene::setActiveCamera(string s){
	m_ActiveCamera = Resources::getCamera(s);
}
void Scene::centerSceneToObject(Object* center){
    glm::vec3 offset = -(center->getPosition());
    for(auto object:m_Objects){
        Object* obj = object.second;
        if(obj != center && obj->parent() == nullptr){
            obj->setPosition(obj->getPosition() + offset);
        }
    }
    if(center->parent() == nullptr)
        center->setPosition(0.0f,0.0f,0.0f);
}
Scene::~Scene(){
    SAFE_DELETE(m_Skybox);
}
void Scene::update(float dt){
	Camera* active = getActiveCamera();
    for (auto it = m_Objects.cbegin(); it != m_Objects.cend();){
		Object* obj = it->second;
        if (obj->isDestroyed()){
            epriv::Core::m_Engine->m_ResourceManager->_remObject(obj->name());
            m_Objects.erase(it++);
        }
        else{
			obj->checkRender(active); //consider batch culling using the thread pool
            obj->update(dt); 
			++it;
        }
    }
    for (auto it = m_Cameras.cbegin(); it != m_Cameras.cend();){
		Camera* cam = it->second;
        if (cam->isDestroyed()){
			epriv::Core::m_Engine->m_ResourceManager->_remCamera(cam->name());
        }
        else{
			cam->update(dt); 
			++it;
        }
    }
    if(m_Skybox != nullptr) m_Skybox->update();
}
void Scene::setBackgroundColor(float r, float g, float b){ Math::setColor(m_BackgroundColor,r,g,b); }

glm::vec3 Scene::getBackgroundColor(){ return m_BackgroundColor; }
unordered_map<string,Object*>& Scene::objects() { return m_Objects; }
unordered_map<string,SunLight*>& Scene::lights() { return m_Lights; }
unordered_map<string,Camera*>& Scene::cameras() { return m_Cameras; }
unordered_map<string,LightProbe*>& Scene::lightProbes(){ return m_LightProbes; }
Object* Scene::getObject(string& name){ return m_Objects.at(name); }
SunLight* Scene::getLight(string& name){ return m_Lights.at(name); }
Camera* Scene::getCamera(string& name){ return m_Cameras.at(name); }
SkyboxEmpty* Scene::skybox() const { return m_Skybox; }
void Scene::setSkybox(SkyboxEmpty* s){ m_Skybox = s; }
