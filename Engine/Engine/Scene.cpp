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
	Handle entityHandle = epriv::Core::m_Engine->m_ComponentManager->m_EntityPool->add(entity,EntityType::Basic);
	entity->m_ID = entityHandle.index;
	m_Entities.push_back(entityHandle.index);
}
Entity* Scene::getEntity(uint entityID){
	return epriv::Core::m_Engine->m_ComponentManager->m_EntityPool->getAsFast<Entity>(entityID);
}
bool Scene::hasEntity(Entity* entity){
	for(auto entityInScene:m_Entities){if (entity->m_ID == entityInScene) return true; } //rethink this maybe use a fixed size array?
	return false;
}
bool Scene::hasEntity(uint entityID){
	for(auto entityInScene:m_Entities){if (entityID == entityInScene) return true; } //rethink this maybe use a fixed size array?
	return false;
}
Camera* Scene::getActiveCamera(){ return m_ActiveCamera; }
void Scene::setActiveCamera(Camera* c){
	m_ActiveCamera = c;
}
void Scene::centerSceneToObject(Object* center){
    glm::vec3 offset = -(center->getPosition());
	//fix this after implementing components
    for(auto object:m_Objects){
        Object* obj = object.second;
        if(obj != center && obj->parent() == nullptr){
            obj->setPosition(obj->getPosition() + offset);
        }
    }
	for(auto object:m_Entities){
		Entity* e = getEntity(object);
		epriv::ComponentBodyBaseClass& entityBody = *(e->getComponent<epriv::ComponentBodyBaseClass>());
        if(e->parent() == nullptr){
			entityBody.setPosition(entityBody.position() + offset);
        }
    }
    if(center->parent() == nullptr)
        center->setPosition(0.0f,0.0f,0.0f);
}
void Scene::centerSceneToObject(Entity* center){
	epriv::ComponentBodyBaseClass& bodyBase = *(center->getComponent<epriv::ComponentBodyBaseClass>());
	//fix this after implementing components
    glm::vec3 offset = -(bodyBase.position());
    for(auto object:m_Objects){
        Object* obj = object.second;
        if(obj->parent() == nullptr){
            obj->setPosition(obj->getPosition() + offset);
        }
    }
	for(auto object:m_Entities){
		Entity* e = getEntity(object);
		epriv::ComponentBodyBaseClass& entityBody = *(e->getComponent<epriv::ComponentBodyBaseClass>());
        if(e != center && e->parent() == nullptr){
			entityBody.setPosition(entityBody.position() + offset);
        }
    }
    if(center->parent() == nullptr)
        bodyBase.setPosition(0.0f,0.0f,0.0f);
}
Scene::~Scene(){
    SAFE_DELETE(m_Skybox);
}
void Scene::update(float dt){
}
void Scene::setBackgroundColor(float r, float g, float b){ Math::setColor(m_BackgroundColor,r,g,b); }

glm::vec3 Scene::getBackgroundColor(){ return m_BackgroundColor; }
std::vector<uint>& Scene::entities(){ return m_Entities; }
unordered_map<string,Object*>& Scene::objects() { return m_Objects; }
vector<SunLight*>& Scene::lights() { return m_Lights; }
unordered_map<string,LightProbe*>& Scene::lightProbes(){ return m_LightProbes; }
Object* Scene::getObject(string& name){ return m_Objects.at(name); }
SkyboxEmpty* Scene::skybox() const { return m_Skybox; }
void Scene::setSkybox(SkyboxEmpty* s){ m_Skybox = s; }
