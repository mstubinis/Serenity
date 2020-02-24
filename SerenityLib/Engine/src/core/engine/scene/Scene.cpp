#include <core/engine/scene/Scene.h>
#include <core/engine/scene/SceneOptions.h>
#include <core/engine/system/Engine.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/model/ModelInstance.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/scene/Skybox.h>
//#include <core/engine/scene/Viewport.h>
#include <core/engine/materials/Material.h>
#include <core/engine/renderer/particles/ParticleEmitter.h>
#include <core/engine/renderer/RenderGraph.h>
#include <core/engine/lights/Lights.h>

using namespace Engine;
using namespace std;


struct EmptyOnUpdateFunctor final {void operator()(Scene* scene, const float& dt) const {

}};

class Scene::impl final {
    public:
        Engine::priv::ParticleSystem          m_ParticleSystem;
        priv::ECS<Entity>                     m_ECS;

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
            for (auto& data : priv::InternalScenePublicInterface::GetEntities(super)) {
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
            for (auto& particle : super.m_i->m_ParticleSystem.getParticles()) {
                particle.setPosition(particle.position() - glm::vec3(centerPos));
            }
            centerBody.setPosition(static_cast<decimal>(0.0), static_cast<decimal>(0.0), static_cast<decimal>(0.0));
        }
        void _addModelInstanceToPipeline(Scene& scene, ModelInstance& modelInstance, const vector<priv::RenderGraph>& render_graph_list, const RenderStage::Stage& stage) {
            priv::RenderGraph* renderGraph = nullptr;
        
            for (auto& render_graph_ptr : render_graph_list) {
                if (render_graph_ptr.m_ShaderProgram == modelInstance.shaderProgram()) {
                    renderGraph = &const_cast<priv::RenderGraph&>(render_graph_ptr);
                    break;
                }
            }
            if (!renderGraph) {
                renderGraph = &scene.m_RenderGraphs[stage].emplace_back(*modelInstance.shaderProgram());
            }
            renderGraph->addModelInstanceToPipeline(modelInstance);
        }
        void _removeModelInstanceFromPipeline(ModelInstance& modelInstance, const vector<priv::RenderGraph>& render_graph_list) {
            priv::RenderGraph* renderGraph = nullptr;
            for (auto& render_graph_ptr : render_graph_list) {
                if (render_graph_ptr.m_ShaderProgram == modelInstance.shaderProgram()) {
                    renderGraph = &const_cast<priv::RenderGraph&>(render_graph_ptr);
                    break;
                }
            }
            if (renderGraph) {
                renderGraph->removeModelInstanceFromPipeline(modelInstance);
            }
        }
};
vector<Particle>& priv::InternalScenePublicInterface::GetParticles(const Scene& scene) {
    return scene.m_i->m_ParticleSystem.getParticles();
}
vector<Viewport>& priv::InternalScenePublicInterface::GetViewports(const Scene& scene) {
    return scene.m_Viewports;
}
vector<Camera*>& priv::InternalScenePublicInterface::GetCameras(const Scene& scene) {
    return scene.m_Cameras;
}
vector<Engine::priv::EntityPOD>& priv::InternalScenePublicInterface::GetEntities(const Scene& scene) {
    return scene.m_i->m_ECS.m_EntityPool.m_Pool;
}
vector<SunLight*>& priv::InternalScenePublicInterface::GetLights(const Scene& scene) {
    return scene.m_Lights;
}
vector<SunLight*>& priv::InternalScenePublicInterface::GetSunLights(const Scene& scene) {
    return scene.m_SunLights;
}
vector<DirectionalLight*>& priv::InternalScenePublicInterface::GetDirectionalLights(const Scene& scene) {
    return scene.m_DirectionalLights;
}
vector<PointLight*>& priv::InternalScenePublicInterface::GetPointLights(const Scene& scene) {
    return scene.m_PointLights;
}
vector<SpotLight*>& priv::InternalScenePublicInterface::GetSpotLights(const Scene& scene) {
    return scene.m_SpotLights;
}
vector<RodLight*>& priv::InternalScenePublicInterface::GetRodLights(const Scene& scene) {
    return scene.m_RodLights;
}


void priv::InternalScenePublicInterface::UpdateMaterials(Scene& scene, const float& dt) {
    for (uint i = 0; i < RenderStage::_TOTAL; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            for (auto& materialNode : render_graph_ptr.m_MaterialNodes) {
                auto& _material = *materialNode.material;
                _material.m_UpdatedThisFrame = false;
            }
        }
    }
    for (uint i = 0; i < RenderStage::_TOTAL; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            for (auto& materialNode : render_graph_ptr.m_MaterialNodes) {
                auto& _material = *materialNode.material;
                if (!_material.m_UpdatedThisFrame) {
                    _material.update(dt);
                    _material.m_UpdatedThisFrame = true;
                }
            }
            
        }
    }
}
void priv::InternalScenePublicInterface::UpdateParticleSystem(Scene& scene, const float& dt) {
    auto& camera = *scene.getActiveCamera();
    scene.m_i->m_ParticleSystem.update(dt, camera);
}

void priv::InternalScenePublicInterface::RenderGeometryOpaque( Renderer& renderer, const Scene& scene, const Viewport& viewport, const Camera& camera, const bool useDefaultShaders) {
    for (uint i = RenderStage::GeometryOpaque; i < RenderStage::GeometryOpaque_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            auto& render_graph = const_cast<RenderGraph&>(render_graph_ptr);
            render_graph.validate_model_instances_for_rendering(viewport, camera);
            render_graph.render(renderer, viewport, camera, useDefaultShaders, SortingMode::None);
        }
    }
}
void priv::InternalScenePublicInterface::RenderGeometryTransparent( Renderer& renderer, const Scene& scene, const Viewport& viewport, const Camera& camera, const bool useDefaultShaders) {
    for (uint i = RenderStage::GeometryTransparent; i < RenderStage::GeometryTransparent_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            auto& render_graph = const_cast<RenderGraph&>(render_graph_ptr);
            render_graph.sort(camera, SortingMode::BackToFront);
            render_graph.validate_model_instances_for_rendering(viewport, camera);
            render_graph.render(renderer, viewport, camera, useDefaultShaders, SortingMode::None);
        }
    }
}
void priv::InternalScenePublicInterface::RenderGeometryTransparentTrianglesSorted( Renderer& renderer, const Scene& scene, const Viewport& viewport, const Camera& camera, const bool useDefaultShaders) {
    for (uint i = RenderStage::GeometryTransparentTrianglesSorted; i < RenderStage::GeometryTransparentTrianglesSorted_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            auto& render_graph = const_cast<RenderGraph&>(render_graph_ptr);
            render_graph.sort(camera, SortingMode::FrontToBack);
            render_graph.validate_model_instances_for_rendering(viewport, camera);
            render_graph.render(renderer, viewport, camera, useDefaultShaders, SortingMode::FrontToBack);
        }
    }
}
void priv::InternalScenePublicInterface::RenderForwardOpaque( Renderer& renderer, const Scene& scene, const Viewport& viewport, const Camera& camera, const bool useDefaultShaders) {
    for (uint i = RenderStage::ForwardOpaque; i < RenderStage::ForwardOpaque_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            auto& render_graph = const_cast<RenderGraph&>(render_graph_ptr);
            render_graph.validate_model_instances_for_rendering(viewport, camera);
            render_graph.render(renderer, viewport, camera, useDefaultShaders, SortingMode::None);
        }
    }
}
void priv::InternalScenePublicInterface::RenderForwardTransparent( Renderer& renderer, const Scene& scene, const Viewport& viewport, const Camera& camera, const bool useDefaultShaders) {
    for (uint i = RenderStage::ForwardTransparent; i < RenderStage::ForwardTransparent_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            auto& render_graph = const_cast<RenderGraph&>(render_graph_ptr);
            render_graph.sort_bruteforce(camera, SortingMode::BackToFront);
            render_graph.validate_model_instances_for_rendering(viewport, camera);
            render_graph.render_bruteforce(renderer, viewport, camera, useDefaultShaders, SortingMode::None);
        }
    }
}
void priv::InternalScenePublicInterface::RenderForwardTransparentTrianglesSorted( Renderer& renderer, const Scene& scene, const Viewport& viewport, const Camera& camera, const bool useDefaultShaders) {
    for (uint i = RenderStage::ForwardTransparentTrianglesSorted; i < RenderStage::ForwardTransparentTrianglesSorted_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            //TODO: sort_bruteforce and render_bruteforce doesn't work here... probably has to do with custom binds and unbinds and custom shader(s)
            auto& render_graph = const_cast<RenderGraph&>(render_graph_ptr);
            render_graph.sort(camera, SortingMode::FrontToBack);
            render_graph.validate_model_instances_for_rendering(viewport, camera);
            render_graph.render(renderer, viewport, camera, useDefaultShaders, SortingMode::FrontToBack);
        }
    }
}
void priv::InternalScenePublicInterface::RenderForwardParticles( Renderer& renderer, const Scene& scene, const Viewport& viewport, const Camera& camera, const bool useDefaultShaders) {
    for (uint i = RenderStage::ForwardParticles; i < RenderStage::ForwardParticles_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            auto& render_graph = const_cast<RenderGraph&>(render_graph_ptr);
            render_graph.sort_cheap_bruteforce(camera, SortingMode::BackToFront);
            render_graph.validate_model_instances_for_rendering(viewport, camera);
            render_graph.render_bruteforce(renderer, viewport, camera, useDefaultShaders, SortingMode::None);
        }
    }
}
void priv::InternalScenePublicInterface::RenderDecals( Renderer& renderer, const Scene& scene, const Viewport& viewport, const Camera& camera, const bool useDefaultShaders) {
    for (uint i = RenderStage::Decals; i < RenderStage::Decals_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            auto& render_graph = const_cast<RenderGraph&>(render_graph_ptr);
            render_graph.sort(camera, SortingMode::None);
            render_graph.validate_model_instances_for_rendering(viewport, camera);
            render_graph.render(renderer, viewport, camera, useDefaultShaders, SortingMode::None);
        }
    }
}
void priv::InternalScenePublicInterface::RenderParticles( Renderer& renderer, const Scene& scene, const Viewport& viewport, const Camera& camera, ShaderProgram& program, const GBuffer& gBuffer) {
    scene.m_i->m_ParticleSystem.render(camera, program, gBuffer);
}


priv::ECS<Entity>& priv::InternalScenePublicInterface::GetECS(const Scene& scene) {
    return scene.m_i->m_ECS;
}
void priv::InternalScenePublicInterface::CleanECS(Scene& scene, const uint entityData) {
    for (auto& pipelines : scene.m_RenderGraphs) {
        for (auto& graph : pipelines) {
            graph.clean(entityData);
        }
    }
}

void priv::InternalScenePublicInterface::AddModelInstanceToPipeline(Scene& scene, ModelInstance& modelInstance, const RenderStage::Stage& stage) {
    scene.m_i->_addModelInstanceToPipeline(scene, modelInstance, scene.m_RenderGraphs[stage], stage);
}
void priv::InternalScenePublicInterface::RemoveModelInstanceFromPipeline(Scene& scene, ModelInstance& modelInstance, const RenderStage::Stage& stage){
    scene.m_i->_removeModelInstanceFromPipeline(modelInstance, scene.m_RenderGraphs[stage]);
}

Scene::Scene(const string& name) : Scene(name, SceneOptions::DEFAULT_OPTIONS){

}
Scene::Scene(const string& name, const SceneOptions& options) : EngineResource(ResourceType::Scene, name) {
    m_RenderGraphs.resize(RenderStage::_TOTAL);
    m_GI      = glm::vec3(1.0f);
    m_Sun     = nullptr;
    m_Skybox  = nullptr;
    m_ID      = priv::Core::m_Engine->m_ResourceManager.AddScene(*this);
    m_i       = NEW impl();
    m_i->_init(*this, name, options);
    setName(name);
    setOnUpdateFunctor(EmptyOnUpdateFunctor());
    registerEvent(EventType::SceneChanged);
}
Scene::~Scene() {
    m_i->_destruct();
    SAFE_DELETE(m_i);
    SAFE_DELETE(m_Skybox);
    SAFE_DELETE_VECTOR(m_Lights);
    SAFE_DELETE_VECTOR(m_Cameras);
    unregisterEvent(EventType::SceneChanged);
}
void Scene::setGodRaysSun(Entity* sun) {
    m_Sun = sun;
}
Entity* Scene::getGodRaysSun() const {
    return m_Sun;
}
const unsigned int Scene::numViewports() const {
    return static_cast<unsigned int>(m_Viewports.size());
}
const uint Scene::id() const {
    return m_ID;
}

ParticleEmitter* Scene::addParticleEmitter(ParticleEmitter& emitter) {
    return m_i->m_ParticleSystem.add_emitter(emitter);
}

Viewport& Scene::addViewport(const float x, const float y, const float width, const float height, const Camera& camera) {
    const unsigned int id = numViewports();
    auto& viewport = m_Viewports.emplace_back(*this, camera);
    viewport.m_ID = id;
    viewport.setViewportDimensions(x, y, width, height);
    return viewport;
}
Entity Scene::createEntity() { 
    return m_i->m_ECS.createEntity(*this); 
}
Entity Scene::getEntity(const Engine::priv::EntityPOD& data) { 
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
    return m_Viewports[0];
}
Camera* Scene::getActiveCamera() const {
    if (m_Viewports.size() == 0)
        return nullptr;
    return &(const_cast<Camera&>(m_Viewports[0].getCamera()));
}
void Scene::setActiveCamera(Camera& camera){
    if (m_Viewports.size() == 0) {
        const unsigned int id = numViewports();
        auto& viewport = m_Viewports.emplace_back(*this, camera);
        viewport.m_ID = id;
        return;
    }
    m_Viewports[0].setCamera(camera);
}
void Scene::centerSceneToObject(const Entity& centerEntity){
    return m_i->_centerToObject(*this, centerEntity);
}
void Scene::update(const float& dt){
    m_OnUpdateFunctor(this, dt);
}
void Scene::onResize(const unsigned int& width, const unsigned int& height) {

}
void Scene::render() {

}

const glm::vec4& Scene::getBackgroundColor() const {
    return m_Viewports[0].m_BackgroundColor;
}
void Scene::setBackgroundColor(const float& r, const float& g, const float& b, const float& a) {
    Math::setColor(m_Viewports[0].m_BackgroundColor, r, g, b, a);
}
void Scene::setBackgroundColor(const glm::vec4& backgroundColor) {
    setBackgroundColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
}
Skybox* Scene::skybox() const { 
    return m_Skybox;
}
void Scene::setSkybox(Skybox* s){ 
    m_Skybox = s;
}
const glm::vec3& Scene::getGlobalIllumination() const {
    return m_GI;
}
void Scene::setGlobalIllumination(const glm::vec3& globalIllumination) {
    setGlobalIllumination(globalIllumination.x, globalIllumination.y, globalIllumination.z);
}
void Scene::setGlobalIllumination(const float global, const float diffuse, const float specular) {
    m_GI.x = global;
    m_GI.y = diffuse;
    m_GI.z = specular;
    Engine::Renderer::Settings::Lighting::setGIContribution(global, diffuse, specular);
}
void Scene::onEvent(const Event& e) {
    if (e.type == EventType::SceneChanged && e.eventSceneChanged.newScene == this) {
        Engine::Renderer::Settings::Lighting::setGIContribution(m_GI.x, m_GI.y, m_GI.z);
    }
}
