
#include <serenity/scene/Scene.h>
#include <serenity/scene/SceneOptions.h>
#include <serenity/system/Engine.h>
#include <serenity/math/Engine_Math.h>
#include <serenity/model/ModelInstance.h>
#include <serenity/scene/Skybox.h>
#include <serenity/resources/material/Material.h>
#include <serenity/renderer/RenderGraph.h>
#include <serenity/lights/Lights.h>
#include <serenity/scene/Camera.h>

#include <serenity/ecs/systems/SystemGameUpdate.h>
#include <serenity/ecs/systems/SystemSceneUpdate.h>
#include <serenity/ecs/systems/SystemSceneChanging.h>
#include <serenity/ecs/systems/SystemComponentTransform.h>
#include <serenity/ecs/systems/SystemComponentRigidBody.h>
#include <serenity/ecs/systems/SystemSyncTransformToRigid.h>
#include <serenity/ecs/systems/SystemStepPhysics.h>
#include <serenity/ecs/systems/SystemSyncRigidToTransform.h>
#include <serenity/ecs/systems/SystemTransformParentChild.h>
#include <serenity/ecs/systems/SystemCompoundChildTransforms.h>
#include <serenity/ecs/systems/SystemComponentTransformDebugDraw.h>
#include <serenity/ecs/systems/SystemComponentCamera.h>
#include <serenity/ecs/systems/SystemComponentLogic.h>
#include <serenity/ecs/systems/SystemComponentLogic1.h>
#include <serenity/ecs/systems/SystemComponentLogic2.h>
#include <serenity/ecs/systems/SystemComponentLogic3.h>
#include <serenity/ecs/systems/SystemComponentModel.h>
#include <serenity/ecs/systems/SystemComponentScript.h>

struct SceneImpl final {
    template<class FUNC> static void iterateMaterials(Scene& scene, FUNC&& func) {
        for (size_t i = 0; i < RenderStage::_TOTAL; ++i) {
            for (auto& graph : scene.m_RenderGraphs[i]) {
                graph.iterateMaterials(std::forward<FUNC&&>(func));
            }
        }
    }
};

#pragma region Scene

Scene::Scene(uint32_t id, std::string_view name, const SceneOptions& options) 
    : m_ParticleSystem{ options.maxAmountOfParticleEmitters, options.maxAmountOfParticles }
    , m_ID{ id }
{
    m_ECS.init(options);

    internal_register_lights();
    internal_register_components();
    internal_register_systems();

    setName(name);

    registerEvent(EventType::SceneChanged);
}
Scene::Scene(uint32_t id, std::string_view name)
    : Scene{ id, name, SceneOptions::DEFAULT_OPTIONS }
{}
Scene::~Scene() {
    SAFE_DELETE(m_Skybox);
    SAFE_DELETE_VECTOR(m_Cameras);
    m_ECS.destruct();
    unregisterEvent(EventType::SceneChanged);
}
void Scene::internal_register_lights() {
    m_LightsModule.registerLightType<SunLight>();
    m_LightsModule.registerLightType<PointLight>();
    m_LightsModule.registerLightType<DirectionalLight>();
    m_LightsModule.registerLightType<SpotLight>();
    m_LightsModule.registerLightType<RodLight>();
    m_LightsModule.registerLightType<ProjectionLight>();
}
void Scene::internal_register_components() {
    registerComponent<ComponentLogic>();
    registerComponent<ComponentTransform>();
    registerComponent<ComponentRigidBody>();
    registerComponent<ComponentCollisionShape>();
    registerComponent<ComponentLogic1>();
    registerComponent<ComponentModel>();
    registerComponent<ComponentLogic2>();
    registerComponent<ComponentCamera>();
    registerComponent<ComponentLogic3>();
    registerComponent<ComponentName>();
    registerComponent<ComponentScript>();

    Engine::priv::ComponentCollisionShapeDeferredLoading::get().registerEvent(EventType::ResourceLoaded);
}
void Scene::internal_register_systems() {
    registerSystemOrdered<SystemComponentRigidBody, std::tuple<>, ComponentRigidBody>(20'000);

    registerSystemOrdered<SystemGameUpdate, std::tuple<>>(30'000);
    registerSystemOrdered<SystemSceneUpdate, std::tuple<>>(40'000);
    registerSystemOrdered<SystemComponentScript, std::tuple<>, ComponentScript>(45'000);
    registerSystemOrdered<SystemComponentLogic, std::tuple<>, ComponentLogic>(50'000);

    registerSystemOrdered<SystemComponentTransform, std::tuple<>, ComponentTransform>(60'000);
    registerSystemOrdered<SystemTransformParentChild, std::tuple<>, ComponentTransform>(80'000);
    registerSystemOrdered<SystemSyncRigidToTransform, std::tuple<>, ComponentTransform, ComponentRigidBody>(81'000);
    registerSystemOrdered<SystemStepPhysics, std::tuple<>, ComponentRigidBody>(85'000); //TODO: figure this out

    //registerSystemOrdered<SystemStepPhysics, std::tuple<>, ComponentRigidBody>(100'000);
    registerSystemOrdered<SystemSyncTransformToRigid, std::tuple<>, ComponentTransform, ComponentRigidBody>(110'000);

    registerSystemOrdered<SystemCompoundChildTransforms, std::tuple<>, ComponentCollisionShape>(115'000);

    registerSystemOrdered<SystemComponentLogic1, std::tuple<>, ComponentLogic1>(120'000);
    registerSystemOrdered<SystemComponentModel, std::tuple<>, ComponentModel>(130'000);

    registerSystemOrdered<SystemComponentLogic2, std::tuple<>, ComponentLogic2>(140'000);
    registerSystemOrdered<SystemComponentCamera, std::tuple<>, ComponentCamera>(150'000);
    registerSystemOrdered<SystemComponentLogic3, std::tuple<>, ComponentLogic3>(160'000);

    registerSystemOrdered<SystemSceneChanging, std::tuple<>>(170'000);

    registerSystemOrdered<SystemComponentTransformDebugDraw, std::tuple<>, ComponentTransform, ComponentModel>(1'000'000);
}
size_t Scene::getNumLights() const noexcept {
    size_t count = 0;
    for (const auto& lightContainer : m_LightsModule) {
        count += lightContainer->size();
    }
    return count;
}
Engine::view_ptr<ParticleEmitter> Scene::addParticleEmitter(ParticleEmissionProperties& properties, Scene& scene, float lifetime, Entity parent) {
    return m_ParticleSystem.add_emitter(properties, scene, lifetime, parent);
}
Viewport& Scene::addViewport(float x, float y, float width, float height, Camera& camera) {
    Viewport& viewport  = m_Viewports.emplace_back(*this, camera);
    viewport.setViewportDimensions(x, y, width, height);
    return viewport;
}
Entity Scene::createEntity() { 
    return m_ECS.createEntity(*this); 
}
std::vector<Entity> Scene::createEntity(uint32_t amount) {
    return m_ECS.createEntity(*this, amount);
}
std::vector<Entity> Scene::createEntities(uint32_t amount) {
    return m_ECS.createEntity(*this, amount);
}
void Scene::removeEntity(Entity entity) { 
    m_ECS.removeEntity(entity);
}
Viewport& Scene::getMainViewport() {
    return m_Viewports[0];
}
Camera* Scene::getActiveCamera() const {
    return m_Viewports.size() > 0 ? &m_Viewports[0].getCamera() : nullptr;
}
void Scene::setActiveCamera(Camera& camera){
    if (m_Viewports.size() == 0) {
        auto& mainViewport = m_Viewports.emplace_back(*this, camera);
        mainViewport.setResizeFunc([](float x, float y, float width, float height, Viewport& viewport, void* userPointer) {
            viewport.setViewportDimensions(0.0f, 0.0f, width, height);
        });
        return;
    }
    m_Viewports[0].setCamera(camera);
}
void Scene::centerSceneToObject(Entity centerEntity) {
    auto centerTransform = centerEntity.getComponent<ComponentTransform>();
    ASSERT(centerTransform, __FUNCTION__ << "(): centerTransform was null!");
    auto centerPos       = centerTransform->getWorldPosition();
    auto centerPosFloat  = glm::vec3{ centerPos };
    for (const Entity e : Engine::priv::PublicScene::GetEntities(*this)) {
        if (e != centerEntity) {
            auto eTransform = e.getComponent<ComponentTransform>();
            if (eTransform && !eTransform->hasParent()) {
                eTransform->translate(-centerPos, false);
            }
        }
    }
    for (auto& particle : m_ParticleSystem.getParticles()) {
        if (particle.isActive()) {
            particle.translate(-centerPosFloat);
        }
    }
    for (auto& soundEffect : Engine::Sound::getAllSoundEffects()) {
        if (soundEffect.isActive() && soundEffect.getAttenuation() > 0.0f) {
            soundEffect.translate(-centerPosFloat);
        }
    }
    centerTransform->setPosition(decimal(0.0));
    Engine::priv::Core::m_Engine->m_SoundModule.updateCameraPosition(*this);
    ComponentTransform::recalculateAllParentChildMatrices(*this); //hmm is this needed?
}
void Scene::update(const float dt) {
    m_ECS.update(dt, *this);

    Engine::priv::PublicScene::UpdateMaterials(*this, dt);

    m_ParticleSystem.update(dt, *getActiveCamera());
}
void Scene::preUpdate(const float dt) {
    m_ECS.preUpdate(*this, dt);
}
void Scene::postUpdate(const float dt) {
    m_ECS.postUpdate(*this, dt);
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
    m_GI = glm::vec3{ global, diffuse, specular };
    Engine::Renderer::Settings::Lighting::setGIContribution(global, diffuse, specular);
}
void Scene::onEvent(const Event& e) {
    if (e.type == EventType::SceneChanged && e.eventSceneChanged.newScene == this) {
        Engine::Renderer::Settings::Lighting::setGIContribution(m_GI.x, m_GI.y, m_GI.z);
    }
}

#pragma endregion


#pragma region PublicScene
std::vector<Particle>& Engine::priv::PublicScene::GetParticles(const Scene& scene) {
    return scene.m_ParticleSystem.getParticles();
}
std::vector<Viewport>& Engine::priv::PublicScene::GetViewports(const Scene& scene) {
    return scene.m_Viewports;
}
std::vector<Camera*>& Engine::priv::PublicScene::GetCameras(const Scene& scene) {
    return scene.m_Cameras;
}
std::vector<Entity>& Engine::priv::PublicScene::GetEntities(Scene& scene) {
    return scene.m_ECS.m_EntityPool.m_Pool;
}
Engine::priv::ECS& Engine::priv::PublicScene::GetECS(Scene& scene) {
    return scene.m_ECS;
}
Engine::priv::ECS& Engine::priv::PublicScene::GetECS(Entity entity) {
    assert(!entity.null() && entity.scene() != nullptr);
    return Engine::priv::PublicScene::GetECS(*entity.scene());
}
void Engine::priv::PublicScene::CleanECS(Scene& scene, Entity inEntity) {
    for (auto& pipelines : scene.m_RenderGraphs) {
        for (auto& graph : pipelines) {
            graph.clean(inEntity);
        }
    }
}
void Engine::priv::PublicScene::UpdateMaterials(Scene& scene, const float dt) {
    SceneImpl::iterateMaterials(scene, [](Material& material) {
        material.m_UpdatedThisFrame = false;
        });
    SceneImpl::iterateMaterials(scene, [dt](Material& material) {
        if (!material.m_UpdatedThisFrame) {
            material.update(dt);
            material.m_UpdatedThisFrame = true;
        }
        });
}
void Engine::priv::PublicScene::RenderGeometryOpaque(RenderModule& renderer, Scene& scene, Viewport* viewport, Camera* camera, bool useDefaultShaders) {
    for (size_t i = RenderStage::GeometryOpaque; i < RenderStage::GeometryOpaque_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            render_graph_ptr.validate_model_instances_for_rendering(camera, viewport);
            render_graph_ptr.render(renderer, camera, useDefaultShaders, SortingMode::None);
        }
    }
}
void Engine::priv::PublicScene::RenderGeometryTransparent(RenderModule& renderer, Scene& scene, Viewport* viewport, Camera* camera, bool useDefaultShaders) {
    for (size_t i = RenderStage::GeometryTransparent; i < RenderStage::GeometryTransparent_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            render_graph_ptr.sort(camera, SortingMode::BackToFront);
            render_graph_ptr.validate_model_instances_for_rendering(camera, viewport);
            render_graph_ptr.render(renderer, camera, useDefaultShaders, SortingMode::None);
        }
    }
}
void Engine::priv::PublicScene::RenderGeometryTransparentTrianglesSorted(RenderModule& renderer, Scene& scene, Viewport* viewport, Camera* camera, bool useDefaultShaders) {
    for (size_t i = RenderStage::GeometryTransparentTrianglesSorted; i < RenderStage::GeometryTransparentTrianglesSorted_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            render_graph_ptr.sort(camera, SortingMode::FrontToBack);
            render_graph_ptr.validate_model_instances_for_rendering(camera, viewport);
            render_graph_ptr.render(renderer, camera, useDefaultShaders, SortingMode::FrontToBack);
        }
    }
}
void Engine::priv::PublicScene::RenderForwardOpaque(RenderModule& renderer, Scene& scene, Viewport* viewport, Camera* camera, bool useDefaultShaders) {
    for (size_t i = RenderStage::ForwardOpaque; i < RenderStage::ForwardOpaque_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            render_graph_ptr.validate_model_instances_for_rendering(camera, viewport);
            render_graph_ptr.render(renderer, camera, useDefaultShaders, SortingMode::None);
        }
    }
}
void Engine::priv::PublicScene::RenderForwardTransparent(RenderModule& renderer, Scene& scene, Viewport* viewport, Camera* camera, bool useDefaultShaders) {
    for (size_t i = RenderStage::ForwardTransparent; i < RenderStage::ForwardTransparent_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            render_graph_ptr.sort_bruteforce(camera, SortingMode::BackToFront);
            render_graph_ptr.validate_model_instances_for_rendering(camera, viewport);
            render_graph_ptr.render_bruteforce(renderer, camera, useDefaultShaders, SortingMode::None);
        }
    }
}
void Engine::priv::PublicScene::RenderForwardTransparentTrianglesSorted(RenderModule& renderer, Scene& scene, Viewport* viewport, Camera* camera, bool useDefaultShaders) {
    for (size_t i = RenderStage::ForwardTransparentTrianglesSorted; i < RenderStage::ForwardTransparentTrianglesSorted_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            //TODO: sort_bruteforce and render_bruteforce doesn't work here... probably has to do with custom binds and unbinds and custom shader(s)
            render_graph_ptr.sort(camera, SortingMode::FrontToBack);
            render_graph_ptr.validate_model_instances_for_rendering(camera, viewport);
            render_graph_ptr.render(renderer, camera, useDefaultShaders, SortingMode::FrontToBack);
        }
    }
}
void Engine::priv::PublicScene::RenderForwardParticles(RenderModule& renderer, Scene& scene, Viewport* viewport, Camera* camera, bool useDefaultShaders) {
    for (size_t i = RenderStage::ForwardParticles; i < RenderStage::ForwardParticles_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            auto& render_graph = const_cast<RenderGraph&>(render_graph_ptr);
            render_graph.sort_cheap_bruteforce(camera, SortingMode::BackToFront);
            render_graph.validate_model_instances_for_rendering(camera, viewport);
            render_graph.render_bruteforce(renderer, camera, useDefaultShaders, SortingMode::None);
        }
    }
}
void Engine::priv::PublicScene::RenderDecals(RenderModule& renderer, Scene& scene, Viewport* viewport, Camera* camera, bool useDefaultShaders) {
    for (size_t i = RenderStage::Decals; i < RenderStage::Decals_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            render_graph_ptr.sort(camera, SortingMode::None);
            render_graph_ptr.validate_model_instances_for_rendering(camera, viewport);
            render_graph_ptr.render(renderer, camera, useDefaultShaders, SortingMode::None);
        }
    }
}
void Engine::priv::PublicScene::RenderParticles(RenderModule& renderer, Scene& scene, Viewport& viewport, Camera& camera, Handle program) {
    scene.m_ParticleSystem.render(viewport, camera, program, renderer);
}
void Engine::priv::PublicScene::RenderGeometryOpaqueShadowMap(RenderModule& renderer, Scene& scene, Viewport* viewport, const glm::mat4& viewProjMatrix) {
    for (size_t i = RenderStage::GeometryOpaque; i < RenderStage::GeometryOpaque_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            render_graph_ptr.validate_model_instances_for_rendering(viewProjMatrix, viewport);
            render_graph_ptr.render_shadow_map(renderer, nullptr);
        }
    }
}
void Engine::priv::PublicScene::RenderGeometryTransparentShadowMap(RenderModule& renderer, Scene& scene, Viewport* viewport, const glm::mat4& viewProjMatrix) {
    for (size_t i = RenderStage::GeometryTransparent; i < RenderStage::GeometryTransparent_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            render_graph_ptr.sort(nullptr, SortingMode::BackToFront);
            render_graph_ptr.validate_model_instances_for_rendering(viewProjMatrix, viewport);
            render_graph_ptr.render_shadow_map(renderer, nullptr);
        }
    }
}
void Engine::priv::PublicScene::RenderGeometryTransparentTrianglesSortedShadowMap(RenderModule& renderer, Scene& scene, Viewport* viewport, const glm::mat4& viewProjMatrix) {
    for (size_t i = RenderStage::GeometryTransparentTrianglesSorted; i < RenderStage::GeometryTransparentTrianglesSorted_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            render_graph_ptr.sort(nullptr, SortingMode::FrontToBack);
            render_graph_ptr.validate_model_instances_for_rendering(viewProjMatrix, viewport);
            render_graph_ptr.render_shadow_map(renderer, nullptr);
        }
    }
}
void Engine::priv::PublicScene::RenderForwardOpaqueShadowMap(RenderModule& renderer, Scene& scene, Viewport* viewport, const glm::mat4& viewProjMatrix) {
    for (size_t i = RenderStage::ForwardOpaque; i < RenderStage::ForwardOpaque_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            render_graph_ptr.validate_model_instances_for_rendering(viewProjMatrix, viewport);
            render_graph_ptr.render_shadow_map(renderer, nullptr);
        }
    }
}
void Engine::priv::PublicScene::RenderForwardTransparentShadowMap(RenderModule& renderer, Scene& scene, Viewport* viewport, const glm::mat4& viewProjMatrix) {
    for (size_t i = RenderStage::ForwardTransparent; i < RenderStage::ForwardTransparent_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            render_graph_ptr.sort_bruteforce(nullptr, SortingMode::BackToFront);
            render_graph_ptr.validate_model_instances_for_rendering(viewProjMatrix, viewport);
            render_graph_ptr.render_bruteforce_shadow_map(renderer, nullptr);
        }
    }
}
void Engine::priv::PublicScene::RenderForwardTransparentTrianglesSortedShadowMap(RenderModule& renderer, Scene& scene, Viewport* viewport, const glm::mat4& viewProjMatrix) {
    for (size_t i = RenderStage::ForwardTransparentTrianglesSorted; i < RenderStage::ForwardTransparentTrianglesSorted_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            //TODO: sort_bruteforce and render_bruteforce doesn't work here... probably has to do with custom binds and unbinds and custom shader(s)
            render_graph_ptr.sort(nullptr, SortingMode::FrontToBack);
            render_graph_ptr.validate_model_instances_for_rendering(viewProjMatrix, viewport);
            render_graph_ptr.render_shadow_map(renderer, nullptr);
        }
    }
}


void Engine::priv::PublicScene::AddModelInstanceToPipeline(Scene& scene, ModelInstance& modelInstance, RenderStage stage) {
    auto& renderGraphs = scene.m_RenderGraphs[stage];
    Engine::priv::RenderGraph* renderGraph = nullptr;
    for (auto& graph : renderGraphs) {
        if (graph.m_ShaderProgram == modelInstance.getShaderProgram()) {
            renderGraph = &graph;
            break;
        }
    }
    if (!renderGraph) {
        renderGraph = &scene.m_RenderGraphs[(size_t)stage].emplace_back(modelInstance.getShaderProgram());
    }
    renderGraph->internal_addModelInstanceToPipeline(modelInstance);
}
void Engine::priv::PublicScene::RemoveModelInstanceFromPipeline(Scene& scene, ModelInstance& modelInstance, RenderStage stage) {
    auto& renderGraphs = scene.m_RenderGraphs[stage];
    Engine::priv::RenderGraph* renderGraph = nullptr;
    for (auto& graph : renderGraphs) {
        if (graph.m_ShaderProgram == modelInstance.getShaderProgram()) {
            renderGraph = &graph;
            break;
        }
    }
    if (renderGraph) {
        renderGraph->internal_removeModelInstanceFromPipeline(modelInstance);
    }
}
void Engine::priv::PublicScene::SkipRenderThisFrame(Scene& scene) {
    scene.m_SkipRenderThisFrame = true;
}
bool Engine::priv::PublicScene::IsSkipRenderThisFrame(Scene& scene) {
    return scene.m_SkipRenderThisFrame;
}
bool Engine::priv::PublicScene::HasItemsToRender(Scene& scene) {
    return scene.m_RenderGraphs.hasItemsToRender();
}
#pragma endregion