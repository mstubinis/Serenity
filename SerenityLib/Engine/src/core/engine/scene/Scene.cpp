#include <core/engine/scene/Scene.h>
#include <core/engine/scene/SceneOptions.h>
#include <core/engine/lights/Lights.h>
#include <core/engine/Engine.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/model/ModelInstance.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/scene/Skybox.h>
#include <core/engine/scene/Viewport.h>
#include <core/engine/materials/Material.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/renderer/particles/ParticleEmitter.h>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;


struct EmptyOnUpdateFunctor final {void operator()(Scene* scene, const double& dt) const {

}};


epriv::SceneData::SceneData() {
    m_ID = 0;
    m_GI = glm::vec3(1.0f);
    m_Sun = nullptr;
    m_Skybox = nullptr;
}
epriv::SceneData::~SceneData() {
    SAFE_DELETE(m_Skybox);
}

struct Scene::impl final {
    ECS<Entity>                       m_ECS;


    void _init(Scene& super, const string& _name, const SceneOptions& options) {

        m_ECS.assignSystem<ComponentLogic> (ComponentLogic_System_CI());
        m_ECS.assignSystem<ComponentBody>  (ComponentBody_System_CI());
        m_ECS.assignSystem<ComponentLogic1>(ComponentLogic1_System_CI());
        m_ECS.assignSystem<ComponentCamera>(ComponentCamera_System_CI());
        m_ECS.assignSystem<ComponentLogic2>(ComponentLogic2_System_CI());
        m_ECS.assignSystem<ComponentModel> (ComponentModel_System_CI());
        m_ECS.assignSystem<ComponentLogic3>(ComponentLogic3_System_CI());
        m_ECS.assignSystem<ComponentName>  (ComponentName_System_CI());
    }
    void _destruct() {      
    }
    void _centerToObject(Scene& super, const Entity& centerEntity) {
        //TODO: handle parent->child relationship
        auto& centerBody = *const_cast<Entity&>(centerEntity).getComponent<ComponentBody>();
        auto centerPos = centerBody.position();
        glm_vec3 _eBody_pos;
        Entity e;
        for (auto& data : InternalScenePublicInterface::GetEntities(super)) {
            e = super.getEntity(data);
            if (e != centerEntity) {
                auto* eBody = e.getComponent<ComponentBody>();
                if (eBody) {
                    auto& _eBody = *eBody;
                    _eBody_pos = _eBody.position();
                    _eBody.setPosition(_eBody_pos - centerPos);
                }
            }
        }
        for (auto& particle : super.m_Data.m_ParticleSystem.getParticles()) {
            particle.setPosition(particle.position() - glm::vec3(centerPos));
        }


        centerBody.setPosition(static_cast<decimal>(0.0), static_cast<decimal>(0.0), static_cast<decimal>(0.0));
    }
    void _addModelInstanceToPipeline(Scene& _scene, ModelInstance& _modelInstance, const vector<RenderGraph*>& render_graph_list, const RenderStage::Stage& _stage) {
        RenderGraph* renderGraph = nullptr;
        
        for (auto& render_graph_ptr : render_graph_list) {
            if (&render_graph_ptr->shaderProgram == _modelInstance.shaderProgram()) {
                renderGraph = render_graph_ptr;
                break;
            }
        }
        if (!renderGraph) {
            renderGraph = new RenderGraph(*_modelInstance.shaderProgram());
            _scene.m_Data.m_RenderGraphs[_stage].push_back(renderGraph);
        }
        //material node check
        MaterialNode* materialNode = nullptr;
        MeshNode*     meshNode     = nullptr;
        InstanceNode* instanceNode = nullptr;
        for (auto& itr : renderGraph->materialNodes) {
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
            renderGraph->materialNodes.push_back(materialNode);
        }
        if (!meshNode) {
            meshNode = new MeshNode(*_modelInstance.mesh());
            materialNode->meshNodes.push_back(meshNode);
        }
        if (!instanceNode) {
            instanceNode = new InstanceNode(_modelInstance);
            meshNode->instanceNodes.push_back(instanceNode);
            renderGraph->instancesTotal.push_back(instanceNode);
        }
    }
    void _removeModelInstanceFromPipeline(Scene& _scene, ModelInstance& _modelInstance, const vector<RenderGraph*>& render_graph_list, const RenderStage::Stage& _stage) {
        RenderGraph* renderGraph = nullptr;
        for (auto& render_graph_ptr : render_graph_list) {
            if (&render_graph_ptr->shaderProgram == _modelInstance.shaderProgram()) {
                renderGraph = render_graph_ptr;
                break;
            }
        }
        if (renderGraph) {
            //material node check
            MaterialNode* materialNode = nullptr;
            MeshNode*     meshNode     = nullptr;
            InstanceNode* instanceNode = nullptr;
            for (auto& itr : renderGraph->materialNodes) {
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
                removeFromVector(renderGraph->instancesTotal, instanceNode);
                SAFE_DELETE(instanceNode);
                if (meshNode->instanceNodes.size() == 0) {
                    removeFromVector(materialNode->meshNodes, meshNode);
                    SAFE_DELETE(meshNode);
                    if (materialNode->meshNodes.size() == 0) {
                        removeFromVector(renderGraph->materialNodes, materialNode);
                        SAFE_DELETE(materialNode);
                    }
                }
            }
        }
    }
};
vector<Particle>& InternalScenePublicInterface::GetParticles(Scene& scene) {
    return scene.m_Data.m_ParticleSystem.getParticles();
}
vector<Viewport*>& InternalScenePublicInterface::GetViewports(Scene& scene) {
    return scene.m_Data.m_Viewports;
}
vector<Camera*>& InternalScenePublicInterface::GetCameras(Scene& scene) {
    return scene.m_Data.m_Cameras;
}
vector<Engine::epriv::EntityPOD>& InternalScenePublicInterface::GetEntities(Scene& scene) {
    return scene.m_i->m_ECS.m_EntityPool._pool;
}
vector<SunLight*>& InternalScenePublicInterface::GetLights(Scene& scene) {
    return scene.m_Data.m_Lights;
}
vector<SunLight*>& InternalScenePublicInterface::GetSunLights(Scene& scene) { 
    return scene.m_Data.m_SunLights; 
}
vector<DirectionalLight*>& InternalScenePublicInterface::GetDirectionalLights(Scene& scene) { 
    return scene.m_Data.m_DirectionalLights; 
}
vector<PointLight*>& InternalScenePublicInterface::GetPointLights(Scene& scene) { 
    return scene.m_Data.m_PointLights; 
}
vector<SpotLight*>& InternalScenePublicInterface::GetSpotLights(Scene& scene) { 
    return scene.m_Data.m_SpotLights; 
}
vector<RodLight*>& InternalScenePublicInterface::GetRodLights(Scene& scene) { 
    return scene.m_Data.m_RodLights; 
}


void InternalScenePublicInterface::UpdateMaterials(Scene& scene, const double& dt) {
    for (uint i = 0; i < RenderStage::_TOTAL; ++i) {
        for (auto& render_graph_ptr : scene.m_Data.m_RenderGraphs[i]) {
            for (auto& materialNode : render_graph_ptr->materialNodes) {
                auto& _material = *materialNode->material;
                _material.m_UpdatedThisFrame = false;
            }
        }
    }
    for (uint i = 0; i < RenderStage::_TOTAL; ++i) {
        for (auto& render_graph_ptr : scene.m_Data.m_RenderGraphs[i]) {
            for (auto& materialNode : render_graph_ptr->materialNodes) {
                auto& _material = *materialNode->material;
                if (!_material.m_UpdatedThisFrame) {
                    _material.update(dt);
                    _material.m_UpdatedThisFrame = true;
                }
            }
            
        }
    }
}
void InternalScenePublicInterface::UpdateParticleSystem(Scene& scene, const double& dt) {
    scene.m_Data.m_ParticleSystem.update(dt);
}

void InternalScenePublicInterface::RenderGeometryOpaque(Scene& scene, Viewport& viewport, Camera& camera, const bool useDefaultShaders) {
    for (uint i = RenderStage::GeometryOpaque; i < RenderStage::GeometryOpaque_4; ++i) {
        for (auto& render_graph_ptr : scene.m_Data.m_RenderGraphs[i]) {
            auto& render_graph = *render_graph_ptr;
            render_graph.validate_model_instances_for_rendering(viewport, camera);
            render_graph.render(viewport, camera, useDefaultShaders, SortingMode::None);
        }
    }
}
void InternalScenePublicInterface::RenderGeometryTransparent(Scene& scene, Viewport& viewport, Camera& camera, const bool useDefaultShaders) {
    for (uint i = RenderStage::GeometryTransparent; i < RenderStage::GeometryTransparent_4; ++i) {
        for (auto& render_graph_ptr : scene.m_Data.m_RenderGraphs[i]) {
            auto& render_graph = *render_graph_ptr;
            render_graph.sort(camera, SortingMode::BackToFront);
            render_graph.validate_model_instances_for_rendering(viewport, camera);
            render_graph.render(viewport, camera, useDefaultShaders, SortingMode::None);
        }
    }
}
void InternalScenePublicInterface::RenderGeometryTransparentTrianglesSorted(Scene& scene, Viewport& viewport, Camera& camera, const bool useDefaultShaders) {
    for (uint i = RenderStage::GeometryTransparentTrianglesSorted; i < RenderStage::GeometryTransparentTrianglesSorted_4; ++i) {
        for (auto& render_graph_ptr : scene.m_Data.m_RenderGraphs[i]) {
            auto& render_graph = *render_graph_ptr;
            render_graph.sort(camera, SortingMode::FrontToBack);
            render_graph.validate_model_instances_for_rendering(viewport, camera);
            render_graph.render(viewport, camera, useDefaultShaders, SortingMode::FrontToBack);
        }
    }
}
void InternalScenePublicInterface::RenderForwardOpaque(Scene& scene, Viewport& viewport, Camera& camera, const bool useDefaultShaders) {
    for (uint i = RenderStage::ForwardOpaque; i < RenderStage::ForwardOpaque_4; ++i) {
        for (auto& render_graph_ptr : scene.m_Data.m_RenderGraphs[i]) {
            auto& render_graph = *render_graph_ptr;
            render_graph.validate_model_instances_for_rendering(viewport, camera);
            render_graph.render(viewport, camera, useDefaultShaders, SortingMode::None);
        }
    }
}
void InternalScenePublicInterface::RenderForwardTransparent(Scene& scene, Viewport& viewport, Camera& camera, const bool useDefaultShaders) {
    for (uint i = RenderStage::ForwardTransparent; i < RenderStage::ForwardTransparent_4; ++i) {
        for (auto& render_graph_ptr : scene.m_Data.m_RenderGraphs[i]) {
            auto& render_graph = *render_graph_ptr;
            render_graph.sort_bruteforce(camera, SortingMode::BackToFront);
            render_graph.validate_model_instances_for_rendering(viewport, camera);
            render_graph.render_bruteforce(viewport, camera, useDefaultShaders, SortingMode::None);
        }
    }
}
void InternalScenePublicInterface::RenderForwardTransparentTrianglesSorted(Scene& scene, Viewport& viewport, Camera& camera, const bool useDefaultShaders) {
    for (uint i = RenderStage::ForwardTransparentTrianglesSorted; i < RenderStage::ForwardTransparentTrianglesSorted_4; ++i) {
        for (auto& render_graph_ptr : scene.m_Data.m_RenderGraphs[i]) {
            //TODO: sort_bruteforce and render_bruteforce doesn't work here... probably has to do with custom binds and unbinds and custom shader(s)
            auto& render_graph = *render_graph_ptr;
            render_graph.sort(camera, SortingMode::FrontToBack);
            render_graph.validate_model_instances_for_rendering(viewport, camera);
            render_graph.render(viewport, camera, useDefaultShaders, SortingMode::FrontToBack);
        }
    }
}
void InternalScenePublicInterface::RenderForwardParticles(Scene& scene, Viewport& viewport, Camera& camera, const bool useDefaultShaders) {
    for (uint i = RenderStage::ForwardParticles; i < RenderStage::ForwardParticles_4; ++i) {
        for (auto& render_graph_ptr : scene.m_Data.m_RenderGraphs[i]) {
            auto& render_graph = *render_graph_ptr;
            render_graph.sort_cheap_bruteforce(camera, SortingMode::BackToFront);
            render_graph.validate_model_instances_for_rendering(viewport, camera);
            render_graph.render_bruteforce(viewport, camera, useDefaultShaders, SortingMode::None);
        }
    }
}
void InternalScenePublicInterface::RenderDecals(Scene& scene, Viewport& viewport, Camera& camera, const bool useDefaultShaders) {
    for (uint i = RenderStage::Decals; i < RenderStage::Decals_4; ++i) {
        for (auto& render_graph_ptr : scene.m_Data.m_RenderGraphs[i]) {
            auto& render_graph = *render_graph_ptr;
            render_graph.sort(camera, SortingMode::None);
            render_graph.validate_model_instances_for_rendering(viewport, camera);
            render_graph.render(viewport, camera, useDefaultShaders, SortingMode::None);
        }
    }
}
void InternalScenePublicInterface::RenderParticles(Scene& scene, Viewport& viewport, Camera& camera, ShaderProgram& program, GBuffer& gBuffer) {
    scene.m_Data.m_ParticleSystem.render(camera, program, gBuffer);
}


ECS<Entity>& InternalScenePublicInterface::GetECS(Scene& scene) {
    return scene.m_i->m_ECS;
}
void InternalScenePublicInterface::CleanECS(Scene& scene, const uint entityData) {
    for (auto& pipelines : scene.m_Data.m_RenderGraphs) {
        for (auto& pipeline : pipelines) {
            pipeline->clean(entityData);
        }
    }
}

void InternalScenePublicInterface::AddModelInstanceToPipeline(Scene& scene, ModelInstance& modelInstance, const RenderStage::Stage& stage) {
    scene.m_i->_addModelInstanceToPipeline(scene, modelInstance, scene.m_Data.m_RenderGraphs[stage], stage);
}
void InternalScenePublicInterface::RemoveModelInstanceFromPipeline(Scene& scene, ModelInstance& modelInstance, const RenderStage::Stage& stage){
    scene.m_i->_removeModelInstanceFromPipeline(scene, modelInstance, scene.m_Data.m_RenderGraphs[stage], stage);
}

Scene::Scene(const string& name) : Scene(name, SceneOptions::DEFAULT_OPTIONS){

}
Scene::Scene(const string& name, const SceneOptions& options) : m_i(new impl), EngineResource(ResourceType::Scene, name) {
    m_Data.m_ID = Core::m_Engine->m_ResourceManager._addScene(*this);
    setName(name);
    m_i->_init(*this, name, options);
    m_Data.m_RenderGraphs.resize(RenderStage::_TOTAL);
    registerEvent(EventType::SceneChanged);
    setOnUpdateFunctor(EmptyOnUpdateFunctor());
}
Scene::~Scene() {
    unregisterEvent(EventType::SceneChanged);
    m_i->_destruct();
    for (auto& renderGraph : m_Data.m_RenderGraphs)
        SAFE_DELETE_VECTOR(renderGraph);
    SAFE_DELETE_VECTOR(m_Data.m_Viewports);
}
void Scene::setGodRaysSun(Entity* sun) {
    m_Data.m_Sun = sun;
}
Entity* Scene::getGodRaysSun() {
    return m_Data.m_Sun;
}
const unsigned int Scene::numViewports() const {
    return static_cast<unsigned int>(m_Data.m_Viewports.size());
}
const uint Scene::id() const {
    return m_Data.m_ID;
}

ParticleEmitter* Scene::addParticleEmitter(ParticleEmitter& emitter) {
    //auto& i = *m_i;
    return m_Data.m_ParticleSystem.add_emitter(emitter);
}

Viewport& Scene::addViewport(const float x, const float y, const float width, const float height, const Camera& camera) {
    Viewport& viewport = *(new Viewport(*this, camera));
    viewport.setViewportDimensions(x, y, width, height);
    unsigned int id = numViewports();
    viewport.m_ID = id;
    m_Data.m_Viewports.push_back(&viewport);
    return viewport;
}
Entity Scene::createEntity() { 
    return m_i->m_ECS.createEntity(*this); 
}
Entity Scene::getEntity(const Engine::epriv::EntityPOD& data) { 
    return Entity(data.ID, data.sceneID, data.versionID); 
}
void Scene::removeEntity(const unsigned int& entityID) {
    m_i->m_ECS.removeEntity(entityID);
}
void Scene::removeEntity(Entity& entity) { 
    EntityDataRequest dataRequest(entity);
    m_i->m_ECS.removeEntity(dataRequest.ID);
}
Viewport& Scene::getMainViewport() {
    return *m_Data.m_Viewports[0];
}
Camera* Scene::getActiveCamera() const {
    if (m_Data.m_Viewports.size() == 0)
        return nullptr;
    return &(const_cast<Camera&>(m_Data.m_Viewports[0]->getCamera())); 
}
void Scene::setActiveCamera(Camera& camera){
    if (m_Data.m_Viewports.size() == 0) {
        Viewport& viewport = *(new Viewport(*this, camera));
        unsigned int id = numViewports();
        viewport.m_ID = id;
        m_Data.m_Viewports.push_back(&viewport);
        return;
    }
    m_Data.m_Viewports[0]->setCamera(camera);
}
void Scene::centerSceneToObject(const Entity& centerEntity){
    return m_i->_centerToObject(*this, centerEntity);
}
void Scene::update(const double& dt){
    m_OnUpdateFunctor(dt);
}
void Scene::onResize(const unsigned int& width, const unsigned int& height) {

}
void Scene::render() {

}

const glm::vec4& Scene::getBackgroundColor() const {
    return m_Data.m_Viewports[0]->m_BackgroundColor;
}
void Scene::setBackgroundColor(const float& r, const float& g, const float& b, const float& a) {
    Math::setColor(m_Data.m_Viewports[0]->m_BackgroundColor, r, g, b, a);
}
void Scene::setBackgroundColor(const glm::vec4& backgroundColor) {
    setBackgroundColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
}
Skybox* Scene::skybox() const { 
    return m_Data.m_Skybox; 
}
void Scene::setSkybox(Skybox* s){ 
    m_Data.m_Skybox = s; 
}
const glm::vec3& Scene::getGlobalIllumination() const {
    return m_Data.m_GI;
}
void Scene::setGlobalIllumination(const glm::vec3& globalIllumination) {
    setGlobalIllumination(globalIllumination.x, globalIllumination.y, globalIllumination.z);
}
void Scene::setGlobalIllumination(const float global, const float diffuse, const float specular) {
    m_Data.m_GI.x = global;
    m_Data.m_GI.y = diffuse;
    m_Data.m_GI.z = specular;
    Renderer::Settings::Lighting::setGIContribution(m_Data.m_GI.x, m_Data.m_GI.y, m_Data.m_GI.z);
}
void Scene::onEvent(const Event& e) {
    if (e.type == EventType::SceneChanged && e.eventSceneChanged.newScene == this) {
        Renderer::Settings::Lighting::setGIContribution(m_Data.m_GI.x, m_Data.m_GI.y, m_Data.m_GI.z);
    }
}
