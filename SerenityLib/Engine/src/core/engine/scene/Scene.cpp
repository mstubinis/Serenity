#include <core/engine/scene/Scene.h>
#include <core/engine/lights/Lights.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/math/Engine_Math.h>
#include <core/ModelInstance.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/scene/Skybox.h>
#include <core/engine/scene/Viewport.h>

#include <ecs/Entity.h>
#include <ecs/Components.h>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

uint InternalScenePublicInterface::NumScenes = 0;

struct EmptyOnUpdateFunctor final {void operator()(Scene* scene, const double& dt) const {

}};

struct Scene::impl final {
    glm::vec3                         m_GI;
    uint                              m_ID;

    Skybox*                           m_Skybox;
    vector<Viewport*>                 m_Viewports;
    vector<Camera*>                   m_Cameras;
    vector<vector<RenderPipeline*>>   m_Pipelines;

    vector<SunLight*>                 m_Lights;
    vector<SunLight*>                 m_SunLights;
    vector<DirectionalLight*>         m_DirectionalLights;
    vector<PointLight*>               m_PointLights;
    vector<SpotLight*>                m_SpotLights;
    vector<RodLight*>                 m_RodLights;

    vector<uint>                      m_Entities;

    ECS<Entity>                       m_ECS;
    Entity*                           m_Sun;

    void _init(Scene& super, const string& _name) {
        m_GI              = glm::vec3(1.0f);
        m_Skybox          = nullptr;
        m_Sun             = nullptr;

        m_Pipelines.resize(RenderStage::_TOTAL);
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
        SAFE_DELETE_VECTOR(m_Viewports);
        for(auto& pipeline: m_Pipelines)
            SAFE_DELETE_VECTOR(pipeline);
    }
    void _centerToObject(Scene& super, const Entity& centerEntity) {
        //TODO: handle parent->child relationship
        ComponentBody& centerBody = *const_cast<Entity&>(centerEntity).getComponent<ComponentBody>();
        auto& entities = InternalScenePublicInterface::GetEntities(super);
        for (auto& data : entities) {
            Entity e = super.getEntity(data);
            if (e != centerEntity) {
                auto* eBody = e.getComponent<ComponentBody>();
                if (eBody) {
                    auto& _eBody = *eBody;
                    _eBody.setPosition(_eBody.position() - centerBody.position());
                }
            }
        }
        centerBody.setPosition(0.0f);
    }
    void _addModelInstanceToPipeline(Scene& _scene, ModelInstance& _modelInstance, const vector<RenderPipeline*>& _pipelinesList, const RenderStage::Stage& _stage) {
        RenderPipeline* _pipeline = nullptr;
        
        for (auto& pipeline : _pipelinesList) {
            if (&pipeline->shaderProgram == _modelInstance.shaderProgram()) {
                _pipeline = pipeline;
                break;
            }
        }
        if (!_pipeline) {
            _pipeline = new RenderPipeline(*_modelInstance.shaderProgram());
            _scene.m_i->m_Pipelines[_stage].push_back(_pipeline);
        }
        //material node check
        MaterialNode* materialNode = nullptr;
        MeshNode*     meshNode     = nullptr;
        InstanceNode* instanceNode = nullptr;
        for (auto& itr : _pipeline->materialNodes) {
            if (itr->material == _modelInstance.material()) {
                materialNode = itr;
                //mesh node check
                for (auto& itr1 : materialNode->meshNodes) {
                    if (itr1->mesh == _modelInstance.mesh()) {
                        meshNode = itr1;
                        //instance check
                        for (auto& itr2 : meshNode->instanceNodes) {
                            if (itr2->instance == &_modelInstance) {
                                instanceNode = itr2;
                                break;
                            }
                        }
                    }
                }
            }
        }
        if (!materialNode) {
            materialNode = new MaterialNode(*_modelInstance.material());
            _pipeline->materialNodes.push_back(materialNode);
        }
        if (!meshNode) {
            meshNode = new MeshNode(*_modelInstance.mesh());
            materialNode->meshNodes.push_back(meshNode);
        }
        if (!instanceNode) {
            instanceNode = new InstanceNode(_modelInstance);
            meshNode->instanceNodes.push_back(instanceNode);
            _pipeline->instancesTotal.push_back(instanceNode);
        }
    }
    void _removeModelInstanceFromPipeline(Scene& _scene, ModelInstance& _modelInstance, const vector<RenderPipeline*>& _pipelinesList, const RenderStage::Stage& _stage) {
        RenderPipeline* _pipeline = nullptr;
        for (auto& pipeline : _pipelinesList) {
            if (&pipeline->shaderProgram == _modelInstance.shaderProgram()) {
                _pipeline = pipeline;
                break;
            }
        }
        if (_pipeline) {
            //material node check
            MaterialNode* materialNode = nullptr;
            MeshNode*     meshNode     = nullptr;
            InstanceNode* instanceNode = nullptr;
            for (auto& itr : _pipeline->materialNodes) {
                if (itr->material == _modelInstance.material()) {
                    materialNode = itr;
                    //mesh node check
                    for (auto& itr1 : materialNode->meshNodes) {
                        if (itr1->mesh == _modelInstance.mesh()) {
                            meshNode = itr1;
                            //instance check
                            for (auto& itr2 : meshNode->instanceNodes) {
                                if (itr2->instance == &_modelInstance) {
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
                removeFromVector(_pipeline->instancesTotal, instanceNode);
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
vector<Viewport*>& InternalScenePublicInterface::GetViewports(Scene& scene) {
    return scene.m_i->m_Viewports;
}
vector<Camera*>& InternalScenePublicInterface::GetCameras(Scene& scene) {
    return scene.m_i->m_Cameras;
}
vector<Engine::epriv::EntityPOD>& InternalScenePublicInterface::GetEntities(Scene& scene) {
    return scene.m_i->m_ECS.entityPool._pool;
}
vector<SunLight*>& InternalScenePublicInterface::GetLights(Scene& scene) {
    return scene.m_i->m_Lights;
}
vector<SunLight*>& InternalScenePublicInterface::GetSunLights(Scene& scene) { 
    return scene.m_i->m_SunLights; 
}
vector<DirectionalLight*>& InternalScenePublicInterface::GetDirectionalLights(Scene& scene) { 
    return scene.m_i->m_DirectionalLights; 
}
vector<PointLight*>& InternalScenePublicInterface::GetPointLights(Scene& scene) { 
    return scene.m_i->m_PointLights; 
}
vector<SpotLight*>& InternalScenePublicInterface::GetSpotLights(Scene& scene) { 
    return scene.m_i->m_SpotLights; 
}
vector<RodLight*>& InternalScenePublicInterface::GetRodLights(Scene& scene) { 
    return scene.m_i->m_RodLights; 
}


void InternalScenePublicInterface::RenderGeometryOpaque(Scene& scene, Viewport& viewport, Camera& camera, const double& dt, const bool useDefaultShaders) {
    for (uint i = RenderStage::GeometryOpaque; i < RenderStage::GeometryOpaque_4; ++i) {
        for (auto& pipeline : scene.m_i->m_Pipelines[i]) {
            pipeline->cpu_execute(viewport, camera, dt);
            pipeline->render(viewport, camera, dt, useDefaultShaders, SortingMode::None);
        }
    }
}
void InternalScenePublicInterface::RenderGeometryTransparent(Scene& scene, Viewport& viewport, Camera& camera, const double& dt, const bool useDefaultShaders) {
    for (uint i = RenderStage::GeometryTransparent; i < RenderStage::GeometryTransparent_4; ++i) {
        for (auto& pipeline : scene.m_i->m_Pipelines[i]) {
            pipeline->sort(camera, SortingMode::BackToFront);
            pipeline->cpu_execute(viewport, camera, dt);
            pipeline->render(viewport, camera, dt, useDefaultShaders, SortingMode::None);
        }
    }
}
void InternalScenePublicInterface::RenderGeometryTransparentTrianglesSorted(Scene& scene, Viewport& viewport, Camera& camera, const double& dt, const bool useDefaultShaders) {
    for (uint i = RenderStage::GeometryTransparentTrianglesSorted; i < RenderStage::GeometryTransparentTrianglesSorted_4; ++i) {
        for (auto& pipeline : scene.m_i->m_Pipelines[i]) {
            pipeline->sort(camera, SortingMode::FrontToBack);
            pipeline->cpu_execute(viewport, camera, dt);
            pipeline->render(viewport, camera, dt, useDefaultShaders, SortingMode::FrontToBack);
        }
    }
}
void InternalScenePublicInterface::RenderForwardOpaque(Scene& scene, Viewport& viewport, Camera& camera, const double& dt, const bool useDefaultShaders) {
    for (uint i = RenderStage::ForwardOpaque; i < RenderStage::ForwardOpaque_4; ++i) {
        for (auto& pipeline : scene.m_i->m_Pipelines[i]) {
            pipeline->cpu_execute(viewport, camera, dt);
            pipeline->render(viewport, camera, dt, useDefaultShaders, SortingMode::None);
        }
    }
}
void InternalScenePublicInterface::RenderForwardTransparent(Scene& scene, Viewport& viewport, Camera& camera, const double& dt, const bool useDefaultShaders) {
    for (uint i = RenderStage::ForwardTransparent; i < RenderStage::ForwardTransparent_4; ++i) {
        for (auto& pipeline : scene.m_i->m_Pipelines[i]) {
            pipeline->sort_bruteforce(camera, SortingMode::BackToFront);
            pipeline->cpu_execute(viewport, camera, dt);
            pipeline->render_bruteforce(viewport, camera, dt, useDefaultShaders, SortingMode::None);
        }
    }
}
void InternalScenePublicInterface::RenderForwardTransparentTrianglesSorted(Scene& scene, Viewport& viewport, Camera& camera, const double& dt, const bool useDefaultShaders) {
    for (uint i = RenderStage::ForwardTransparentTrianglesSorted; i < RenderStage::ForwardTransparentTrianglesSorted_4; ++i) {
        for (auto& pipeline : scene.m_i->m_Pipelines[i]) {
            //TODO: sort_bruteforce and render_bruteforce doesn't work here... probably has to do with custom binds and unbinds and custom shader(s)
            pipeline->sort(camera, SortingMode::FrontToBack);
            pipeline->cpu_execute(viewport, camera, dt);
            pipeline->render(viewport, camera, dt, useDefaultShaders, SortingMode::FrontToBack);
        }
    }
}
void InternalScenePublicInterface::RenderForwardParticles(Scene& scene, Viewport& viewport, Camera& camera, const double& dt, const bool useDefaultShaders) {
    for (uint i = RenderStage::ForwardParticles; i < RenderStage::ForwardParticles_4; ++i) {
        for (auto& pipeline : scene.m_i->m_Pipelines[i]) {
            pipeline->sort_cheap_bruteforce(camera, SortingMode::BackToFront);
            pipeline->cpu_execute(viewport, camera, dt);
            pipeline->render_bruteforce(viewport, camera, dt, useDefaultShaders, SortingMode::None);
        }
    }
}
void InternalScenePublicInterface::RenderDecals(Scene& scene, Viewport& viewport, Camera& camera, const double& dt, const bool useDefaultShaders) {
    for (uint i = RenderStage::Decals; i < RenderStage::Decals_4; ++i) {
        for (auto& pipeline : scene.m_i->m_Pipelines[i]) {
            pipeline->sort(camera, SortingMode::None);
            pipeline->cpu_execute(viewport, camera, dt);
            pipeline->render(viewport, camera, dt, useDefaultShaders, SortingMode::None);
        }
    }
}

ECS<Entity>& InternalScenePublicInterface::GetECS(Scene& scene) {
    return scene.m_i->m_ECS;
}
void InternalScenePublicInterface::CleanECS(Scene& scene, const uint entityData) {
    for (auto& pipelines : scene.m_i->m_Pipelines) {
        for (auto& pipeline : pipelines) {
            pipeline->clean(entityData);
        }
    }
}

void InternalScenePublicInterface::AddModelInstanceToPipeline(Scene& scene, ModelInstance& modelInstance, const RenderStage::Stage& stage) {
    scene.m_i->_addModelInstanceToPipeline(scene, modelInstance, scene.m_i->m_Pipelines[stage], stage);
}
void InternalScenePublicInterface::RemoveModelInstanceFromPipeline(Scene& scene, ModelInstance& modelInstance, const RenderStage::Stage& stage){
    scene.m_i->_removeModelInstanceFromPipeline(scene, modelInstance, scene.m_i->m_Pipelines[stage], stage);
}

Scene::Scene(const string& name):m_i(new impl){
    m_i->_init(*this, name);
    registerEvent(EventType::SceneChanged);
    setOnUpdateFunctor(EmptyOnUpdateFunctor());
}
Scene::~Scene() {
    unregisterEvent(EventType::SceneChanged);
    m_i->_destruct();
}
void Scene::setGodRaysSun(Entity* sun) {
    m_i->m_Sun = sun;
}
Entity* Scene::getGodRaysSun() {
    return m_i->m_Sun;
}
const uint Scene::numViewports() const {
    return m_i->m_Viewports.size();
}
const uint Scene::id() const {
    return m_i->m_ID;
}
Viewport& Scene::addViewport(const float x, const float y, const float width, const float height, const Camera& camera) {
    Viewport* viewport = new Viewport(*this, camera);
    viewport->setViewportDimensions(x, y, width, height);
    viewport->m_ID = static_cast<unsigned short>(numViewports());
    m_i->m_Viewports.push_back(viewport);
    return *viewport;
}
Entity Scene::createEntity() { 
    return m_i->m_ECS.createEntity(*this); 
}
Entity Scene::getEntity(const Engine::epriv::EntityPOD& data) { 
    return Entity(data.ID, data.sceneID, data.versionID); 
}
void Scene::removeEntity(const uint& entityID) {
    m_i->m_ECS.removeEntity(entityID);
}
void Scene::removeEntity(Entity& entity) { 
    EntityDataRequest dataRequest(entity);
    m_i->m_ECS.removeEntity(dataRequest.ID);
}
Viewport& Scene::getMainViewport() {
    return *m_i->m_Viewports[0];
}
Camera* Scene::getActiveCamera() const {
    if (m_i->m_Viewports.size() == 0)
        return nullptr;
    return &(const_cast<Camera&>(m_i->m_Viewports[0]->getCamera())); 
}
void Scene::setActiveCamera(Camera& camera){
    if (m_i->m_Viewports.size() == 0) {
        Viewport* viewport = new Viewport(*this, camera);
        m_i->m_Viewports.push_back(viewport);
        return;
    }
    m_i->m_Viewports[0]->setCamera(camera);
}
void Scene::centerSceneToObject(const Entity& centerEntity){
    return m_i->_centerToObject(*this, centerEntity);
}
void Scene::update(const double& dt){
    m_OnUpdateFunctor(dt);
}
const glm::vec4& Scene::getBackgroundColor() const {
    return m_i->m_Viewports[0]->m_BackgroundColor;
}
void Scene::setBackgroundColor(const float& r, const float& g, const float& b, const float& a) {
    Math::setColor(m_i->m_Viewports[0]->m_BackgroundColor, r, g, b, a);
}
void Scene::setBackgroundColor(const glm::vec4& backgroundColor) {
    setBackgroundColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
}
Skybox* Scene::skybox() const { 
    return m_i->m_Skybox; 
}
void Scene::setSkybox(Skybox* s){ 
    m_i->m_Skybox = s; 
}
const glm::vec3& Scene::getGlobalIllumination() const {
    return m_i->m_GI;
}
void Scene::setGlobalIllumination(const glm::vec3& globalIllumination) {
    setGlobalIllumination(globalIllumination.x, globalIllumination.y, globalIllumination.z);
}
void Scene::setGlobalIllumination(const float global, const float diffuse, const float specular) {
    auto& i  = *m_i;
    i.m_GI.x = global;
    i.m_GI.y = diffuse;
    i.m_GI.z = specular;
    Renderer::Settings::Lighting::setGIContribution(i.m_GI.x, i.m_GI.y, i.m_GI.z);
}
void Scene::onEvent(const Event& e) {
    if (e.type == EventType::SceneChanged && e.eventSceneChanged.newScene == this) {
        auto& i = *m_i;
        Renderer::Settings::Lighting::setGIContribution(i.m_GI.x, i.m_GI.y, i.m_GI.z);
    }
}
