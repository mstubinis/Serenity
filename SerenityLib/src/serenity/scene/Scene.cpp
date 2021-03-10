
#include <serenity/scene/Scene.h>
#include <serenity/scene/SceneOptions.h>
#include <serenity/system/Engine.h>
#include <serenity/math/Engine_Math.h>
#include <serenity/model/ModelInstance.h>
#include <serenity/scene/Camera.h>
#include <serenity/scene/Skybox.h>
#include <serenity/resources/material/Material.h>
#include <serenity/renderer/particles/ParticleSystem.h>
#include <serenity/renderer/RenderGraph.h>
#include <serenity/lights/Lights.h>

#include <serenity/ecs/systems/SystemComponentBody.h>
#include <serenity/ecs/systems/SystemComponentCamera.h>
#include <serenity/ecs/systems/SystemComponentLogic.h>
#include <serenity/ecs/systems/SystemComponentLogic1.h>
#include <serenity/ecs/systems/SystemComponentLogic2.h>
#include <serenity/ecs/systems/SystemComponentLogic3.h>
#include <serenity/ecs/systems/SystemComponentModel.h>
#include <serenity/ecs/systems/SystemComponentBodyDebugDraw.h>

class Scene::impl final {
    public:
        Engine::priv::ParticleSystem   m_ParticleSystem;
        Engine::priv::ECS              m_ECS;

        impl() = delete;

        impl(uint32_t maxEmitters, uint32_t maxParticles)
            : m_ParticleSystem{ maxEmitters, maxParticles }
        {}

        void _init(Scene& super, const SceneOptions& options) {
            m_ECS.init(options);

            m_ECS.registerComponent<ComponentLogic>();
            m_ECS.registerComponent<ComponentBody>();
            m_ECS.registerComponent<ComponentLogic1>();
            m_ECS.registerComponent<ComponentModel>();
            m_ECS.registerComponent<ComponentLogic2>();
            m_ECS.registerComponent<ComponentCamera>();
            m_ECS.registerComponent<ComponentLogic3>();
            m_ECS.registerComponent<ComponentName>();

            auto* logicSystem     = m_ECS.registerSystem<SystemComponentLogic, ComponentLogic>();
            auto* bodySystem      = m_ECS.registerSystem<SystemComponentBody, ComponentBody>();
            auto* logic1System    = m_ECS.registerSystem<SystemComponentLogic1, ComponentLogic1>();
            auto* modelSystem     = m_ECS.registerSystem<SystemComponentModel, ComponentModel>();
            auto* logic2System    = m_ECS.registerSystem<SystemComponentLogic2, ComponentLogic2>();
            auto* debugDrawSystem = m_ECS.registerSystem <SystemComponentBodyDebugDraw, ComponentBody, ComponentModel>();
            auto* cameraSystem    = m_ECS.registerSystem<SystemComponentCamera, ComponentCamera>();
            auto* logic3System    = m_ECS.registerSystem<SystemComponentLogic3, ComponentLogic3>();
        }
        void _centerToObject(Scene& super, Entity centerEntity) {
            auto centerBody     = centerEntity.getComponent<ComponentBody>();
            auto centerPos      = centerBody->getPosition();
            auto centerPosFloat = glm::vec3(centerPos);
            for (const auto e : Engine::priv::PublicScene::GetEntities(super)) {
                if (e != centerEntity) {
                    auto eBody = e.getComponent<ComponentBody>();
                    if (eBody) {
                        if (!eBody->hasParent()) {
                            eBody->setPosition(eBody->getPosition() - centerPos);
                        }
                    }
                }
            }
            for (auto& particle : m_ParticleSystem.getParticles()) {
                if (particle.isActive()) {
                    particle.setPosition(particle.position() - centerPosFloat);
                }
            }
            for (auto& soundEffect : Engine::Sound::getAllSoundEffects()) {
                if (soundEffect.isActive() && soundEffect.getAttenuation() > 0.0f) {
                    soundEffect.setPosition(soundEffect.getPosition() - centerPosFloat);
                }
            }
            centerBody->setPosition((decimal)0.0, (decimal)0.0, (decimal)0.0);
            Engine::priv::Core::m_Engine->m_SoundModule.updateCameraPosition(super);
            ComponentBody::recalculateAllParentChildMatrices(super);
        }
        void _addModelInstanceToPipeline(Scene& scene, ModelInstance& inModelInstance, std::vector<Engine::priv::RenderGraph>& renderGraphs, RenderStage stage) {
            Engine::priv::RenderGraph* renderGraph = nullptr;
            for (auto& graph : renderGraphs) {
                if (graph.m_ShaderProgram == inModelInstance.shaderProgram()) {
                    renderGraph = &graph;
                    break;
                }
            }
            if (!renderGraph) {
                renderGraph = &scene.m_RenderGraphs[(size_t)stage].emplace_back(inModelInstance.shaderProgram());
            }
            renderGraph->internal_addModelInstanceToPipeline(inModelInstance);
        }
        void _removeModelInstanceFromPipeline(ModelInstance& inModelInstance, std::vector<Engine::priv::RenderGraph>& renderGraphs) {
            Engine::priv::RenderGraph* renderGraph = nullptr;
            for (auto& graph : renderGraphs) {
                if (graph.m_ShaderProgram == inModelInstance.shaderProgram()) {
                    renderGraph = &graph;
                    break;
                }
            }
            if (renderGraph) {
                renderGraph->internal_removeModelInstanceFromPipeline(inModelInstance);
            }
        }
};
std::vector<Particle>& Engine::priv::PublicScene::GetParticles(const Scene& scene) {
    return scene.m_i->m_ParticleSystem.getParticles();
}
std::vector<Viewport>& Engine::priv::PublicScene::GetViewports(const Scene& scene) {
    return scene.m_Viewports;
}
std::vector<Camera*>& Engine::priv::PublicScene::GetCameras(const Scene& scene) {
    return scene.m_Cameras;
}
std::vector<Entity>& Engine::priv::PublicScene::GetEntities(const Scene& scene) {
    return scene.m_i->m_ECS.m_EntityPool.m_Pool;
}
Engine::priv::ECS& Engine::priv::PublicScene::GetECS(Scene& scene) {
    return scene.m_i->m_ECS;
}
void Engine::priv::PublicScene::CleanECS(Scene& scene, Entity inEntity) {
    for (auto& pipelines : scene.m_RenderGraphs) {
        for (auto& graph : pipelines) {
            graph.clean(inEntity);
        }
    }
}
void Engine::priv::PublicScene::UpdateMaterials(Scene& scene, const float dt) {
    for (size_t i = 0; i < (size_t)RenderStage::_TOTAL; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            for (auto& materialNode : render_graph_ptr.m_MaterialNodes) {
                auto material = materialNode.material.get<Material>();
                material->m_UpdatedThisFrame = false;
            }
        }
    }
    for (size_t i = 0; i < (size_t)RenderStage::_TOTAL; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            for (auto& materialNode : render_graph_ptr.m_MaterialNodes) {
                auto material = materialNode.material.get<Material>();
                if (!material->m_UpdatedThisFrame) {
                    material->update(dt);
                    material->m_UpdatedThisFrame = true;
                }
            }
        }
    }
}
void Engine::priv::PublicScene::RenderGeometryOpaque(RenderModule& renderer, Scene& scene, Viewport& viewport, Camera& camera, bool useDefaultShaders) {
    for (size_t i = (size_t)RenderStage::GeometryOpaque; i < (size_t)RenderStage::GeometryOpaque_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            render_graph_ptr.validate_model_instances_for_rendering(viewport, camera);
            render_graph_ptr.render(renderer, viewport, camera, useDefaultShaders, SortingMode::None);
        }
    }
}
void Engine::priv::PublicScene::RenderGeometryTransparent(RenderModule& renderer, Scene& scene, Viewport& viewport, Camera& camera, bool useDefaultShaders) {
    for (size_t i = (size_t)RenderStage::GeometryTransparent; i < (size_t)RenderStage::GeometryTransparent_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            render_graph_ptr.sort(camera, SortingMode::BackToFront);
            render_graph_ptr.validate_model_instances_for_rendering(viewport, camera);
            render_graph_ptr.render(renderer, viewport, camera, useDefaultShaders, SortingMode::None);
        }
    }
}
void Engine::priv::PublicScene::RenderGeometryTransparentTrianglesSorted(RenderModule& renderer, Scene& scene, Viewport& viewport, Camera& camera, bool useDefaultShaders) {
    for (size_t i = (size_t)RenderStage::GeometryTransparentTrianglesSorted; i < (size_t)RenderStage::GeometryTransparentTrianglesSorted_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            render_graph_ptr.sort(camera, SortingMode::FrontToBack);
            render_graph_ptr.validate_model_instances_for_rendering(viewport, camera);
            render_graph_ptr.render(renderer, viewport, camera, useDefaultShaders, SortingMode::FrontToBack);
        }
    }
}
void Engine::priv::PublicScene::RenderForwardOpaque(RenderModule& renderer, Scene& scene, Viewport& viewport, Camera& camera, bool useDefaultShaders) {
    for (size_t i = (size_t)RenderStage::ForwardOpaque; i < (size_t)RenderStage::ForwardOpaque_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            render_graph_ptr.validate_model_instances_for_rendering(viewport, camera);
            render_graph_ptr.render(renderer, viewport, camera, useDefaultShaders, SortingMode::None);
        }
    }
}
void Engine::priv::PublicScene::RenderForwardTransparent(RenderModule& renderer, Scene& scene, Viewport& viewport, Camera& camera, bool useDefaultShaders) {
    for (size_t i = (size_t)RenderStage::ForwardTransparent; i < (size_t)RenderStage::ForwardTransparent_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            render_graph_ptr.sort_bruteforce(camera, SortingMode::BackToFront);
            render_graph_ptr.validate_model_instances_for_rendering(viewport, camera);
            render_graph_ptr.render_bruteforce(renderer, viewport, camera, useDefaultShaders, SortingMode::None);
        }
    }
}
void Engine::priv::PublicScene::RenderForwardTransparentTrianglesSorted(RenderModule& renderer, Scene& scene, Viewport& viewport, Camera& camera, bool useDefaultShaders) {
    for (size_t i = (size_t)RenderStage::ForwardTransparentTrianglesSorted; i < (size_t)RenderStage::ForwardTransparentTrianglesSorted_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            //TODO: sort_bruteforce and render_bruteforce doesn't work here... probably has to do with custom binds and unbinds and custom shader(s)
            render_graph_ptr.sort(camera, SortingMode::FrontToBack);
            render_graph_ptr.validate_model_instances_for_rendering(viewport, camera);
            render_graph_ptr.render(renderer, viewport, camera, useDefaultShaders, SortingMode::FrontToBack);
        }
    }
}
void Engine::priv::PublicScene::RenderForwardParticles(RenderModule& renderer, Scene& scene, Viewport& viewport, Camera& camera, bool useDefaultShaders) {
    for (size_t i = (size_t)RenderStage::ForwardParticles; i < (size_t)RenderStage::ForwardParticles_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            auto& render_graph = const_cast<RenderGraph&>(render_graph_ptr);
            render_graph.sort_cheap_bruteforce(camera, SortingMode::BackToFront);
            render_graph.validate_model_instances_for_rendering(viewport, camera);
            render_graph.render_bruteforce(renderer, viewport, camera, useDefaultShaders, SortingMode::None);
        }
    }
}
void Engine::priv::PublicScene::RenderDecals(RenderModule& renderer, Scene& scene, Viewport& viewport, Camera& camera, bool useDefaultShaders) {
    for (size_t i = (size_t)RenderStage::Decals; i < (size_t)RenderStage::Decals_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            render_graph_ptr.sort(camera, SortingMode::None);
            render_graph_ptr.validate_model_instances_for_rendering(viewport, camera);
            render_graph_ptr.render(renderer, viewport, camera, useDefaultShaders, SortingMode::None);
        }
    }
}
void Engine::priv::PublicScene::RenderParticles(RenderModule& renderer, Scene& scene, Viewport& viewport, Camera& camera, Handle program) {
    scene.m_i->m_ParticleSystem.render(viewport, camera, program, renderer);
}
void Engine::priv::PublicScene::AddModelInstanceToPipeline(Scene& scene, ModelInstance& modelInstance, RenderStage stage) {
    scene.m_i->_addModelInstanceToPipeline(scene, modelInstance, scene.m_RenderGraphs[(uint32_t)stage], stage);
}
void Engine::priv::PublicScene::RemoveModelInstanceFromPipeline(Scene& scene, ModelInstance& modelInstance, RenderStage stage){
    scene.m_i->_removeModelInstanceFromPipeline(modelInstance, scene.m_RenderGraphs[(uint32_t)stage]);
}
void Engine::priv::PublicScene::SkipRenderThisFrame(Scene& scene, bool isSkip) {
    scene.m_SkipRenderThisFrame = isSkip;
}
bool Engine::priv::PublicScene::IsSkipRenderThisFrame(Scene& scene) {
    return scene.m_SkipRenderThisFrame;
}
bool Engine::priv::PublicScene::HasItemsToRender(Scene& scene) {
    return scene.m_RenderGraphs.hasItemsToRender();
}

Scene::Scene(std::string_view name, const SceneOptions& options) {
    m_LightsModule.registerLightType<SunLight>();
    m_LightsModule.registerLightType<PointLight>();
    m_LightsModule.registerLightType<DirectionalLight>();
    m_LightsModule.registerLightType<SpotLight>();
    m_LightsModule.registerLightType<RodLight>();
    m_LightsModule.registerLightType<ProjectionLight>();

    m_ID      = Engine::priv::Core::m_Engine->m_ResourceManager.AddScene(*this);
    m_i       = std::make_unique<impl>(options.maxAmountOfParticleEmitters, options.maxAmountOfParticles);
    m_i->_init(*this, options);
    setName(name);

    registerEvent(EventType::SceneChanged);
}
Scene::Scene(std::string_view name)
    : Scene{ name, SceneOptions::DEFAULT_OPTIONS }
{}
Scene::~Scene() {
    SAFE_DELETE(m_Skybox);
    SAFE_DELETE_VECTOR(m_Cameras);
    unregisterEvent(EventType::SceneChanged);
}
size_t Scene::getNumLights() const noexcept {
    size_t count = 0;
    for (const auto& itr : m_LightsModule) {
        count += itr->size();
    }
    return count;
}
void Scene::clearAllEntities() noexcept {
    m_i->m_ECS.clearAllEntities();
}
void Scene::addCamera(Camera& camera) {
    m_Cameras.emplace_back(&camera);
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
ParticleEmitter* Scene::addParticleEmitter(ParticleEmissionProperties& properties, Scene& scene, float lifetime, Entity* parent) {
    return m_i->m_ParticleSystem.add_emitter(properties, scene, lifetime, (parent) ? *parent : Entity());
}
Viewport& Scene::addViewport(float x, float y, float width, float height, Camera& camera) {
    uint32_t id         = numViewports();
    Viewport& viewport  = m_Viewports.emplace_back(*this, camera);
    viewport.m_ID       = id;
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
    if (m_Viewports.size() == 0){
        return nullptr;
    }
    return &m_Viewports[0].getCamera();
}
void Scene::setActiveCamera(Camera& camera){
    if (m_Viewports.size() == 0) {
        uint32_t id        = numViewports();
        Viewport& viewport = m_Viewports.emplace_back(*this, camera);
        viewport.m_ID      = id;
        return;
    }
    m_Viewports[0].setCamera(camera);
}
void Scene::centerSceneToObject(Entity centerEntity){
    return m_i->_centerToObject(*this, centerEntity);
}
void Scene::update(const float dt){
    m_OnUpdateFunctor(this, dt);
    m_i->m_ECS.update(dt, *this);

    Engine::priv::PublicScene::UpdateMaterials(*this, dt);

    m_i->m_ParticleSystem.update(dt, *getActiveCamera());
}
void Scene::preUpdate(const float dt) {
    m_i->m_ECS.preUpdate(*this, dt);
}
void Scene::postUpdate(const float dt) {
    m_i->m_ECS.postUpdate(*this, dt);
}
const glm::vec4& Scene::getBackgroundColor() const {
    return m_Viewports[0].m_BackgroundColor;
}
void Scene::setBackgroundColor(float r, float g, float b, float a) {
    Engine::Math::setColor(m_Viewports[0].m_BackgroundColor, r, g, b, a);
}
void Scene::setBackgroundColor(const glm::vec4& backgroundColor) {
    setBackgroundColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
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
