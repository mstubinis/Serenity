#include "core/Light.h"
#include "core/engine/Engine_Resources.h"
#include "core/engine/Engine_Renderer.h"
#include "core/engine/Engine_Math.h"
#include "core/MeshInstance.h"
#include "core/Camera.h"
#include "core/Skybox.h"

#include "ecs/Entity.h"

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

uint InternalScenePublicInterface::NumScenes = 0;

class Scene::impl final {
    public:
        SkyboxEmpty* m_Skybox;
        Camera* m_ActiveCamera;
        vector<uint> m_Entities;
        vector<SunLight*> m_Lights;
        glm::vec3 m_BackgroundColor;
        uint m_ID;
        vector<vector<RenderPipeline*>> m_Pipelines;


        ECS<Entity> m_ECS;

        void _init(Scene& super,string& _name) {
            m_Pipelines.resize(RenderStage::_TOTAL);
            m_Skybox = nullptr;
            m_ActiveCamera = nullptr;
            m_BackgroundColor = glm::vec3(0.0f);
            super.setName(_name);
            Core::m_Engine->m_ResourceManager._addScene(super);
            ++InternalScenePublicInterface::NumScenes;
            m_ID = InternalScenePublicInterface::NumScenes;
        }
        void _destruct() {
            SAFE_DELETE(m_Skybox);
            for(auto vec: m_Pipelines)
                SAFE_DELETE_VECTOR(vec);
        }
        uint _OLD_addEntity(Scene& super, OLD_Entity& _entity) {
            if (_entity.ID != 0) return 0;
            for (auto entityInScene : m_Entities) { if (_entity.ID == entityInScene) return entityInScene; } //rethink this maybe use a fixed size array?
            const uint entityID = Core::m_Engine->m_ComponentManager.m_EntityPool->add(&_entity);
            _entity.ID = entityID;
            OLD_ComponentManager::onEntityAddedToScene(super, _entity);
            m_Entities.push_back(entityID);
            return entityID;
        }
        bool _OLD_hasEntity(uint& _id) {
            for (auto entityInScene : m_Entities) {
                if (_id == entityInScene)
                    return true;
            }
            return false;
        }
        bool _OLD_hasEntity(OLD_Entity& _entity) { return _OLD_hasEntity(_entity.ID); }
        void _centerToObject(Scene& super,uint& centerID) {
            //TODO: handle parent->child relationship
            OLD_Entity* center = super.OLD_getEntity(centerID);
            OLD_ComponentBody& bodyBase = *(center->getComponent<OLD_ComponentBody>());
            for (auto entityID : m_Entities) {
                OLD_Entity* e = super.OLD_getEntity(entityID);
                OLD_ComponentBody& entityBody = *(e->getComponent<OLD_ComponentBody>());
                if (e != center) {
                    entityBody.setPosition(entityBody.position() - bodyBase.position());
                }
            }
            bodyBase.setPosition(0.0f, 0.0f, 0.0f);
        }
        void _addMeshInstanceToPipeline(Scene& _scene, MeshInstance& _meshInstance, const vector<RenderPipeline*>& _pipelinesList, RenderStage::Stage _stage) {
            epriv::RenderPipeline* _pipeline = nullptr;
            for (auto pipeline : _pipelinesList) {
                if (&pipeline->shaderProgram == _meshInstance.shaderProgram()) {
                    _pipeline = pipeline;
                    break;
                }
            }
            if (!_pipeline) {
                _pipeline = new epriv::RenderPipeline(*_meshInstance.shaderProgram());
                _scene.m_i->m_Pipelines[_stage].push_back(_pipeline);
            }
            //material node check
            MaterialNode* materialNode = nullptr;
            MeshNode* meshNode = nullptr;
            InstanceNode* instanceNode = nullptr;
            for (auto itr : _pipeline->materialNodes) {
                if (itr->material == _meshInstance.material()) {
                    materialNode = itr;
                    //mesh node check
                    for (auto itr1 : materialNode->meshNodes) {
                        if (itr1->mesh == _meshInstance.mesh()) {
                            meshNode = itr1;
                            //instance check
                            for (auto itr2 : meshNode->instanceNodes) {
                                if (itr2->instance == &_meshInstance) {
                                    instanceNode = itr2;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            if (!materialNode) {
                materialNode = new MaterialNode(*_meshInstance.material());
                _pipeline->materialNodes.emplace_back(materialNode);
            }
            if (!meshNode) {
                meshNode = new MeshNode(*_meshInstance.mesh());
                materialNode->meshNodes.emplace_back(meshNode);
            }
            if (!instanceNode) {
                instanceNode = new InstanceNode(_meshInstance);
                meshNode->instanceNodes.emplace_back(instanceNode);
            }
        }
        void _removeMeshInstanceFromPipeline(Scene& _scene, MeshInstance& _meshInstance, const vector<RenderPipeline*>& _pipelinesList, RenderStage::Stage _stage) {
            RenderPipeline* _pipeline = nullptr;
            for (auto pipeline : _pipelinesList) {
                if (&pipeline->shaderProgram == _meshInstance.shaderProgram()) {
                    _pipeline = pipeline;
                    break;
                }
            }
            if (_pipeline) {
                //material node check
                MaterialNode* materialNode = nullptr;
                MeshNode* meshNode = nullptr;
                InstanceNode* instanceNode = nullptr;
                for (auto itr : _pipeline->materialNodes) {
                    if (itr->material == _meshInstance.material()) {
                        materialNode = itr;
                        //mesh node check
                        for (auto itr1 : materialNode->meshNodes) {
                            if (itr1->mesh == _meshInstance.mesh()) {
                                meshNode = itr1;
                                //instance check
                                for (auto itr2 : meshNode->instanceNodes) {
                                    if (itr2->instance == &_meshInstance) {
                                        instanceNode = itr2;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
                if (meshNode) {
                    removeFromVector(meshNode->instanceNodes, instanceNode);
                    if (meshNode->instanceNodes.size() == 0) {
                        removeFromVector(materialNode->meshNodes, meshNode);
                        if (materialNode->meshNodes.size() == 0) {
                            removeFromVector(_pipeline->materialNodes, materialNode);
                        }
                    }
                }
            }
        }
};
vector<uint>& InternalScenePublicInterface::GetEntities(Scene& _scene) { return _scene.m_i->m_Entities; }
vector<SunLight*>& InternalScenePublicInterface::GetLights(Scene& _scene) { return _scene.m_i->m_Lights; }

void InternalScenePublicInterface::RenderGeometryOpaque(Scene& _scene,Camera& _camera) {
    for (auto pipeline : _scene.m_i->m_Pipelines[RenderStage::GeometryOpaque]) { 
        pipeline->render(); 
    } 
}
void InternalScenePublicInterface::RenderGeometryTransparent(Scene& _scene, Camera& _camera) {
    for (auto pipeline : _scene.m_i->m_Pipelines[RenderStage::GeometryTransparent]) { 
        pipeline->sort(_camera);
        pipeline->render(); 
    } 
}
void InternalScenePublicInterface::RenderForwardOpaque(Scene& _scene, Camera& _camera) {
    for (auto pipeline : _scene.m_i->m_Pipelines[RenderStage::ForwardOpaque]) { 
        pipeline->render(); 
    }
}
void InternalScenePublicInterface::RenderForwardTransparent(Scene& _scene, Camera& _camera) {
    for (auto pipeline : _scene.m_i->m_Pipelines[RenderStage::ForwardTransparent]) { 
        pipeline->sort(_camera);
        pipeline->render(); 
    }
}

ECS<Entity>& InternalScenePublicInterface::GetECS(Scene& _scene) {
    return _scene.m_i->m_ECS;
}
void InternalScenePublicInterface::AddMeshInstanceToPipeline(Scene& _scene, MeshInstance& _meshInstance, RenderStage::Stage _stage) {
    _scene.m_i->_addMeshInstanceToPipeline(_scene, _meshInstance, _scene.m_i->m_Pipelines[_stage], _stage);
}
void InternalScenePublicInterface::RemoveMeshInstanceFromPipeline(Scene& _scene, MeshInstance& _meshInstance, RenderStage::Stage _stage){
    _scene.m_i->_removeMeshInstanceFromPipeline(_scene, _meshInstance, _scene.m_i->m_Pipelines[_stage], _stage);
}

Scene::Scene(string name):m_i(new impl){
    m_i->_init(*this, name);
    registerEvent(EventType::SceneChanged);
}
uint Scene::id() { return m_i->m_ID; }
uint Scene::OLD_addEntity(OLD_Entity& entity){ return m_i->_OLD_addEntity(*this,entity); }
void Scene::OLD_removeEntity(OLD_Entity& e){ e.destroy(); }
void Scene::OLD_removeEntity(uint id){
    OLD_Entity& e = *OLD_Components::GetEntity(id);
    OLD_removeEntity(e);
}
OLD_Entity* Scene::OLD_getEntity(uint entityID){
    if(entityID == 0) return nullptr;
    return OLD_Components::GetEntity(entityID);
}
bool Scene::OLD_hasEntity(OLD_Entity& entity){ return m_i->_OLD_hasEntity(entity); }
bool Scene::OLD_hasEntity(uint entityID){ return m_i->_OLD_hasEntity(entityID); }
Camera* Scene::getActiveCamera(){ return m_i->m_ActiveCamera; }
void Scene::setActiveCamera(Camera& c){ m_i->m_ActiveCamera = &c; }
void Scene::centerSceneToObject(OLD_Entity& center){ return m_i->_centerToObject(*this, center.ID); }
void Scene::centerSceneToObject(uint centerID) { return m_i->_centerToObject(*this, centerID); }
Scene::~Scene(){
    unregisterEvent(EventType::SceneChanged);
    m_i->_destruct();
}
void Scene::update(const float& dt){
}
glm::vec3 Scene::getBackgroundColor(){ return m_i->m_BackgroundColor; }
SkyboxEmpty* Scene::skybox() const { return m_i->m_Skybox; }
void Scene::setSkybox(SkyboxEmpty* s){ m_i->m_Skybox = s; }
void Scene::setBackgroundColor(float r, float g, float b){ Math::setColor(m_i->m_BackgroundColor,r,g,b); }