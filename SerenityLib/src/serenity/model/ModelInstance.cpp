
#include <serenity/model/ModelInstance.h>
#include <serenity/system/Engine.h>
#include <serenity/math/Engine_Math.h>
#include <serenity/resources/mesh/Mesh.h>
#include <serenity/resources/mesh/animation/Skeleton.h>
#include <serenity/resources/material/Material.h>
#include <serenity/scene/Camera.h>
#include <serenity/scene/Scene.h>
#include <serenity/scene/Viewport.h>
#include <serenity/resources/shader/ShaderProgram.h>
#include <serenity/ecs/components/Components.h>

#include <serenity/lights/Lights.h>
#include <serenity/scene/Skybox.h>
#include <serenity/resources/texture/Texture.h>
#include <serenity/renderer/pipelines/IRenderingPipeline.h>

using namespace Engine;

uint32_t ModelInstance::m_ViewportFlagDefault = ViewportFlag::All;
decimal ModelInstance::m_GlobalDistanceFactor = decimal(1100.0);

constexpr auto DefaultModelInstanceBindFunctor = [](ModelInstance* i, const Engine::priv::RenderModule* renderer) {
    const auto stage                   = i->getStage();
    auto& scene                        = *Engine::Resources::getCurrentScene();
    auto* camera                       = scene.getActiveCamera();
    const Entity parent                = i->getOwner();
    const auto transform               = parent.getComponent<ComponentTransform>();
    const glm::mat4 parentWorldMatrix  = transform->getWorldMatrixRendering();
    const auto& animationContainer     = i->getRunningAnimations();

    Engine::Renderer::sendUniform1Safe("Object_Color", i->getColor().toPackedInt());
    Engine::Renderer::sendUniform1Safe("Gods_Rays_Color", i->getGodRaysColor().toPackedInt());

    if (stage == RenderStage::ForwardTransparentTrianglesSorted || stage == RenderStage::ForwardTransparent || stage == RenderStage::ForwardOpaque) {
        renderer->m_Pipeline->sendGPUDataAllLights(scene, *camera);
        renderer->m_Pipeline->sendGPUDataGI(scene.skybox());
    }
    if (animationContainer.size() > 0) {
        Engine::Renderer::sendUniform1Safe("AnimationPlaying", 1);
        Engine::Renderer::sendUniformMatrix4vSafe("gBones[0]", animationContainer.getTransforms(), (uint32_t)animationContainer.getTransforms().size());
    } else {
        Engine::Renderer::sendUniform1Safe("AnimationPlaying", 0);
    }
    const glm::mat4 renderingMatrix = parentWorldMatrix * i->getModelMatrix();

    //world space normals
    const glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3{ renderingMatrix }));

    //view space normals
    //const glm::mat4 view = cam.getView();
    //const glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3{ view * model }));

    Engine::Renderer::sendUniformMatrix4("Model", renderingMatrix);
    Engine::Renderer::sendUniformMatrix3Safe("NormalMatrix", normalMatrix);
};
constexpr auto DefaultModelInstanceUnbindFunctor = [](ModelInstance* i, const Engine::priv::RenderModule* renderer) {
    //auto& i = *static_cast<ModelInstance*>(r);
};

bool priv::PublicModelInstance::IsViewportValid(const ModelInstance& modelInstance, const Viewport& viewport) {
    const auto flags = modelInstance.getViewportFlags();
    return flags == 0 || flags & (1 << viewport.getId());
}

ModelInstance::ModelInstance(Entity parent, Handle mesh, Handle material, Handle shaderProgram)
    : m_Owner{ parent }
{
    internal_init(mesh, material, shaderProgram);
    setCustomBindFunctor(DefaultModelInstanceBindFunctor);
    setCustomUnbindFunctor(DefaultModelInstanceUnbindFunctor);
}
ModelInstance::ModelInstance(ModelInstance&& other) noexcept
    : m_ModelMatrix        { std::move(other.m_ModelMatrix) }
    , m_Orientation        { std::move(other.m_Orientation) }
    , m_Position           { std::move(other.m_Position) }
    , m_Scale              { std::move(other.m_Scale) }
    , m_CustomBindFunctor  { std::move(other.m_CustomBindFunctor) }
    , m_CustomUnbindFunctor{ std::move(other.m_CustomUnbindFunctor) }
    , m_DrawingMode        { std::move(other.m_DrawingMode) }
    , m_ViewportFlag       { std::move(other.m_ViewportFlag) }
    , m_Animations         { std::move(other.m_Animations) }
    , m_Owner              { std::move(other.m_Owner) }
    , m_ShaderProgramHandle{ std::move(other.m_ShaderProgramHandle) }
    , m_MeshHandle         { std::move(other.m_MeshHandle) }
    , m_MaterialHandle     { std::move(other.m_MaterialHandle) }
    , m_Stage              { std::move(other.m_Stage) }
    , m_GodRaysColor       { std::move(other.m_GodRaysColor) }
    , m_Color              { std::move(other.m_Color) }
    , m_UserPointer        { std::exchange(other.m_UserPointer, nullptr) }
    , m_Radius             { std::move(other.m_Radius) }
    , m_Index              { std::move(other.m_Index) }
    , m_PassedRenderCheck  { std::move(other.m_PassedRenderCheck) }
    , m_Visible            { std::move(other.m_Visible) }
    , m_ForceRender        { std::move(other.m_ForceRender) }
    , m_IsShadowCaster     { std::move(other.m_IsShadowCaster) }
{
    //internal_calculate_radius();
    if (other.isRegistered(EventType::ResourceLoaded)) {
        registerEvent(EventType::ResourceLoaded);
    }
}
ModelInstance& ModelInstance::operator=(ModelInstance&& other) noexcept {
    if (this != &other) {
        m_ModelMatrix         = std::move(other.m_ModelMatrix);
        m_Orientation         = std::move(other.m_Orientation);
        m_Position            = std::move(other.m_Position);
        m_Scale               = std::move(other.m_Scale);
        m_CustomBindFunctor   = std::move(other.m_CustomBindFunctor);
        m_CustomUnbindFunctor = std::move(other.m_CustomUnbindFunctor);
        m_DrawingMode         = std::move(other.m_DrawingMode);
        m_ViewportFlag        = std::move(other.m_ViewportFlag);
        m_Animations          = std::move(other.m_Animations);
        m_Owner               = std::move(other.m_Owner);
        m_ShaderProgramHandle = std::move(other.m_ShaderProgramHandle);
        m_MeshHandle          = std::move(other.m_MeshHandle);
        m_MaterialHandle      = std::move(other.m_MaterialHandle);
        m_Stage               = std::move(other.m_Stage);
        m_GodRaysColor        = std::move(other.m_GodRaysColor);
        m_Color               = std::move(other.m_Color);
        m_UserPointer         = std::exchange(other.m_UserPointer, nullptr);
        m_Radius              = std::move(other.m_Radius);
        m_Index               = std::move(other.m_Index);
        m_PassedRenderCheck   = std::move(other.m_PassedRenderCheck);
        m_Visible             = std::move(other.m_Visible);
        m_ForceRender         = std::move(other.m_ForceRender);
        m_IsShadowCaster      = std::move(other.m_IsShadowCaster);
        //internal_calculate_radius();
        unregisterEvent(EventType::ResourceLoaded);
        if (other.isRegistered(EventType::ResourceLoaded)) {
            registerEvent(EventType::ResourceLoaded);
        }
    }
    return *this;
}
ModelInstance::~ModelInstance() {
    unregisterEvent(EventType::ResourceLoaded);
}
float ModelInstance::internal_calculate_radius() {
    auto mesh = m_MeshHandle.get<Mesh>();
    if (!mesh->isLoaded()) {
        registerEvent(EventType::ResourceLoaded);
        return 0.0f;
    }
    m_Radius = mesh->getRadius();
    return m_Radius;
}
void ModelInstance::internal_init(Handle mesh, Handle material, Handle shaderProgram) {
    if (!shaderProgram) {
        shaderProgram     = ShaderProgram::Deferred;
    }
    m_ViewportFlag        = ModelInstance::m_ViewportFlagDefault;
    m_ShaderProgramHandle = shaderProgram;
    m_MaterialHandle      = material;
    m_MeshHandle          = mesh;
    internal_update_model_matrix();
}
void ModelInstance::internal_update_model_matrix(bool recalcRadius) {
    if (recalcRadius) {
        internal_calculate_radius();
    }
    auto model = m_Owner.getComponent<ComponentModel>();
    if (model && recalcRadius) {
        Engine::priv::ComponentModel_Functions::CalculateRadius(*model);
    }
    Math::setFinalModelMatrix(m_ModelMatrix, m_Position, m_Orientation, m_Scale);
}
void ModelInstance::setStage(RenderStage stage, ComponentModel& componentModel) {
    m_Stage = stage;
    componentModel.setStage(stage, m_Index);
}
void ModelInstance::setPosition(float x, float y, float z){
    m_Position = glm::vec3(x, y, z);
    internal_update_model_matrix(false);
}
void ModelInstance::setOrientation(const glm::quat& orientation) {
    m_Orientation = orientation;
    internal_update_model_matrix(false);
}
void ModelInstance::setOrientation(float x, float y, float z) {
    Math::setRotation(m_Orientation, x, y, z);
    internal_update_model_matrix(false);
}
void ModelInstance::setScale(float x, float y, float z){
    bool recalcRadius = false;
    if (m_Scale.x != x || m_Scale.y != y || m_Scale.z != z) {
        recalcRadius = true;
    }
    m_Scale = glm::vec3(x, y, z);
    internal_update_model_matrix(recalcRadius);
}
void ModelInstance::translate(float x, float y, float z) {
    m_Position += glm::vec3(x, y, z);
    internal_update_model_matrix(false);
}
void ModelInstance::rotate(float x, float y, float z, bool local){
    Math::rotate(m_Orientation, x, y, z, local);
    internal_update_model_matrix(false);
}

void ModelInstance::setShaderProgram(Handle shaderProgram, ComponentModel& componentModel) {
    if (!shaderProgram) { 
        shaderProgram = ShaderProgram::Deferred;
    }
    componentModel.setModel(m_MeshHandle, m_MaterialHandle, m_Index, shaderProgram, m_Stage);
}
void ModelInstance::setMesh(Handle mesh, ComponentModel& componentModel){
    m_Animations.clear();
    componentModel.setModel(mesh, m_MaterialHandle, m_Index, m_ShaderProgramHandle, m_Stage);
    internal_update_model_matrix();
}
void ModelInstance::setMaterial(Handle material, ComponentModel& componentModel){
    componentModel.setModel(m_MeshHandle, material, m_Index, m_ShaderProgramHandle, m_Stage);
}
void ModelInstance::onEvent(const Event& e) {
    if (e.type == EventType::ResourceLoaded && e.eventResource.resource->type() == ResourceType::Mesh) {
        //Mesh* mesh           = static_cast<Mesh*>(e.eventResource.resource);
        Mesh* meshFromHandle = m_MeshHandle.get<Mesh>();
        if (meshFromHandle->isLoaded()) {
            internal_update_model_matrix(true);
            unregisterEvent(EventType::ResourceLoaded);
        }
    }  
}
