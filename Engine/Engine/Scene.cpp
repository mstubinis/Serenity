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

using namespace Engine;
using namespace std;

Scene::Scene(string name){
    m_Skybox = nullptr;
    m_ActiveCamera = nullptr;
    m_BackgroundColor = glm::vec3(0.0f);
    name = epriv::Core::m_Engine->m_ResourceManager->_buildSceneName(name);
    setName(name);

    epriv::Core::m_Engine->m_ResourceManager->_addScene(this);

    if(!Resources::getCurrentScene()){
        Resources::setCurrentScene(this);
    }
}
uint Scene::addEntity(Entity* entity){
    for(auto entityInScene:m_Entities){if (entity->m_ID == entityInScene) return entityInScene; } //rethink this maybe use a fixed size array?
    uint entityID = epriv::Core::m_Engine->m_ComponentManager->m_EntityPool->add(entity);
    entity->m_ID = entityID;
    entity->m_Scene = this;
    m_Entities.push_back(entityID);
    return entityID;
}
void Scene::removeEntity(Entity* e,bool immediate){
    e->destroy(immediate);
}
void Scene::removeEntity(uint id,bool immediate){
    Entity* e = epriv::Core::m_Engine->m_ComponentManager->_getEntity(id);
    removeEntity(e,immediate);
}
Entity* Scene::getEntity(uint entityID){
    if(entityID == epriv::UINT_MAX_VALUE)
        return nullptr;
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

void Scene::centerSceneToObject(Entity* center){
    epriv::ComponentBodyBaseClass& bodyBase = *(center->getComponent<epriv::ComponentBodyBaseClass>());
    for(auto entityID:m_Entities){
        Entity* e = getEntity(entityID);
        epriv::ComponentBodyBaseClass& entityBody = *(e->getComponent<epriv::ComponentBodyBaseClass>());
        if(e != center && !e->parent()){
            entityBody.setPosition(entityBody.position() - bodyBase.position());
        }
    }
    if(!center->parent()){
        bodyBase.setPosition(0.0f,0.0f,0.0f);
    }
}
Scene::~Scene(){
    SAFE_DELETE(m_Skybox);
}
void Scene::update(const float& dt){
}
glm::vec3 Scene::getBackgroundColor(){ return m_BackgroundColor; }
std::vector<uint>& Scene::entities(){ return m_Entities; }
vector<SunLight*>& Scene::lights() { return m_Lights; }
unordered_map<string,LightProbe*>& Scene::lightProbes(){ return m_LightProbes; }
SkyboxEmpty* Scene::skybox() const { return m_Skybox; }
void Scene::setSkybox(SkyboxEmpty* s){ m_Skybox = s; }
void Scene::setBackgroundColor(float r, float g, float b){ Math::setColor(m_BackgroundColor,r,g,b); }
