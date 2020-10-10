#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/model/ModelInstance.h>
#include <core/engine/system/Engine.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/mesh/Skeleton.h>
#include <core/engine/materials/Material.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/scene/Viewport.h>
#include <core/engine/shaders/ShaderProgram.h>
#include <ecs/Components.h>

#include <core/engine/lights/Lights.h>
#include <core/engine/scene/Skybox.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/renderer/pipelines/IRenderingPipeline.h>

using namespace Engine;

unsigned int ModelInstance::m_ViewportFlagDefault = ViewportFlag::All;
decimal ModelInstance::m_GlobalDistanceFactor     = (decimal)1100.0;

namespace Engine::priv {
    constexpr auto DefaultModelInstanceBindFunctor = [](ModelInstance* i, const Engine::priv::RenderModule* renderer) {
        auto stage            = i->stage();
        auto& scene           = *Resources::getCurrentScene();
        auto* camera          = scene.getActiveCamera();
        glm::vec3 camPos      = camera->getPosition();
        auto* body            = (i->parent().getComponent<ComponentBody>());
        glm::mat4 parentModel = body->modelMatrixRendering();
        auto& animationVector = i->getRunningAnimations();

        Engine::Renderer::sendUniform1Safe("Object_Color", i->color().toPackedInt());
        Engine::Renderer::sendUniform1Safe("Gods_Rays_Color", i->godRaysColor().toPackedInt());

        if (stage == RenderStage::ForwardTransparentTrianglesSorted || stage == RenderStage::ForwardTransparent || stage == RenderStage::ForwardOpaque) {
            auto& lights     = priv::InternalScenePublicInterface::GetLights(scene);
            int maxLights    = glm::min((int)lights.size(), MAX_LIGHTS_PER_PASS);
            Engine::Renderer::sendUniform1Safe("numLights", maxLights);
            for (int i = 0; i < maxLights; ++i) {
                auto& light     = *lights[i];
                auto lightType  = light.type();
                auto start      = "light[" + std::to_string(i) + "].";
                switch (lightType) {
                    case LightType::Sun: {
                        auto& sunLight         = (SunLight&)light;
                        renderer->m_Pipeline->sendGPUDataSunLight(*camera, sunLight, start);
                        break;
                    }case LightType::Directional: {
                        auto& directionalLight = (DirectionalLight&)light;
                        renderer->m_Pipeline->sendGPUDataDirectionalLight(*camera, directionalLight, start);
                        break;
                    }case LightType::Point: {
                        auto& pointLight       = (PointLight&)light;
                        renderer->m_Pipeline->sendGPUDataPointLight(*camera, pointLight, start);
                        break;
                    }case LightType::Spot: {
                        auto& spotLight        = (SpotLight&)light;
                        renderer->m_Pipeline->sendGPUDataSpotLight(*camera, spotLight, start);
                        break;
                    }case LightType::Rod: {
                        auto& rodLight         = (RodLight&)light;
                        renderer->m_Pipeline->sendGPUDataRodLight(*camera, rodLight, start);
                        break;
                    }case LightType::Projection:{
                        auto& projectionLight  = (ProjectionLight&)light;
                        renderer->m_Pipeline->sendGPUDataProjectionLight(*camera, projectionLight, start);
                        break;
                    }default: {
                        break;
                    }
                }
            }
            Skybox* skybox          = scene.skybox();
            const auto maxTextures  = renderer->m_Pipeline->getMaxNumTextureUnits() - 1U;
            Engine::Renderer::sendUniform4Safe("ScreenData", renderer->m_GI_Pack, Engine::Renderer::Settings::getGamma(), 0.0f, 0.0f);
            if (skybox && skybox->texture().get<Texture>()->hasGlobalIlluminationData()) {
                Engine::Renderer::sendTextureSafe("irradianceMap", skybox->texture().get<Texture>()->getConvolutionTexture().get<Texture>()->address(), maxTextures - 2, GL_TEXTURE_CUBE_MAP);
                Engine::Renderer::sendTextureSafe("prefilterMap", skybox->texture().get<Texture>()->getPreEnvTexture().get<Texture>()->address(), maxTextures - 1, GL_TEXTURE_CUBE_MAP);
                Engine::Renderer::sendTextureSafe("brdfLUT", *Texture::BRDF.get<Texture>(), maxTextures);
            }else{
                Engine::Renderer::sendTextureSafe("irradianceMap", Texture::Black.get<Texture>()->address(), maxTextures - 2, GL_TEXTURE_2D);
                Engine::Renderer::sendTextureSafe("prefilterMap", Texture::Black.get<Texture>()->address(), maxTextures - 1, GL_TEXTURE_2D);
                Engine::Renderer::sendTextureSafe("brdfLUT", *Texture::BRDF.get<Texture>(), maxTextures);
            }
        }
        if (animationVector.size() > 0) {
            Engine::Renderer::sendUniform1Safe("AnimationPlaying", 1);
            Engine::Renderer::sendUniformMatrix4vSafe("gBones[0]", animationVector.getTransforms(), (unsigned int)animationVector.getTransforms().size());
        }else{
            Engine::Renderer::sendUniform1Safe("AnimationPlaying", 0);
        }
        glm::mat4 modelMatrix = parentModel * i->modelMatrix();

        //world space normals
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));

        //view space normals
        //glm::mat4 view = cam.getView();
        //glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(view * model)));

        Engine::Renderer::sendUniformMatrix4Safe("Model", modelMatrix);
        Engine::Renderer::sendUniformMatrix3Safe("NormalMatrix", normalMatrix);
    };
    constexpr auto DefaultModelInstanceUnbindFunctor = [](ModelInstance* i, const Engine::priv::RenderModule* renderer) {
        //auto& i = *static_cast<ModelInstance*>(r);
    };
};
bool priv::InternalModelInstancePublicInterface::IsViewportValid(const ModelInstance& modelInstance, const Viewport& viewport) {
    const auto flags = modelInstance.getViewportFlags();
    return (flags & (1 << viewport.id()) || flags == 0);
}

ModelInstance::ModelInstance(Entity parent, Handle mesh, Handle material, Handle shaderProgram)
    : m_Parent{ parent }
{
    internal_init(mesh, material, shaderProgram);
    setCustomBindFunctor(priv::DefaultModelInstanceBindFunctor);
    setCustomUnbindFunctor(priv::DefaultModelInstanceUnbindFunctor);
}
/*
ModelInstance::ModelInstance(Entity parent, Mesh* mesh, Material* mat, ShaderProgram* program) 
    : m_Parent{ parent }
{
    internal_init(mesh, mat, program);
    setCustomBindFunctor(priv::DefaultModelInstanceBindFunctor);
    setCustomUnbindFunctor(priv::DefaultModelInstanceUnbindFunctor);
}
ModelInstance::ModelInstance(Entity parent, Handle mesh, Handle mat, ShaderProgram* program) 
    : ModelInstance{ parent, mesh.get<Mesh>(), mat.get<Material>(), program }
{}
ModelInstance::ModelInstance(Entity parent, Mesh* mesh, Handle mat, ShaderProgram* program) 
    : ModelInstance{ parent, mesh, mat.get<Material>(), program }
{}
ModelInstance::ModelInstance(Entity parent, Handle mesh, Material* mat, ShaderProgram* program) 
    : ModelInstance{ parent, mesh.get<Mesh>(), mat, program }
{}
*/
ModelInstance::ModelInstance(ModelInstance&& other) noexcept
    : m_DrawingMode{ std::move(other.m_DrawingMode) }
    , m_AnimationVector{ std::move(other.m_AnimationVector) }
    , m_Parent{ std::move(other.m_Parent) }
    , m_Stage{ std::move(other.m_Stage) }
    , m_Position{ std::move(other.m_Position) }
    , m_Scale{ std::move(other.m_Scale) }
    , m_GodRaysColor{ std::move(other.m_GodRaysColor) }
    , m_Orientation{ std::move(other.m_Orientation) }
    , m_ModelMatrix{ std::move(other.m_ModelMatrix) }
    , m_Color{ std::move(other.m_Color) }
    , m_PassedRenderCheck{ std::move(other.m_PassedRenderCheck) }
    , m_Visible{ std::move(other.m_Visible) }
    , m_ForceRender{ std::move(other.m_ForceRender) }
    , m_Index{ std::move(other.m_Index) }
    , m_ShaderProgramHandle{ std::move(other.m_ShaderProgramHandle) }
    , m_MeshHandle{ std::move(other.m_MeshHandle) }
    , m_MaterialHandle{ std::move(other.m_MaterialHandle) }
    , m_CustomBindFunctor{ std::move(other.m_CustomBindFunctor) }
    , m_CustomUnbindFunctor{ std::move(other.m_CustomUnbindFunctor) }
    , m_ViewportFlag{ std::move(other.m_ViewportFlagDefault) }
{
    m_ViewportFlagDefault    = std::move(other.m_ViewportFlagDefault);
    m_UserPointer            = std::move(other.m_UserPointer);

    internal_calculate_radius();
}
ModelInstance& ModelInstance::operator=(ModelInstance&& other) noexcept {
    m_DrawingMode            = std::move(other.m_DrawingMode);
    m_ViewportFlagDefault    = std::move(other.m_ViewportFlagDefault);
    m_ViewportFlag           = std::move(other.m_ViewportFlagDefault);
    m_AnimationVector        = std::move(other.m_AnimationVector);
    m_Parent                 = std::move(other.m_Parent);
    m_Stage                  = std::move(other.m_Stage);
    m_Position               = std::move(other.m_Position);
    m_Scale                  = std::move(other.m_Scale);
    m_GodRaysColor           = std::move(other.m_GodRaysColor);
    m_Orientation            = std::move(other.m_Orientation);
    m_ModelMatrix            = std::move(other.m_ModelMatrix);
    m_Color                  = std::move(other.m_Color);
    m_PassedRenderCheck      = std::move(other.m_PassedRenderCheck);
    m_Visible                = std::move(other.m_Visible);
    m_ForceRender            = std::move(other.m_ForceRender);
    m_Index                  = std::move(other.m_Index);
    m_UserPointer            = std::move(other.m_UserPointer);
    m_ShaderProgramHandle    = std::move(other.m_ShaderProgramHandle);
    m_MeshHandle             = std::move(other.m_MeshHandle);
    m_MaterialHandle         = std::move(other.m_MaterialHandle);
    m_CustomBindFunctor      = std::move(other.m_CustomBindFunctor);
    m_CustomUnbindFunctor    = std::move(other.m_CustomUnbindFunctor);

    internal_calculate_radius();
    return *this;
}
ModelInstance::~ModelInstance() {
    unregisterEvent(EventType::ResourceLoaded);
}
float ModelInstance::internal_calculate_radius() {
    if (!m_MeshHandle.get<Mesh>()->isLoaded()) {
        registerEvent(EventType::ResourceLoaded);
        return 0.0f;
    }
    m_Radius = m_MeshHandle.get<Mesh>()->getRadius();
    return m_Radius;
}
void ModelInstance::bind(const Engine::priv::RenderModule& renderer) {
    m_CustomBindFunctor(this, &renderer);
}
void ModelInstance::unbind(const Engine::priv::RenderModule& renderer) {
    m_CustomUnbindFunctor(this, &renderer);
}
void ModelInstance::setDefaultViewportFlag(unsigned int flag) {
    m_ViewportFlagDefault = flag;
}
void ModelInstance::setDefaultViewportFlag(ViewportFlag::Flag flag) {
    m_ViewportFlagDefault = flag;
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
    auto* model = m_Parent.getComponent<ComponentModel>();
    if (model && recalcRadius) {
        Engine::priv::ComponentModel_Functions::CalculateRadius(*model);
    }
    Math::setFinalModelMatrix(m_ModelMatrix, m_Position, m_Orientation, m_Scale);
    if (recalcRadius) {
        internal_calculate_radius();
    }
}
void ModelInstance::setStage(RenderStage stage, ComponentModel& componentModel) {
    m_Stage = stage;
    componentModel.setStage(stage, m_Index);
}
void ModelInstance::setColor(float r, float g, float b, float a) {
    m_Color = Engine::color_vector_4(r, g, b, a);
}
void ModelInstance::setColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    m_Color = Engine::color_vector_4(r, g, b, a);
}
void ModelInstance::setColor(const glm::vec4& color){
    ModelInstance::setColor(color.r, color.g, color.b, color.a);
}
void ModelInstance::setColor(const glm::vec3& color) {
    ModelInstance::setColor(color.r, color.g, color.b, 1.0f);
}
void ModelInstance::setGodRaysColor(float r, float g, float b) {
    m_GodRaysColor = Engine::color_vector_4(r, g, b, m_GodRaysColor.a());
}
void ModelInstance::setGodRaysColor(const glm::vec3& color){
    ModelInstance::setGodRaysColor(color.r, color.g, color.b);
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
void ModelInstance::setScale(float scale) {
    ModelInstance::setScale(scale, scale, scale);
}
void ModelInstance::setScale(float x, float y, float z){
    bool recalcRadius = false;
    if (m_Scale.x != x || m_Scale.y != y || m_Scale.z != z) {
        recalcRadius = true;
    }
    m_Scale = glm::vec3(x, y, z);
    internal_update_model_matrix(recalcRadius);
}
void ModelInstance::translate(float x, float y, float z){
    m_Position += glm::vec3(x, y, z);
    internal_update_model_matrix(false);
}
void ModelInstance::rotate(float x, float y, float z){
    Math::rotate(m_Orientation, x, y, z);
    internal_update_model_matrix(false);
}
void ModelInstance::scale(float x, float y, float z){
    ModelInstance::setScale(x + m_Scale.x, y + m_Scale.y, z + m_Scale.z);
}
void ModelInstance::setPosition(const glm::vec3& v){
    ModelInstance::setPosition(v.x, v.y, v.z);
}
void ModelInstance::setScale(const glm::vec3& v){
    ModelInstance::setScale(v.x, v.y, v.z);
}
void ModelInstance::translate(const glm::vec3& v){
    ModelInstance::translate(v.x, v.y, v.z);
}
void ModelInstance::rotate(const glm::vec3& v){
    ModelInstance::rotate(v.x, v.y, v.z);
}
void ModelInstance::scale(const glm::vec3& v) {
    ModelInstance::scale(v.x, v.y, v.z);
}

void ModelInstance::setShaderProgram(Handle shaderProgram, ComponentModel& componentModel) {
    if (!shaderProgram) { 
        shaderProgram = ShaderProgram::Deferred;
    }
    componentModel.setModel(m_MeshHandle, m_MaterialHandle, m_Index, shaderProgram, m_Stage);
}
void ModelInstance::setMesh(Handle mesh, ComponentModel& componentModel){
    m_AnimationVector.clear();
    componentModel.setModel(mesh, m_MaterialHandle, m_Index, m_ShaderProgramHandle, m_Stage);
    internal_update_model_matrix();
}
void ModelInstance::setMaterial(Handle material, ComponentModel& componentModel){
    componentModel.setModel(m_MeshHandle, material, m_Index, m_ShaderProgramHandle, m_Stage);
}
void ModelInstance::playAnimation(const std::string& animationName, float start, float end, unsigned int requestedLoops){
    m_AnimationVector.emplace_animation(m_MeshHandle, animationName, start, end, requestedLoops);
}

void ModelInstance::onEvent(const Event& e) {
    if (e.type == EventType::ResourceLoaded) {
        if (e.eventResource.resource && e.eventResource.resource->type() == ResourceType::Mesh) {
            Mesh* mesh           = (Mesh*)e.eventResource.resource;
            Mesh* meshFromHandle = m_MeshHandle.get<Mesh>();
            if (meshFromHandle->isLoaded() || mesh == meshFromHandle) {
                internal_update_model_matrix();
                unregisterEvent(EventType::ResourceLoaded);
            }
        }
    }
}
