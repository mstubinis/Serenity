#include <core/engine/utils/PrecompiledHeader.h>
#include <ecs/ComponentModel.h>
#include <ecs/ComponentBody.h>
#include <ecs/ComponentCamera.h>

#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/model/ModelInstance.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/system/Engine.h>
#include <core/engine/scene/Scene.h>

using namespace Engine;
using namespace Engine::priv;

float ComponentModel_Functions::CalculateRadius(ComponentModel& modelComponent) {
    std::vector<glm::vec3> points_total; //TODO: vector.reserve for performance here?
    for (size_t i = 0; i < modelComponent.m_ModelInstances.size(); ++i) {
        auto& modelInstance     = *modelComponent.m_ModelInstances[i];
        auto& mesh              = *modelInstance.mesh();
        if (mesh == false) {
            continue;
        }
        auto modelInstanceScale = Engine::Math::Max(modelInstance.getScale());
        glm::vec3 localPosition = Engine::Math::getMatrixPosition(modelInstance.modelMatrix());
        auto positions          = mesh.getVertexData().getPositions();
        for (auto& vertexPosition : positions) {
            points_total.emplace_back(localPosition + (vertexPosition * modelInstanceScale));
        }
    }
    float     maxRadius      = 0.0f;
    glm::vec3 maxBoundingBox = glm::vec3(0.0f);
    for (auto& point : points_total) {
        auto abs_point   = glm::abs(point);
        float radius     = glm::length(abs_point);
        maxRadius        = std::max(maxRadius, radius);
        maxBoundingBox.x = std::max(maxBoundingBox.x, abs_point.x);
        maxBoundingBox.y = std::max(maxBoundingBox.y, abs_point.y);
        maxBoundingBox.z = std::max(maxBoundingBox.z, abs_point.z);
    }
    modelComponent.m_Radius          = maxRadius;
    modelComponent.m_RadiusBox       = maxBoundingBox;
    auto* body                       = modelComponent.m_Owner.getComponent<ComponentBody>();
    if (body) {
        auto bodyScale               = Engine::Math::Max(glm::vec3(body->getScale()));
        modelComponent.m_Radius     *= bodyScale;
        modelComponent.m_RadiusBox  *= bodyScale;
    }
    return modelComponent.m_Radius; //now modified by the body scale
};
void ComponentModel_Functions::RegisterDeferredMeshLoaded(ComponentModel& super, Mesh* mesh) {
    if (mesh && *mesh == false) {
        super.registerEvent(EventType::ResourceLoaded);
    }
}

#pragma region Component

ComponentModel::ComponentModel(Entity entity, Handle mesh, Handle material, ShaderProgram* shaderProgram, RenderStage stage) 
    : m_Owner{ entity }
{
    addModel(mesh, material, shaderProgram, stage);
}
ComponentModel::ComponentModel(Entity entity, Mesh* mesh, Handle material,  ShaderProgram* shaderProgram, RenderStage stage) 
    : m_Owner{ entity }
{
    addModel(mesh, material.get<Material>(), shaderProgram, stage);
}
ComponentModel::ComponentModel(Entity entity, Handle mesh, Material* material,  ShaderProgram* shaderProgram, RenderStage stage) 
    : m_Owner{ entity }
{
    addModel(mesh.get<Mesh>(), material, shaderProgram, stage);
}
ComponentModel::ComponentModel(Entity entity, Mesh* mesh, Material* material, ShaderProgram* shaderProgram, RenderStage stage) 
    : m_Owner{ entity }
{
    addModel(mesh, material, shaderProgram, stage);
}
ComponentModel::ComponentModel(Entity entity, Handle mesh, Handle material, Handle shaderProgram, RenderStage stage) 
    : m_Owner{ entity }
{
    addModel(mesh, material, shaderProgram.get<ShaderProgram>(), stage);
}
ComponentModel::ComponentModel(Entity entity, Mesh* mesh, Handle material, Handle shaderProgram, RenderStage stage) 
    : m_Owner{ entity }
{
    addModel(mesh, material.get<Material>(), shaderProgram.get<ShaderProgram>(), stage);
}
ComponentModel::ComponentModel(Entity entity, Handle mesh, Material* material, Handle shaderProgram, RenderStage stage) 
    : m_Owner{ entity }
{
    addModel(mesh.get<Mesh>(), material, shaderProgram.get<ShaderProgram>(), stage);
}
ComponentModel::ComponentModel(Entity entity, Mesh* mesh, Material* material, Handle shaderProgram, RenderStage stage) 
    : m_Owner{ entity }
{
    addModel(mesh, material, shaderProgram.get<ShaderProgram>(), stage);
}
ComponentModel::ComponentModel(ComponentModel&& other) noexcept 
    : m_Owner{ std::move(other.m_Owner) }
    , m_ModelInstances{ std::move(other.m_ModelInstances) }
    , m_Radius{ std::move(other.m_Radius) }
    , m_RadiusBox{ std::move(other.m_RadiusBox) }
{
    if (other.isRegistered(EventType::ResourceLoaded)) {
        registerEvent(EventType::ResourceLoaded);
        other.unregisterEvent(EventType::ResourceLoaded);
    }
}
ComponentModel& ComponentModel::operator=(ComponentModel&& other) noexcept {
    m_Owner          = std::move(other.m_Owner);
    m_ModelInstances = std::move(other.m_ModelInstances);
    m_Radius         = std::move(other.m_Radius);
    m_RadiusBox      = std::move(other.m_RadiusBox);

    if (other.isRegistered(EventType::ResourceLoaded)) {
        registerEvent(EventType::ResourceLoaded);
        other.unregisterEvent(EventType::ResourceLoaded);
    }
    return *this;
}
void ComponentModel::onEvent(const Event& e) {
    if (e.type == EventType::ResourceLoaded && e.eventResource.resource->type() == ResourceType::Mesh) {
        auto* mesh = (Mesh*)e.eventResource.resource;
        std::vector<Mesh*> unfinishedMeshes;
        unfinishedMeshes.reserve(m_ModelInstances.size());
        for (auto& instance : m_ModelInstances) {
            if (instance->m_Mesh && *mesh == false) {
                unfinishedMeshes.emplace_back(instance->m_Mesh);
            }
        }
        if (unfinishedMeshes.size() == 0) {
            ComponentModel_Functions::CalculateRadius(*this);
            unregisterEvent(EventType::ResourceLoaded);
        }
    }
}
void ComponentModel::setViewportFlag(unsigned int flag) {
    for (auto& model_instance : m_ModelInstances) {
        model_instance->setViewportFlag(flag);
    }
}
void ComponentModel::addViewportFlag(unsigned int flag) {
    for (auto& model_instance : m_ModelInstances) {
        model_instance->addViewportFlag(flag);
    }
}
void ComponentModel::setViewportFlag(ViewportFlag::Flag flag) {
    for (auto& model_instance : m_ModelInstances) {
        model_instance->setViewportFlag(flag);
    }
}
void ComponentModel::addViewportFlag(ViewportFlag::Flag flag) {
    for (auto& model_instance : m_ModelInstances) {
        model_instance->addViewportFlag(flag);
    }
}
void ComponentModel::show(bool shown) noexcept { 
    for (auto& modelInstance : m_ModelInstances) {
        modelInstance->show(shown);
    }
}
ModelInstanceHandle ComponentModel::addModel(Mesh* mesh, Material* material, ShaderProgram* shaderProgram, RenderStage stage) {
    ComponentModel_Functions::RegisterDeferredMeshLoaded(*this, mesh);

    auto& modelInstance    = *m_ModelInstances.emplace_back(std::make_unique<ModelInstance>(m_Owner, mesh, material, shaderProgram));
    modelInstance.m_Stage  = stage;
    modelInstance.m_Index  = m_ModelInstances.size() - 1U;

    InternalScenePublicInterface::AddModelInstanceToPipeline(m_Owner.scene(), modelInstance, stage, *this);
    ComponentModel_Functions::CalculateRadius(*this);
    return { modelInstance.m_Index, *this };
}
ModelInstanceHandle ComponentModel::addModel(Handle mesh, Handle material, ShaderProgram* shaderProgram, RenderStage stage) {
    return ComponentModel::addModel(mesh.get<Mesh>(), material.get<Material>(), shaderProgram, stage);
}
ModelInstanceHandle ComponentModel::addModel(Handle mesh, Handle material, Handle shaderProgram, RenderStage stage) {
    return ComponentModel::addModel(mesh.get<Mesh>(), material.get<Material>(), shaderProgram.get<ShaderProgram>(), stage);
}
ModelInstanceHandle ComponentModel::addModel(Mesh* mesh, Material* material, Handle shaderProgram, RenderStage stage) {
    return ComponentModel::addModel(mesh, material, shaderProgram.get<ShaderProgram>(), stage);
}


void ComponentModel::setModel(Handle mesh, Handle material, size_t index, ShaderProgram* shaderProgram, RenderStage stage) {
    ComponentModel::setModel(mesh.get<Mesh>(), material.get<Material>(), index, shaderProgram, stage);
}
void ComponentModel::setModel(Mesh* mesh, Material* material, size_t index, ShaderProgram* shaderProgram, RenderStage stage) {
    ComponentModel_Functions::RegisterDeferredMeshLoaded(*this, mesh);

    auto& model_instance           = *m_ModelInstances[index];
    auto& scene                    = m_Owner.scene();
    InternalScenePublicInterface::RemoveModelInstanceFromPipeline(scene, model_instance, model_instance.stage());

    model_instance.m_ShaderProgram = shaderProgram;
    model_instance.m_Mesh          = mesh;
    model_instance.m_Material      = material;
    model_instance.m_Stage         = stage;

    InternalScenePublicInterface::AddModelInstanceToPipeline(scene, model_instance, stage, *this);
    ComponentModel_Functions::CalculateRadius(*this);
}
void ComponentModel::setModelShaderProgram(ShaderProgram* shaderProgram, size_t index, RenderStage stage) {
    auto& model_instance           = *m_ModelInstances[index];
    auto& scene                    = m_Owner.scene();
    InternalScenePublicInterface::RemoveModelInstanceFromPipeline(scene, model_instance, model_instance.stage());

    model_instance.m_ShaderProgram = shaderProgram;
    model_instance.m_Stage         = stage;

    InternalScenePublicInterface::AddModelInstanceToPipeline(scene, model_instance, stage, *this);
    ComponentModel_Functions::CalculateRadius(*this);
}
void ComponentModel::setModelShaderProgram(Handle shaderPHandle, size_t index, RenderStage stage) {
    ComponentModel::setModelShaderProgram(shaderPHandle.get<ShaderProgram>(), index, stage); 
}
void ComponentModel::setStage(RenderStage stage, size_t index) {
    auto& model_instance   = *m_ModelInstances[index];
    auto& scene            = m_Owner.scene();
    InternalScenePublicInterface::RemoveModelInstanceFromPipeline(scene, model_instance, model_instance.stage());

    model_instance.m_Stage = stage;

    InternalScenePublicInterface::AddModelInstanceToPipeline(scene, model_instance, stage, *this);
}
void ComponentModel::setModelMesh(Mesh* mesh, size_t index, RenderStage stage) {
    ComponentModel_Functions::RegisterDeferredMeshLoaded(*this, mesh);

    auto& model_instance   = *m_ModelInstances[index];
    auto& scene            = m_Owner.scene();

    InternalScenePublicInterface::RemoveModelInstanceFromPipeline(scene, model_instance, model_instance.stage());

    model_instance.m_Mesh  = mesh;
    model_instance.m_Stage = stage;

    InternalScenePublicInterface::AddModelInstanceToPipeline(scene, model_instance, stage, *this);
    ComponentModel_Functions::CalculateRadius(*this);
}
void ComponentModel::setModelMesh(Handle mesh, size_t index, RenderStage stage) {
    ComponentModel::setModelMesh(mesh.get<Mesh>(), index, stage); 
}
void ComponentModel::setModelMaterial(Material* material, size_t index, RenderStage stage) {
    auto& model_instance      = *m_ModelInstances[index];
    auto& scene               = m_Owner.scene();
    InternalScenePublicInterface::RemoveModelInstanceFromPipeline(scene, model_instance, model_instance.stage());

    model_instance.m_Material = material;
    model_instance.m_Stage    = stage;

    InternalScenePublicInterface::AddModelInstanceToPipeline(scene, model_instance, stage, *this);
}
void ComponentModel::setModelMaterial(Handle material, size_t index, RenderStage stage) {
    ComponentModel::setModelMaterial(material.get<Material>(), index, stage);
}
bool ComponentModel::rayIntersectSphere(const ComponentCamera& camera) const {
    const auto& body = *m_Owner.getComponent<ComponentBody>();
    return Math::rayIntersectSphere(body.getPosition(), m_Radius, camera.m_Eye, camera.getViewVector());
}
void ComponentModel::setUserPointer(void* UserPointer) {
    for (auto& instance : m_ModelInstances) {
        instance->setUserPointer(UserPointer);
    }
}


#pragma endregion

#pragma region System

struct priv::ComponentModel_UpdateFunction final { void operator()(void* system, void* componentPool, const float dt, Scene& scene) const {
    auto& pool       = *(ECSComponentPool<ComponentModel>*)componentPool;
    auto& components = pool.data();
    auto lamda_update_component = [&](ComponentModel& componentModel, size_t i, size_t k) {
        for (size_t j = 0; j < componentModel.getNumModels(); ++j) {
            auto& modelInstance = componentModel[j];
            //process the animations here
            modelInstance.m_AnimationVector.process(*modelInstance.mesh(), dt);
        }
    };
    if (components.size() < 100) {
        for (size_t i = 0; i < components.size(); ++i) {
            lamda_update_component(components[i], i, 0);
        }
    }else{
        Engine::priv::threading::addJobSplitVectored(lamda_update_component, components, true, 0);
    }
}};

ComponentModel_System_CI::ComponentModel_System_CI() {
    setUpdateFunction(ComponentModel_UpdateFunction());
    setOnComponentAddedToEntityFunction([](void* system, void* component, Entity entity) {
        auto* component_ptr = (ComponentModel*)component;
        if (component_ptr) {
            ComponentModel_Functions::CalculateRadius(*component_ptr);
        }
    });
    //setOnComponentRemovedFromEntityFunction([](void* system, Entity entity) { });
    setOnEntityAddedToSceneFunction([](void* system, void* componentPool, Entity entity, Scene& scene) {
        auto& pool          = *(ECSComponentPool<ComponentModel>*)componentPool;
        auto* component_ptr = pool.getComponent(entity);
        if (component_ptr) {
            ComponentModel_Functions::CalculateRadius(*component_ptr);
        }
    });
    //setOnSceneEnteredFunction([](void* system, void* componentPool, Scene& scene) { });
    //setOnSceneLeftFunction([](void* system, void* componentPool, Scene& scene) { });
}

#pragma endregion