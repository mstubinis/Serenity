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

using namespace Engine;
using namespace Engine::priv;
using namespace std;

float ComponentModel_Functions::CalculateRadius(ComponentModel& modelComponent) {
    vector<glm::vec3> points_total; //TODO: vector.reserve for performance here?
    for (size_t i = 0; i < modelComponent.m_ModelInstances.size(); ++i) {
        auto& modelInstance            = *modelComponent.m_ModelInstances[i];
        auto& mesh                     = *modelInstance.mesh();
        if (mesh == false) {
            continue;
        }
        auto modelInstanceScale = Math::Max(modelInstance.getScale());
        glm::vec3 localPosition = Math::getMatrixPosition(modelInstance.modelMatrix());
        auto positions          = mesh.getVertexData().getPositions();
        for (auto& vertexPosition : positions) {
            points_total.push_back(localPosition + (vertexPosition * modelInstanceScale));
        }
    }
    float     maxRadius      = 0.0f;
    glm::vec3 maxBoundingBox = glm::vec3(0.0f);
    for (auto& point : points_total) {
        auto abs_point   = glm::abs(point);
        float radius     = glm::length(abs_point);
        maxRadius        = max(maxRadius, radius);
        maxBoundingBox.x = max(maxBoundingBox.x, abs_point.x);
        maxBoundingBox.y = max(maxBoundingBox.y, abs_point.y);
        maxBoundingBox.z = max(maxBoundingBox.z, abs_point.z);
    }
    modelComponent.m_Radius          = maxRadius;
    modelComponent.m_RadiusBox       = maxBoundingBox;
    auto* body                       = modelComponent.m_Owner.getComponent<ComponentBody>();
    if (body) {
        auto bodyScale               =  Math::Max(glm::vec3(body->getScale()));
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

ComponentModel::ComponentModel(Entity entity, Handle mesh, Handle material, ShaderProgram* shaderProgram, RenderStage stage) {
    m_Owner = entity;
    addModel(mesh, material, shaderProgram, stage);
}
ComponentModel::ComponentModel(Entity entity, Mesh* mesh, Handle material,  ShaderProgram* shaderProgram, RenderStage stage) {
    m_Owner = entity;
    addModel(mesh, material.get<Material>(), shaderProgram, stage);
}
ComponentModel::ComponentModel(Entity entity, Handle mesh, Material* material,  ShaderProgram* shaderProgram, RenderStage stage) {
    m_Owner = entity;
    addModel(mesh.get<Mesh>(), material, shaderProgram, stage);
}
ComponentModel::ComponentModel(Entity entity, Mesh* mesh, Material* material, ShaderProgram* shaderProgram, RenderStage stage) {
    m_Owner = entity;
    addModel(mesh, material, shaderProgram, stage);
}
ComponentModel::ComponentModel(Entity entity, Handle mesh, Handle material, Handle shaderProgram, RenderStage stage) {
    m_Owner = entity;
    addModel(mesh, material, shaderProgram.get<ShaderProgram>(), stage);
}
ComponentModel::ComponentModel(Entity entity, Mesh* mesh, Handle material, Handle shaderProgram, RenderStage stage) {
    m_Owner = entity;
    addModel(mesh, material.get<Material>(), shaderProgram.get<ShaderProgram>(), stage);
}
ComponentModel::ComponentModel(Entity entity, Handle mesh, Material* material, Handle shaderProgram, RenderStage stage) {
    m_Owner = entity;
    addModel(mesh.get<Mesh>(), material, shaderProgram.get<ShaderProgram>(), stage);
}
ComponentModel::ComponentModel(Entity entity, Mesh* mesh, Material* material, Handle shaderProgram, RenderStage stage) {
    m_Owner = entity;
    addModel(mesh, material, shaderProgram.get<ShaderProgram>(), stage);
}
ComponentModel::ComponentModel(ComponentModel&& other) noexcept {
    m_Owner          = std::move(other.m_Owner);
    m_ModelInstances = std::move(other.m_ModelInstances);
    m_Radius         = std::move(other.m_Radius);
    m_RadiusBox      = std::move(other.m_RadiusBox);

    if (other.isRegistered(EventType::ResourceLoaded)) {
        registerEvent(EventType::ResourceLoaded);
        other.unregisterEvent(EventType::ResourceLoaded);
    }
}
ComponentModel& ComponentModel::operator=(ComponentModel&& other) noexcept {
    if (&other != this) {
        m_Owner          = std::move(other.m_Owner);
        m_ModelInstances = std::move(other.m_ModelInstances);
        m_Radius         = std::move(other.m_Radius);
        m_RadiusBox      = std::move(other.m_RadiusBox);

        if (other.isRegistered(EventType::ResourceLoaded)) {
            registerEvent(EventType::ResourceLoaded);
            other.unregisterEvent(EventType::ResourceLoaded);
        }
    }
    return *this;
}
ComponentModel::~ComponentModel() {
    SAFE_DELETE_VECTOR(m_ModelInstances);
}
Entity ComponentModel::getOwner() const {
    return m_Owner;
}
void ComponentModel::onEvent(const Event& event_) {
    if (event_.type == EventType::ResourceLoaded && event_.eventResource.resource->type() == ResourceType::Mesh) {
        auto* mesh = (Mesh*)event_.eventResource.resource;
        vector<Mesh*> unfinishedMeshes;
        unfinishedMeshes.reserve(m_ModelInstances.size());
        for (auto& instance : m_ModelInstances) {
            auto& mesh = *instance->m_Mesh;
            if (instance->m_Mesh && mesh == false) {
                unfinishedMeshes.emplace_back(instance->m_Mesh);
            }
        }
        if (unfinishedMeshes.size() == 0) {
            ComponentModel_Functions::CalculateRadius(*this);
            unregisterEvent(EventType::ResourceLoaded);
        }
    }
}
void ComponentModel::removeModel(size_t index) {
    auto* ptr = m_ModelInstances[index];
    m_ModelInstances.erase(m_ModelInstances.begin() + index);
    SAFE_DELETE(ptr);
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
size_t ComponentModel::getNumModels() const {
    return m_ModelInstances.size();
}
ModelInstance& ComponentModel::getModel(size_t index) const {
    return *m_ModelInstances[index];
}
void ComponentModel::show() { 
    for (auto& modelInstance : m_ModelInstances) {
        modelInstance->show();
    }
}
void ComponentModel::hide() { 
    for (auto& modelInstance : m_ModelInstances) {
        modelInstance->hide();
    }
}
float ComponentModel::radius() const {
    return m_Radius; 
}
const glm::vec3& ComponentModel::boundingBox() const {
    return m_RadiusBox;
}
ModelInstance& ComponentModel::addModel(Handle mesh, Handle material, ShaderProgram* shaderProgram, RenderStage stage) {
    return ComponentModel::addModel(mesh.get<Mesh>(), material.get<Material>(), shaderProgram, stage);
}
ModelInstance& ComponentModel::addModel(Mesh* mesh, Material* material, ShaderProgram* shaderProgram, RenderStage stage) {
    ComponentModel_Functions::RegisterDeferredMeshLoaded(*this, mesh);

    auto modelInstance      = NEW ModelInstance(m_Owner, mesh, material, shaderProgram);
    auto& scene             = m_Owner.scene();
    modelInstance->m_Stage  = stage;

    const auto index        = m_ModelInstances.size();
    modelInstance->m_Index  = index;
    m_ModelInstances.push_back(std::move(modelInstance));

    InternalScenePublicInterface::AddModelInstanceToPipeline(scene, *modelInstance, stage);
    ComponentModel_Functions::CalculateRadius(*this);
    return *modelInstance;
}

ModelInstance& ComponentModel::addModel(Handle mesh, Handle material, Handle shaderProgram, RenderStage stage) {
    return ComponentModel::addModel(mesh.get<Mesh>(), material.get<Material>(), shaderProgram.get<ShaderProgram>(), stage);
}
ModelInstance& ComponentModel::addModel(Mesh* mesh, Material* material, Handle shaderProgram, RenderStage stage) {
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

    InternalScenePublicInterface::AddModelInstanceToPipeline(scene, model_instance, stage);
    ComponentModel_Functions::CalculateRadius(*this);
}
void ComponentModel::setModelShaderProgram(ShaderProgram* shaderProgram, size_t index, RenderStage stage) {
    auto& model_instance           = *m_ModelInstances[index];
    auto& scene                    = m_Owner.scene();
    InternalScenePublicInterface::RemoveModelInstanceFromPipeline(scene, model_instance, model_instance.stage());

    model_instance.m_ShaderProgram = shaderProgram;
    model_instance.m_Stage         = stage;

    InternalScenePublicInterface::AddModelInstanceToPipeline(scene, model_instance, stage);
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

    InternalScenePublicInterface::AddModelInstanceToPipeline(scene, model_instance, stage);
}
void ComponentModel::setModelMesh(Mesh* mesh, size_t index, RenderStage stage) {
    ComponentModel_Functions::RegisterDeferredMeshLoaded(*this, mesh);

    auto& model_instance   = *m_ModelInstances[index];
    auto& scene            = m_Owner.scene();

    InternalScenePublicInterface::RemoveModelInstanceFromPipeline(scene, model_instance, model_instance.stage());

    model_instance.m_Mesh  = mesh;
    model_instance.m_Stage = stage;

    InternalScenePublicInterface::AddModelInstanceToPipeline(scene, model_instance, stage);
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

    InternalScenePublicInterface::AddModelInstanceToPipeline(scene, model_instance, stage);
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
    auto& pool       = *static_cast<ECSComponentPool<Entity, ComponentModel>*>(componentPool);
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
    });
    setOnComponentRemovedFromEntityFunction([](void* system, Entity entity) {
    });
    setOnEntityAddedToSceneFunction([](void* system, void* componentPool, Entity entity, Scene& scene) {
    });
    setOnSceneEnteredFunction([](void* system, void* componentPool, Scene& scene) {
    });
    setOnSceneLeftFunction([](void* system, void* componentPool, Scene& scene) {
    });
}

#pragma endregion