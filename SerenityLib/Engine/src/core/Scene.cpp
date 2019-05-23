#include <core/engine/lights/Light.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/Engine_Math.h>
#include <core/MeshInstance.h>
#include <core/Camera.h>
#include <core/Skybox.h>

#include <ecs/Entity.h>
#include <ecs/Components.h>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

uint InternalScenePublicInterface::NumScenes = 0;

struct Scene::impl final {
    SkyboxEmpty*                      m_Skybox;
    Camera*                           m_ActiveCamera;
    glm::vec3                         m_BackgroundColor;
    uint                              m_ID;
    vector<vector<RenderPipeline*>>   m_Pipelines;

    vector<SunLight*>                 m_SunLights;
    vector<DirectionalLight*>         m_DirectionalLights;
    vector<PointLight*>               m_PointLights;
    vector<SpotLight*>                m_SpotLights;
    vector<RodLight*>                 m_RodLights;

    vector<uint>                      m_Entities;

    ECS<Entity>                       m_ECS;

    void _init(Scene& super,string& _name) {
        m_Pipelines.resize(RenderStage::_TOTAL);
        m_Skybox = nullptr;
        m_ActiveCamera = nullptr;
        m_BackgroundColor = glm::vec3(0.0f);
        super.setName(_name);
        Core::m_Engine->m_ResourceManager._addScene(super);
        ++InternalScenePublicInterface::NumScenes;
        m_ID = InternalScenePublicInterface::NumScenes;
            

        m_ECS.assignSystem<ComponentLogic> (ComponentLogic_System());
        m_ECS.assignSystem<ComponentBody>  (ComponentBody_System());
        m_ECS.assignSystem<ComponentLogic1>(ComponentLogic1_System());
        m_ECS.assignSystem<ComponentCamera>(ComponentCamera_System());
        m_ECS.assignSystem<ComponentLogic2>(ComponentLogic2_System());
        m_ECS.assignSystem<ComponentModel> (ComponentModel_System());
        m_ECS.assignSystem<ComponentLogic3>(ComponentLogic3_System());
        m_ECS.assignSystem<ComponentName>  (ComponentName_System());
    }
    void _destruct() {
        SAFE_DELETE(m_Skybox);
        for(auto& vec: m_Pipelines)
            SAFE_DELETE_VECTOR(vec);
    }
    void _centerToObject(Scene& super,Entity& center) {
        //TODO: handle parent->child relationship
        ComponentBody& centerBody = *center.getComponent<ComponentBody>();
        for (auto& data : epriv::InternalScenePublicInterface::GetEntities(super)) {
            Entity e = super.getEntity(data);
            if (e != center) {
                auto* eBody = e.getComponent<ComponentBody>();
                if (eBody) {
                    auto& _eBody = *eBody;
                    _eBody.setPosition(_eBody.position() - centerBody.position());
                }
            }
        }
        centerBody.setPosition(0.0f);
    }
    void _addMeshInstanceToPipeline(Scene& _scene, MeshInstance& _meshInstance, const vector<RenderPipeline*>& _pipelinesList, RenderStage::Stage _stage) {
        epriv::RenderPipeline* _pipeline = nullptr;
        
        for (auto& pipeline : _pipelinesList) {
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
        for (auto& itr : _pipeline->materialNodes) {
            if (itr->material == _meshInstance.material()) {
                materialNode = itr;
                //mesh node check
                for (auto& itr1 : materialNode->meshNodes) {
                    if (itr1->mesh == _meshInstance.mesh()) {
                        meshNode = itr1;
                        //instance check
                        for (auto& itr2 : meshNode->instanceNodes) {
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
        for (auto& pipeline : _pipelinesList) {
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
            for (auto& itr : _pipeline->materialNodes) {
                if (itr->material == _meshInstance.material()) {
                    materialNode = itr;
                    //mesh node check
                    for (auto& itr1 : materialNode->meshNodes) {
                        if (itr1->mesh == _meshInstance.mesh()) {
                            meshNode = itr1;
                            //instance check
                            for (auto& itr2 : meshNode->instanceNodes) {
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
                SAFE_DELETE(instanceNode);
                if (meshNode->instanceNodes.size() == 0) {
                    removeFromVector(materialNode->meshNodes, meshNode);
                    SAFE_DELETE(meshNode);
                    if (materialNode->meshNodes.size() == 0) {
                        removeFromVector(_pipeline->materialNodes, materialNode);
                        SAFE_DELETE(materialNode);
                    }
                }
            }
        }
    }
};
vector<Engine::epriv::EntityPOD>& InternalScenePublicInterface::GetEntities(Scene& _scene) {
    return _scene.m_i->m_ECS.entityPool._pool;
}
vector<SunLight*>& InternalScenePublicInterface::GetSunLights(Scene& _scene) { 
    return _scene.m_i->m_SunLights; 
}
vector<DirectionalLight*>& InternalScenePublicInterface::GetDirectionalLights(Scene& _scene) { 
    return _scene.m_i->m_DirectionalLights; 
}
vector<PointLight*>& InternalScenePublicInterface::GetPointLights(Scene& _scene) { 
    return _scene.m_i->m_PointLights; 
}
vector<SpotLight*>& InternalScenePublicInterface::GetSpotLights(Scene& _scene) { 
    return _scene.m_i->m_SpotLights; 
}
vector<RodLight*>& InternalScenePublicInterface::GetRodLights(Scene& _scene) { 
    return _scene.m_i->m_RodLights; 
}


void InternalScenePublicInterface::RenderGeometryOpaque(Scene& _scene,Camera& _camera) {
    for (auto& pipeline : _scene.m_i->m_Pipelines[RenderStage::GeometryOpaque]) { 
        //pipeline->sort(_camera);
        pipeline->render(); 
    } 
}
void InternalScenePublicInterface::RenderGeometryTransparent(Scene& _scene, Camera& _camera) {
    for (auto& pipeline : _scene.m_i->m_Pipelines[RenderStage::GeometryTransparent]) { 
        pipeline->sort(_camera);
        pipeline->render(); 
    } 
}
void InternalScenePublicInterface::RenderForwardOpaque(Scene& _scene, Camera& _camera) {
    for (auto& pipeline : _scene.m_i->m_Pipelines[RenderStage::ForwardOpaque]) {
        //pipeline->sort(_camera);
        pipeline->render(); 
    }
}
void InternalScenePublicInterface::RenderForwardTransparent(Scene& _scene, Camera& _camera) {
    for (auto& pipeline : _scene.m_i->m_Pipelines[RenderStage::ForwardTransparent]) { 
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


Entity Scene::createEntity() { 
    return m_i->m_ECS.createEntity(*this); 
}
Entity Scene::getEntity(Engine::epriv::EntityPOD& data) { 
    return Entity(data.ID, data.sceneID, data.versionID); 
}
void Scene::removeEntity(uint entityData) { 
    m_i->m_ECS.removeEntity(entityData); 
}
void Scene::removeEntity(Entity& entity) { 
    EntityDataRequest dataRequest(entity);
    m_i->m_ECS.removeEntity(dataRequest.ID);
}

Camera* Scene::getActiveCamera(){ 
    return m_i->m_ActiveCamera; 
}
void Scene::setActiveCamera(Camera& c){ 
    m_i->m_ActiveCamera = &c; 
}
void Scene::centerSceneToObject(Entity& center){ 
    return m_i->_centerToObject(*this, center); 
}
Scene::~Scene(){
    unregisterEvent(EventType::SceneChanged);
    m_i->_destruct();
}
void Scene::update(const float& dt){
}
glm::vec3 Scene::getBackgroundColor(){ 
    return m_i->m_BackgroundColor; 
}
SkyboxEmpty* Scene::skybox() const { 
    return m_i->m_Skybox; 
}
void Scene::setSkybox(SkyboxEmpty* s){ 
    m_i->m_Skybox = s; 
}
void Scene::setBackgroundColor(float r, float g, float b){ 
    Math::setColor(m_i->m_BackgroundColor,r,g,b); 
}