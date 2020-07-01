#include <core/engine/scene/Scene.h>
#include <core/engine/scene/SceneOptions.h>
#include <core/engine/system/Engine.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/model/ModelInstance.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/scene/Skybox.h>
#include <core/engine/materials/Material.h>
#include <core/engine/renderer/particles/ParticleSystem.h>
#include <core/engine/renderer/RenderGraph.h>
#include <core/engine/lights/Lights.h>

using namespace Engine;
using namespace std;

class Scene::impl final {
    public:
        Engine::priv::ParticleSystem          m_ParticleSystem;
        priv::ECS<Entity>                     m_ECS;

        void _init(Scene& super, const string& name, const SceneOptions& options) {
            m_ECS.assignSystem<ComponentBody, Engine::priv::ComponentBody_System>(ComponentBody_System_CI()/*, 10000*/);
            m_ECS.assignSystem<ComponentLogic> (ComponentLogic_System_CI()/*, 20000*/);
            m_ECS.assignSystem<ComponentLogic1>(ComponentLogic1_System_CI()/*, 30000*/);
            m_ECS.assignSystem<ComponentLogic2>(ComponentLogic2_System_CI()/*, 40000*/);
            m_ECS.assignSystem<ComponentLogic3>(ComponentLogic3_System_CI()/*, 50000*/);
            m_ECS.assignSystem<ComponentModel>(ComponentModel_System_CI()/*, 60000*/);
            m_ECS.assignSystem<ComponentCamera>(ComponentCamera_System_CI()/*, 70000*/);
            m_ECS.assignSystem<ComponentName>  (ComponentName_System_CI()/*, 80000*/);
        }
        void _destruct() {

        }
        void _centerToObject(Scene& super, Entity centerEntity) {
            ComponentBody* centerBody = centerEntity.getComponent<ComponentBody>();
            auto centerPos            = centerBody->getPosition();
            auto centerPosFloat       = glm::vec3(centerPos);
            for (const auto e : priv::InternalScenePublicInterface::GetEntities(super)) {
                if (e != centerEntity) {
                    ComponentBody* eBody = e.getComponent<ComponentBody>();
                    if (eBody) {
                        if (!eBody->hasParent()) {
                            eBody->setPosition(eBody->getPosition() - centerPos);
                        }
                    }
                }
            }
            for (auto& particle : super.m_i->m_ParticleSystem.getParticles()) {
                if (particle.isActive()) {
                    particle.setPosition(particle.position() - centerPosFloat);
                }
            }
            centerBody->setPosition(static_cast<decimal>(0.0), static_cast<decimal>(0.0), static_cast<decimal>(0.0));

            ComponentBody::recalculateAllParentChildMatrices(super);
        }
        void _addModelInstanceToPipeline(Scene& scene, ModelInstance& modelInstance, const vector<priv::RenderGraph>& render_graph_list, RenderStage::Stage stage) {
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
vector<Entity>& priv::InternalScenePublicInterface::GetEntities(const Scene& scene) {
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
priv::ECS<Entity>& priv::InternalScenePublicInterface::GetECS(const Scene& scene) {
    return scene.m_i->m_ECS;
}
void priv::InternalScenePublicInterface::CleanECS(Scene& scene, Entity inEntity) {
    for (auto& pipelines : scene.m_RenderGraphs) {
        for (auto& graph : pipelines) {
            graph.clean(inEntity);
        }
    }
}


void priv::InternalScenePublicInterface::UpdateMaterials(Scene& scene, const float dt) {
    for (unsigned int i = 0; i < RenderStage::_TOTAL; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            for (auto& materialNode : render_graph_ptr.m_MaterialNodes) {
                materialNode.material->m_UpdatedThisFrame = false;
            }
        }
    }
    for (unsigned int i = 0; i < RenderStage::_TOTAL; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            for (auto& materialNode : render_graph_ptr.m_MaterialNodes) {
                auto& material = *materialNode.material;
                if (!material.m_UpdatedThisFrame) {
                    material.update(dt);
                    material.m_UpdatedThisFrame = true;
                }
            }
        }
    }
}

void priv::InternalScenePublicInterface::RenderGeometryOpaque(Renderer& renderer, Scene& scene, Viewport& viewport, Camera& camera, bool useDefaultShaders) {
    for (unsigned int i = RenderStage::GeometryOpaque; i < RenderStage::GeometryOpaque_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            auto& render_graph = const_cast<RenderGraph&>(render_graph_ptr);
            render_graph.validate_model_instances_for_rendering(viewport, camera);
            render_graph.render(renderer, viewport, camera, useDefaultShaders, SortingMode::None);
        }
    }
}
void priv::InternalScenePublicInterface::RenderGeometryTransparent(Renderer& renderer, Scene& scene, Viewport& viewport, Camera& camera, bool useDefaultShaders) {
    for (unsigned int i = RenderStage::GeometryTransparent; i < RenderStage::GeometryTransparent_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            auto& render_graph = const_cast<RenderGraph&>(render_graph_ptr);
            render_graph.sort(camera, SortingMode::BackToFront);
            render_graph.validate_model_instances_for_rendering(viewport, camera);
            render_graph.render(renderer, viewport, camera, useDefaultShaders, SortingMode::None);
        }
    }
}
void priv::InternalScenePublicInterface::RenderGeometryTransparentTrianglesSorted(Renderer& renderer, Scene& scene, Viewport& viewport, Camera& camera, bool useDefaultShaders) {
    for (unsigned int i = RenderStage::GeometryTransparentTrianglesSorted; i < RenderStage::GeometryTransparentTrianglesSorted_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            auto& render_graph = const_cast<RenderGraph&>(render_graph_ptr);
            render_graph.sort(camera, SortingMode::FrontToBack);
            render_graph.validate_model_instances_for_rendering(viewport, camera);
            render_graph.render(renderer, viewport, camera, useDefaultShaders, SortingMode::FrontToBack);
        }
    }
}
void priv::InternalScenePublicInterface::RenderForwardOpaque(Renderer& renderer, Scene& scene, Viewport& viewport, Camera& camera, bool useDefaultShaders) {
    for (unsigned int i = RenderStage::ForwardOpaque; i < RenderStage::ForwardOpaque_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            auto& render_graph = const_cast<RenderGraph&>(render_graph_ptr);
            render_graph.validate_model_instances_for_rendering(viewport, camera);
            render_graph.render(renderer, viewport, camera, useDefaultShaders, SortingMode::None);
        }
    }
}
void priv::InternalScenePublicInterface::RenderForwardTransparent(Renderer& renderer, Scene& scene, Viewport& viewport, Camera& camera, bool useDefaultShaders) {
    for (unsigned int i = RenderStage::ForwardTransparent; i < RenderStage::ForwardTransparent_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            auto& render_graph = const_cast<RenderGraph&>(render_graph_ptr);
            render_graph.sort_bruteforce(camera, SortingMode::BackToFront);
            render_graph.validate_model_instances_for_rendering(viewport, camera);
            render_graph.render_bruteforce(renderer, viewport, camera, useDefaultShaders, SortingMode::None);
        }
    }
}
void priv::InternalScenePublicInterface::RenderForwardTransparentTrianglesSorted(Renderer& renderer, Scene& scene, Viewport& viewport, Camera& camera, bool useDefaultShaders) {
    for (unsigned int i = RenderStage::ForwardTransparentTrianglesSorted; i < RenderStage::ForwardTransparentTrianglesSorted_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            //TODO: sort_bruteforce and render_bruteforce doesn't work here... probably has to do with custom binds and unbinds and custom shader(s)
            auto& render_graph = const_cast<RenderGraph&>(render_graph_ptr);
            render_graph.sort(camera, SortingMode::FrontToBack);
            render_graph.validate_model_instances_for_rendering(viewport, camera);
            render_graph.render(renderer, viewport, camera, useDefaultShaders, SortingMode::FrontToBack);
        }
    }
}
void priv::InternalScenePublicInterface::RenderForwardParticles(Renderer& renderer, Scene& scene, Viewport& viewport, Camera& camera, bool useDefaultShaders) {
    for (unsigned int i = RenderStage::ForwardParticles; i < RenderStage::ForwardParticles_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            auto& render_graph = const_cast<RenderGraph&>(render_graph_ptr);
            render_graph.sort_cheap_bruteforce(camera, SortingMode::BackToFront);
            render_graph.validate_model_instances_for_rendering(viewport, camera);
            render_graph.render_bruteforce(renderer, viewport, camera, useDefaultShaders, SortingMode::None);
        }
    }
}
void priv::InternalScenePublicInterface::RenderDecals(Renderer& renderer, Scene& scene, Viewport& viewport, Camera& camera, bool useDefaultShaders) {
    for (unsigned int i = RenderStage::Decals; i < RenderStage::Decals_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            auto& render_graph = const_cast<RenderGraph&>(render_graph_ptr);
            render_graph.sort(camera, SortingMode::None);
            render_graph.validate_model_instances_for_rendering(viewport, camera);
            render_graph.render(renderer, viewport, camera, useDefaultShaders, SortingMode::None);
        }
    }
}
void priv::InternalScenePublicInterface::RenderParticles(Renderer& renderer, Scene& scene, Viewport& viewport, Camera& camera, ShaderProgram& program) {
    scene.m_i->m_ParticleSystem.render(viewport, camera, program, renderer);
}
void priv::InternalScenePublicInterface::AddModelInstanceToPipeline(Scene& scene, ModelInstance& modelInstance, RenderStage::Stage stage) {
    scene.m_i->_addModelInstanceToPipeline(scene, modelInstance, scene.m_RenderGraphs[stage], stage);
}
void priv::InternalScenePublicInterface::RemoveModelInstanceFromPipeline(Scene& scene, ModelInstance& modelInstance, RenderStage::Stage stage){
    scene.m_i->_removeModelInstanceFromPipeline(modelInstance, scene.m_RenderGraphs[stage]);
}

Scene::Scene(const string& name, const SceneOptions& options) : Resource(ResourceType::Scene, name) {
    m_RenderGraphs.resize(RenderStage::_TOTAL);
    m_ID      = priv::Core::m_Engine->m_ResourceManager.AddScene(*this);
    m_i       = NEW impl();
    m_i->_init(*this, name, options);
    setName(name);

    registerEvent(EventType::SceneChanged);
}
Scene::Scene(const string& name) : Scene(name, SceneOptions::DEFAULT_OPTIONS) {

}
Scene::~Scene() {
    m_i->_destruct();
    SAFE_DELETE(m_i);
    SAFE_DELETE(m_Skybox);
    SAFE_DELETE_VECTOR(m_Lights);
    SAFE_DELETE_VECTOR(m_Cameras);
    unregisterEvent(EventType::SceneChanged);
}
void Scene::addCamera(Camera& camera) {
    m_Cameras.push_back(&camera);
    if (!getActiveCamera()) {
        setActiveCamera(camera);
    }
}
Camera* Scene::addCamera(float left, float right, float bottom, float top, float Near, float Far) {
    Camera* camera = NEW Camera(left, right, bottom, top, Near, Far, this);
    Scene::addCamera(*camera);
    return camera;
}
Camera* Scene::addCamera(float angle, float aspectRatio, float Near, float Far) {
    Camera* camera = NEW Camera(angle, aspectRatio, Near, Far, this);
    Scene::addCamera(*camera);
    return camera;
}
void Scene::setGodRaysSun(Entity* sun) {
    m_Sun = sun;
}
Entity* Scene::getGodRaysSun() const {
    return m_Sun;
}
unsigned int Scene::numViewports() const {
    return static_cast<unsigned int>(m_Viewports.size());
}
unsigned int Scene::id() const {
    return m_ID;
}


ParticleEmitter* Scene::addParticleEmitter(ParticleEmissionProperties& properties, Scene& scene, float lifetime, Entity* parent) {
    return m_i->m_ParticleSystem.add_emitter(properties, scene, lifetime, (parent) ? *parent : Entity());
}


Viewport& Scene::addViewport(float x, float y, float width, float height, const Camera& camera) {
    const unsigned int id = numViewports();
    Viewport& viewport    = m_Viewports.emplace_back(*this, camera);
    viewport.m_ID         = id;
    viewport.setViewportDimensions(x, y, width, height);
    return viewport;
}
Entity Scene::createEntity() { 
    return m_i->m_ECS.createEntity(*this); 
}
void Scene::removeEntity(Entity entity) { 
    m_i->m_ECS.removeEntity(entity);
}
Viewport& Scene::getMainViewport() {
    return m_Viewports[0];
}
Camera* Scene::getActiveCamera() const {
    if (m_Viewports.size() == 0) {
        return nullptr;
    }
    return &(const_cast<Camera&>(m_Viewports[0].getCamera()));
}
void Scene::setActiveCamera(Camera& camera){
    if (m_Viewports.size() == 0) {
        const unsigned int id = numViewports();
        Viewport& viewport    = m_Viewports.emplace_back(*this, camera);
        viewport.m_ID         = id;
        return;
    }
    m_Viewports[0].setCamera(camera);
}
void Scene::centerSceneToObject(Entity centerEntity){
    return m_i->_centerToObject(*this, centerEntity);
}
void Scene::setOnUpdateFunctor(std::function<void(Scene*, const float)> functor) {
    m_OnUpdateFunctor = functor;
}
void Scene::update(const float dt){
    m_OnUpdateFunctor(this, dt);
    m_i->m_ECS.update(dt, *this);

    priv::InternalScenePublicInterface::UpdateMaterials(*this, dt);

    m_i->m_ParticleSystem.update(dt, *getActiveCamera());
}
void Scene::preUpdate(const float dt) {
    m_i->m_ECS.preUpdate(*this, dt);
}
void Scene::postUpdate(const float dt) {
    m_i->m_ECS.postUpdate(*this, dt);
}
void Scene::onResize(unsigned int width, unsigned int height) {

}
void Scene::render() {

}

const glm::vec4& Scene::getBackgroundColor() const {
    return m_Viewports[0].m_BackgroundColor;
}
void Scene::setBackgroundColor(float r, float g, float b, float a) {
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
void Scene::setGlobalIllumination(float global, float diffuse, float specular) {
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
