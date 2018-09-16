#include "Scene.h"
#include "Engine.h"
#include "Light.h"
#include "Engine_Resources.h"
#include "Engine_Math.h"
#include "Camera.h"
#include "Skybox.h"

using namespace Engine;
using namespace std;

class Scene::impl final {
    public:
        SkyboxEmpty* m_Skybox;
        Camera* m_ActiveCamera;
        std::vector<uint> m_Entities;
        std::vector<SunLight*> m_Lights;
        std::unordered_map<std::string, LightProbe*> m_LightProbes;
        glm::vec3 m_BackgroundColor;

        void _init(Scene* super,string& _name) {
            m_Skybox = nullptr;
            m_ActiveCamera = nullptr;
            m_BackgroundColor = glm::vec3(0.0f);
            _name = epriv::Core::m_Engine->m_ResourceManager->_buildSceneName(_name);
            super->setName(_name);
            epriv::Core::m_Engine->m_ResourceManager->_addScene(super);
            if (!Resources::getCurrentScene()) {
                Resources::setCurrentScene(super);
            }
        }
        void _destruct() {
            SAFE_DELETE(m_Skybox);
        }
        uint _addEntity(Scene* super, Entity* _entity) {
            for (auto entityInScene : m_Entities) { if (_entity->m_ID == entityInScene) return entityInScene; } //rethink this maybe use a fixed size array?
            const uint entityID = epriv::Core::m_Engine->m_ComponentManager->m_EntityPool->add(_entity);
            _entity->m_ID = entityID;
            epriv::Core::m_Engine->m_ComponentManager->onEntityAddedToScene(super, _entity);
            m_Entities.push_back(entityID);
            return entityID;
        }
        bool _hasEntity(Scene* super, uint& _id) {
            for (auto entityInScene : m_Entities) {
                if (_id == entityInScene)
                    return true;
            }
            return false;
        }
        bool _hasEntity(Scene* super, Entity* _entity) { return _hasEntity(super, _entity->m_ID); }
        void _centerToObject(Scene* super,uint& centerID) {
            Entity* center = super->getEntity(centerID);
            ComponentBody& bodyBase = *(center->getComponent<ComponentBody>());
            for (auto entityID : m_Entities) {
                Entity* e = super->getEntity(entityID);
                ComponentBody& entityBody = *(e->getComponent<ComponentBody>());
                if (e != center && !e->parent()) {
                    entityBody.setPosition(entityBody.position() - bodyBase.position());
                }
            }
            if (!center->parent()) {
                bodyBase.setPosition(0.0f, 0.0f, 0.0f);
            }
        }
};


vector<uint>& epriv::InternalScenePublicInterface::GetEntities(Scene* _scene) { return _scene->m_i->m_Entities; }
vector<SunLight*>& epriv::InternalScenePublicInterface::GetLights(Scene* _scene) { return _scene->m_i->m_Lights; }



Scene::Scene(string name):m_i(new impl){
    m_i->_init(this, name);
    registerEvent(EventType::SceneChanged);
}
uint Scene::addEntity(Entity* entity){ return m_i->_addEntity(this,entity); }
void Scene::removeEntity(Entity* e,bool immediate){
    e->destroy(immediate);
}
void Scene::removeEntity(uint id,bool immediate){
    Entity* e = Components::GetEntity(id);
    removeEntity(e,immediate);
}
Entity* Scene::getEntity(uint entityID){
    if(entityID == 0) return nullptr;
    return Components::GetEntity(entityID);
}
bool Scene::hasEntity(Entity* entity){ return m_i->_hasEntity(this, entity); }
bool Scene::hasEntity(uint entityID){ return m_i->_hasEntity(this, entityID); }
Camera* Scene::getActiveCamera(){ return m_i->m_ActiveCamera; }
void Scene::setActiveCamera(Camera* c){ m_i->m_ActiveCamera = c; }
void Scene::centerSceneToObject(Entity* center){ return m_i->_centerToObject(this, center->m_ID); }
void Scene::centerSceneToObject(uint centerID) { return m_i->_centerToObject(this, centerID); }
Scene::~Scene(){
    unregisterEvent(EventType::SceneChanged);
    m_i->_destruct();
}
void Scene::update(const float& dt){
}
glm::vec3 Scene::getBackgroundColor(){ return m_i->m_BackgroundColor; }
unordered_map<string,LightProbe*>& Scene::lightProbes(){ return m_i->m_LightProbes; }
SkyboxEmpty* Scene::skybox() const { return m_i->m_Skybox; }
void Scene::setSkybox(SkyboxEmpty* s){ m_i->m_Skybox = s; }
void Scene::setBackgroundColor(float r, float g, float b){ Math::setColor(m_i->m_BackgroundColor,r,g,b); }