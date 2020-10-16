#include <core/engine/utils/PrecompiledHeader.h>
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

class Scene::impl final {
    public:
        Engine::priv::ParticleSystem   m_ParticleSystem;
        priv::ECS              m_ECS;

        impl() = delete;

        impl(unsigned int maxEmitters, unsigned int maxParticles)
            : m_ParticleSystem{ maxEmitters, maxParticles }
        {}

        void _init(Scene& super, const SceneOptions& options) {
            m_ECS.init(options);
            m_ECS.assignSystem<ComponentLogic> (ComponentLogic_System_CI()/*, 20000*/);
            m_ECS.assignSystem<ComponentBody, Engine::priv::ComponentBody_System>(ComponentBody_System_CI()/*, 10000*/);
            m_ECS.assignSystem<ComponentLogic1>(ComponentLogic1_System_CI()/*, 30000*/);
            m_ECS.assignSystem<ComponentModel>(ComponentModel_System_CI()/*, 60000*/);
            m_ECS.assignSystem<ComponentLogic2>(ComponentLogic2_System_CI()/*, 40000*/);
            m_ECS.assignSystem<ComponentCamera>(ComponentCamera_System_CI()/*, 70000*/);
            m_ECS.assignSystem<ComponentLogic3>(ComponentLogic3_System_CI()/*, 50000*/);
            m_ECS.assignSystem<ComponentName>  (ComponentName_System_CI()/*, 80000*/);
        }
        void _centerToObject(Scene& super, Entity centerEntity) {
            auto centerBody     = centerEntity.getComponent<ComponentBody>();
            auto centerPos      = centerBody->getPosition();
            auto centerPosFloat = glm::vec3(centerPos);
            for (const auto e : priv::InternalScenePublicInterface::GetEntities(super)) {
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
        void _addModelInstanceToPipeline(Scene& scene, ModelInstance& inModelInstance, std::vector<priv::RenderGraph>& renderGraphs, RenderStage stage, ComponentModel& componentModel) {
            priv::RenderGraph* renderGraph = nullptr;
            for (auto& graph : renderGraphs) {
                if (graph.m_ShaderProgram == inModelInstance.shaderProgram()) {
                    renderGraph = &graph;
                    break;
                }
            }
            if (!renderGraph) {
                renderGraph = &scene.m_RenderGraphs[(size_t)stage].emplace_back(inModelInstance.shaderProgram());
            }
            renderGraph->addModelInstanceToPipeline(inModelInstance, componentModel);
        }
        void _removeModelInstanceFromPipeline(ModelInstance& inModelInstance, std::vector<priv::RenderGraph>& renderGraphs) {
            priv::RenderGraph* renderGraph = nullptr;
            for (auto& graph : renderGraphs) {
                if (graph.m_ShaderProgram == inModelInstance.shaderProgram()) {
                    renderGraph = &graph;
                    break;
                }
            }
            if (renderGraph) {
                renderGraph->removeModelInstanceFromPipeline(inModelInstance);
            }
        }
};
std::vector<Particle>& priv::InternalScenePublicInterface::GetParticles(const Scene& scene) {
    return scene.m_i->m_ParticleSystem.getParticles();
}
std::vector<Viewport>& priv::InternalScenePublicInterface::GetViewports(const Scene& scene) {
    return scene.m_Viewports;
}
std::vector<Camera*>& priv::InternalScenePublicInterface::GetCameras(const Scene& scene) {
    return scene.m_Cameras;
}
std::vector<Entity>& priv::InternalScenePublicInterface::GetEntities(const Scene& scene) {
    return scene.m_i->m_ECS.m_EntityPool.m_Pool;
}
std::vector<SunLight*>& priv::InternalScenePublicInterface::GetSunLights(const Scene& scene) {
    return scene.m_SunLights;
}
std::vector<DirectionalLight*>& priv::InternalScenePublicInterface::GetDirectionalLights(const Scene& scene) {
    return scene.m_DirectionalLights;
}
std::vector<PointLight*>& priv::InternalScenePublicInterface::GetPointLights(const Scene& scene) {
    return scene.m_PointLights;
}
std::vector<SpotLight*>& priv::InternalScenePublicInterface::GetSpotLights(const Scene& scene) {
    return scene.m_SpotLights;
}
std::vector<RodLight*>& priv::InternalScenePublicInterface::GetRodLights(const Scene& scene) {
    return scene.m_RodLights;
}
std::vector<ProjectionLight*>& priv::InternalScenePublicInterface::GetProjectionLights(const Scene& scene) {
    return scene.m_ProjectionLights;
}
Engine::priv::ECS& priv::InternalScenePublicInterface::GetECS(Scene& scene) {
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
    for (size_t i = 0; i < (size_t)RenderStage::_TOTAL; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            for (auto& materialNode : render_graph_ptr.m_MaterialNodes) {
                auto* material = materialNode.material.get<Material>();
                material->m_UpdatedThisFrame = false;
            }
        }
    }
    for (size_t i = 0; i < (size_t)RenderStage::_TOTAL; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            for (auto& materialNode : render_graph_ptr.m_MaterialNodes) {
                auto* material = materialNode.material.get<Material>();
                if (!material->m_UpdatedThisFrame) {
                    material->update(dt);
                    material->m_UpdatedThisFrame = true;
                }
            }
        }
    }
}
void priv::InternalScenePublicInterface::RenderGeometryOpaque(RenderModule& renderer, Scene& scene, Viewport& viewport, Camera& camera, bool useDefaultShaders) {
    for (size_t i = (size_t)RenderStage::GeometryOpaque; i < (size_t)RenderStage::GeometryOpaque_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            render_graph_ptr.validate_model_instances_for_rendering(viewport, camera);
            render_graph_ptr.render(renderer, viewport, camera, useDefaultShaders, SortingMode::None);
        }
    }
}
void priv::InternalScenePublicInterface::RenderGeometryTransparent(RenderModule& renderer, Scene& scene, Viewport& viewport, Camera& camera, bool useDefaultShaders) {
    for (size_t i = (size_t)RenderStage::GeometryTransparent; i < (size_t)RenderStage::GeometryTransparent_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            render_graph_ptr.sort(camera, SortingMode::BackToFront);
            render_graph_ptr.validate_model_instances_for_rendering(viewport, camera);
            render_graph_ptr.render(renderer, viewport, camera, useDefaultShaders, SortingMode::None);
        }
    }
}
void priv::InternalScenePublicInterface::RenderGeometryTransparentTrianglesSorted(RenderModule& renderer, Scene& scene, Viewport& viewport, Camera& camera, bool useDefaultShaders) {
    for (size_t i = (size_t)RenderStage::GeometryTransparentTrianglesSorted; i < (size_t)RenderStage::GeometryTransparentTrianglesSorted_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            render_graph_ptr.sort(camera, SortingMode::FrontToBack);
            render_graph_ptr.validate_model_instances_for_rendering(viewport, camera);
            render_graph_ptr.render(renderer, viewport, camera, useDefaultShaders, SortingMode::FrontToBack);
        }
    }
}
void priv::InternalScenePublicInterface::RenderForwardOpaque(RenderModule& renderer, Scene& scene, Viewport& viewport, Camera& camera, bool useDefaultShaders) {
    for (size_t i = (size_t)RenderStage::ForwardOpaque; i < (size_t)RenderStage::ForwardOpaque_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            render_graph_ptr.validate_model_instances_for_rendering(viewport, camera);
            render_graph_ptr.render(renderer, viewport, camera, useDefaultShaders, SortingMode::None);
        }
    }
}
void priv::InternalScenePublicInterface::RenderForwardTransparent(RenderModule& renderer, Scene& scene, Viewport& viewport, Camera& camera, bool useDefaultShaders) {
    for (size_t i = (size_t)RenderStage::ForwardTransparent; i < (size_t)RenderStage::ForwardTransparent_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            render_graph_ptr.sort_bruteforce(camera, SortingMode::BackToFront);
            render_graph_ptr.validate_model_instances_for_rendering(viewport, camera);
            render_graph_ptr.render_bruteforce(renderer, viewport, camera, useDefaultShaders, SortingMode::None);
        }
    }
}
void priv::InternalScenePublicInterface::RenderForwardTransparentTrianglesSorted(RenderModule& renderer, Scene& scene, Viewport& viewport, Camera& camera, bool useDefaultShaders) {
    for (size_t i = (size_t)RenderStage::ForwardTransparentTrianglesSorted; i < (size_t)RenderStage::ForwardTransparentTrianglesSorted_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            //TODO: sort_bruteforce and render_bruteforce doesn't work here... probably has to do with custom binds and unbinds and custom shader(s)
            render_graph_ptr.sort(camera, SortingMode::FrontToBack);
            render_graph_ptr.validate_model_instances_for_rendering(viewport, camera);
            render_graph_ptr.render(renderer, viewport, camera, useDefaultShaders, SortingMode::FrontToBack);
        }
    }
}
void priv::InternalScenePublicInterface::RenderForwardParticles(RenderModule& renderer, Scene& scene, Viewport& viewport, Camera& camera, bool useDefaultShaders) {
    for (size_t i = (size_t)RenderStage::ForwardParticles; i < (size_t)RenderStage::ForwardParticles_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            auto& render_graph = const_cast<RenderGraph&>(render_graph_ptr);
            render_graph.sort_cheap_bruteforce(camera, SortingMode::BackToFront);
            render_graph.validate_model_instances_for_rendering(viewport, camera);
            render_graph.render_bruteforce(renderer, viewport, camera, useDefaultShaders, SortingMode::None);
        }
    }
}
void priv::InternalScenePublicInterface::RenderDecals(RenderModule& renderer, Scene& scene, Viewport& viewport, Camera& camera, bool useDefaultShaders) {
    for (size_t i = (size_t)RenderStage::Decals; i < (size_t)RenderStage::Decals_4; ++i) {
        for (auto& render_graph_ptr : scene.m_RenderGraphs[i]) {
            render_graph_ptr.sort(camera, SortingMode::None);
            render_graph_ptr.validate_model_instances_for_rendering(viewport, camera);
            render_graph_ptr.render(renderer, viewport, camera, useDefaultShaders, SortingMode::None);
        }
    }
}
void priv::InternalScenePublicInterface::RenderParticles(RenderModule& renderer, Scene& scene, Viewport& viewport, Camera& camera, Handle program) {
    scene.m_i->m_ParticleSystem.render(viewport, camera, program, renderer);
}
void priv::InternalScenePublicInterface::AddModelInstanceToPipeline(Scene& scene, ModelInstance& modelInstance, RenderStage stage, ComponentModel& componentModel) {
    scene.m_i->_addModelInstanceToPipeline(scene, modelInstance, scene.m_RenderGraphs[(unsigned int)stage], stage, componentModel);
}
void priv::InternalScenePublicInterface::RemoveModelInstanceFromPipeline(Scene& scene, ModelInstance& modelInstance, RenderStage stage){
    scene.m_i->_removeModelInstanceFromPipeline(modelInstance, scene.m_RenderGraphs[(unsigned int)stage]);
}
void priv::InternalScenePublicInterface::SkipRenderThisFrame(Scene& scene, bool isSkip) {
    scene.m_SkipRenderThisFrame = isSkip;
}
bool priv::InternalScenePublicInterface::IsSkipRenderThisFrame(Scene& scene) {
    return scene.m_SkipRenderThisFrame;
}


Scene::Scene(const std::string& name, const SceneOptions& options) {
    m_RenderGraphs.resize((unsigned int)RenderStage::_TOTAL);
    m_ID      = priv::Core::m_Engine->m_ResourceManager.AddScene(*this);
    m_i       = std::make_unique<impl>(options.maxAmountOfParticleEmitters, options.maxAmountOfParticles);
    m_i->_init(*this, options);
    setName(name);

    registerEvent(EventType::SceneChanged);
}
Scene::Scene(const std::string& name) 
    : Scene{ name, SceneOptions::DEFAULT_OPTIONS }
{}
Scene::~Scene() {
    SAFE_DELETE(m_Skybox);

    SAFE_DELETE_VECTOR(m_SunLights);
    SAFE_DELETE_VECTOR(m_DirectionalLights);
    SAFE_DELETE_VECTOR(m_PointLights);
    SAFE_DELETE_VECTOR(m_SpotLights);
    SAFE_DELETE_VECTOR(m_RodLights);
    SAFE_DELETE_VECTOR(m_ProjectionLights);

    SAFE_DELETE_VECTOR(m_Cameras);
    unregisterEvent(EventType::SceneChanged);
}

template<class LIGHT, class CONTAINER> constexpr void internal_delete_light(LIGHT* light, CONTAINER& container) {
    light->destroy();
    removeFromVector(container, light);
    SAFE_DELETE(light);
}
void Scene::deleteSunLight(SunLight* light) {
    internal_delete_light(light, m_SunLights);
}
void Scene::deleteDirectionalLight(DirectionalLight* light) {
    internal_delete_light(light, m_DirectionalLights);
}
void Scene::deletePointLight(PointLight* light) {
    internal_delete_light(light, m_PointLights);
}
void Scene::deleteSpotLight(SpotLight* light) {
    internal_delete_light(light, m_SpotLights);
}
void Scene::deleteRodLight(RodLight* light) {
    internal_delete_light(light, m_RodLights);
}
void Scene::deleteProjectionLight(ProjectionLight* light) {
    internal_delete_light(light, m_ProjectionLights);
}
size_t Scene::getNumLights() const noexcept {
    size_t count = 0;
    count += m_SunLights.size();
    count += m_DirectionalLights.size();
    count += m_PointLights.size();
    count += m_SpotLights.size();
    count += m_RodLights.size();
    count += m_ProjectionLights.size();
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
    unsigned int id       = numViewports();
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
    if (m_Viewports.size() == 0){
        return nullptr;
    }
    return &m_Viewports[0].getCamera();
}
void Scene::setActiveCamera(Camera& camera){
    if (m_Viewports.size() == 0) {
        unsigned int id    = numViewports();
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

    priv::InternalScenePublicInterface::UpdateMaterials(*this, dt);

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
    Math::setColor(m_Viewports[0].m_BackgroundColor, r, g, b, a);
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
