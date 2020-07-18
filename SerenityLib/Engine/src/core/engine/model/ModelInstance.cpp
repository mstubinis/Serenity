#include "core/engine/utils/PrecompiledHeader.h"
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
using namespace std;


unsigned int ModelInstance::m_ViewportFlagDefault = ViewportFlag::All;
decimal ModelInstance::m_GlobalDistanceFactor     = (decimal)1100.0;

namespace Engine::priv {
    struct DefaultModelInstanceBindFunctor {void operator()(ModelInstance* i, const Engine::priv::Renderer* renderer) const {
        auto stage            = i->stage();
        auto& scene           = *Resources::getCurrentScene();
        auto* camera          = scene.getActiveCamera();
        glm::vec3 camPos      = camera->getPosition();
        auto* body            = (i->m_Parent.getComponent<ComponentBody>());
        glm::mat4 parentModel = body->modelMatrixRendering();
        auto& animationVector = i->m_AnimationVector;

        Engine::Renderer::sendUniform1Safe("Object_Color", i->m_Color.toPackedInt());
        Engine::Renderer::sendUniform1Safe("Gods_Rays_Color", i->m_GodRaysColor.toPackedInt());

        if (stage == RenderStage::ForwardTransparentTrianglesSorted || stage == RenderStage::ForwardTransparent || stage == RenderStage::ForwardOpaque) {
            auto& lights     = priv::InternalScenePublicInterface::GetLights(scene);
            int maxLights    = glm::min((int)lights.size(), MAX_LIGHTS_PER_PASS);
            Engine::Renderer::sendUniform1Safe("numLights", maxLights);
            for (int i = 0; i < maxLights; ++i) {
                auto& light     = *lights[i];
                auto lightType  = light.type();
                auto start      = "light[" + to_string(i) + "].";
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
            if (skybox && skybox->texture()->numAddresses() >= 3) {
                Engine::Renderer::sendTextureSafe("irradianceMap", skybox->texture()->address(1), maxTextures - 2, GL_TEXTURE_CUBE_MAP);
                Engine::Renderer::sendTextureSafe("prefilterMap", skybox->texture()->address(2), maxTextures - 1, GL_TEXTURE_CUBE_MAP);
                Engine::Renderer::sendTextureSafe("brdfLUT", *Texture::BRDF, maxTextures);
            }else{
                Engine::Renderer::sendTextureSafe("irradianceMap", Texture::Black->address(0), maxTextures - 2, GL_TEXTURE_2D);
                Engine::Renderer::sendTextureSafe("prefilterMap", Texture::Black->address(0), maxTextures - 1, GL_TEXTURE_2D);
                Engine::Renderer::sendTextureSafe("brdfLUT", *Texture::BRDF, maxTextures);
            }
        }
        if (animationVector.size() > 0) {
            Engine::Renderer::sendUniform1Safe("AnimationPlaying", 1);
            Engine::Renderer::sendUniformMatrix4vSafe("gBones[0]", animationVector.m_Transforms, (unsigned int)animationVector.m_Transforms.size());
        }else{
            Engine::Renderer::sendUniform1Safe("AnimationPlaying", 0);
        }
        glm::mat4 modelMatrix = parentModel * i->m_ModelMatrix;

        //world space normals
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));

        //view space normals
        //glm::mat4 view = cam.getView();
        //glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(view * model)));

        Engine::Renderer::sendUniformMatrix4Safe("Model", modelMatrix);
        Engine::Renderer::sendUniformMatrix3Safe("NormalMatrix", normalMatrix);
    }};
    struct DefaultModelInstanceUnbindFunctor {void operator()(ModelInstance* i, const Engine::priv::Renderer* renderer) const {
        //auto& i = *static_cast<ModelInstance*>(r);
    }};
};
const bool priv::InternalModelInstancePublicInterface::IsViewportValid(const ModelInstance& modelInstance, const Viewport& viewport) {
    const auto flags = modelInstance.getViewportFlags();
    return (flags & (1 << viewport.id()) || flags == 0);
}



ModelInstance::ModelInstance(Entity parent, Mesh* mesh, Material* mat, ShaderProgram* program) : m_Parent(parent){
    internal_init(mesh, mat, program);
    setCustomBindFunctor(priv::DefaultModelInstanceBindFunctor());
    setCustomUnbindFunctor(priv::DefaultModelInstanceUnbindFunctor());
}
ModelInstance::ModelInstance(Entity parent, Handle mesh, Handle mat, ShaderProgram* program) : ModelInstance(parent, mesh.get<Mesh>(), mat.get<Material>(), program) {
}
ModelInstance::ModelInstance(Entity parent, Mesh* mesh, Handle mat, ShaderProgram* program) : ModelInstance(parent, mesh, mat.get<Material>(), program) {
}
ModelInstance::ModelInstance(Entity parent, Handle mesh, Material* mat, ShaderProgram* program) : ModelInstance(parent, mesh.get<Mesh>(), mat, program) {
}
ModelInstance::ModelInstance(ModelInstance&& other) noexcept {
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
    m_UserPointer            = std::exchange(other.m_UserPointer, nullptr);
    m_ShaderProgram          = std::exchange(other.m_ShaderProgram, nullptr);
    m_Mesh                   = std::exchange(other.m_Mesh, nullptr);
    m_Material               = std::exchange(other.m_Material, nullptr);
    m_CustomBindFunctor.swap(other.m_CustomBindFunctor);
    m_CustomUnbindFunctor.swap(other.m_CustomUnbindFunctor);
}
ModelInstance& ModelInstance::operator=(ModelInstance&& other) noexcept {
    if (&other != this) {
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
        m_UserPointer            = std::exchange(other.m_UserPointer, nullptr);
        m_ShaderProgram          = std::exchange(other.m_ShaderProgram, nullptr);
        m_Mesh                   = std::exchange(other.m_Mesh, nullptr);
        m_Material               = std::exchange(other.m_Material, nullptr);
        m_CustomBindFunctor.swap(other.m_CustomBindFunctor);
        m_CustomUnbindFunctor.swap(other.m_CustomUnbindFunctor);
    }
    return *this;
}
ModelInstance::~ModelInstance() {
}
void ModelInstance::setGlobalDistanceFactor(decimal factor) {
    ModelInstance::m_GlobalDistanceFactor = factor;
}
decimal ModelInstance::getGlobalDistanceFactor() {
    return ModelInstance::m_GlobalDistanceFactor;
}
void ModelInstance::bind(const Engine::priv::Renderer& renderer) {
    m_CustomBindFunctor(this, &renderer);
}
void ModelInstance::unbind(const Engine::priv::Renderer& renderer) {
    m_CustomUnbindFunctor(this, &renderer);
}
void ModelInstance::setDefaultViewportFlag(unsigned int flag) {
    m_ViewportFlagDefault = flag;
}
void ModelInstance::setDefaultViewportFlag(ViewportFlag::Flag flag) {
    m_ViewportFlagDefault = flag;
}
void ModelInstance::internal_init(Mesh* mesh, Material* mat, ShaderProgram* program) {
    if (!program) {
        program = ShaderProgram::Deferred;
    }
    m_ViewportFlag      = ModelInstance::m_ViewportFlagDefault;
    m_ShaderProgram     = program;
    m_Material          = mat;
    m_Mesh              = mesh;
    internal_update_model_matrix();
}
size_t ModelInstance::index() const {
    return m_Index;
}
ModelDrawingMode::Mode ModelInstance::getDrawingMode() const {
    return m_DrawingMode;
}
void ModelInstance::setDrawingMode(ModelDrawingMode::Mode drawMode) {
    m_DrawingMode = drawMode;
}
void ModelInstance::forceRender(bool forced) {
    m_ForceRender = forced;
}
bool ModelInstance::isForceRendered() const {
    return m_ForceRender;
}
void ModelInstance::setViewportFlag(unsigned int flag) {
    m_ViewportFlag = flag;
}
void ModelInstance::addViewportFlag(unsigned int flag) {
    m_ViewportFlag.add(flag);
}
void ModelInstance::removeViewportFlag(unsigned int flag) {
    m_ViewportFlag.remove(flag);
}
void ModelInstance::setViewportFlag(ViewportFlag::Flag flag) {
    m_ViewportFlag = flag;
}
void ModelInstance::addViewportFlag(ViewportFlag::Flag flag) {
    m_ViewportFlag.add(flag);
}
void ModelInstance::removeViewportFlag(ViewportFlag::Flag flag) {
    m_ViewportFlag.remove(flag);
}
unsigned int ModelInstance::getViewportFlags() const {
    return m_ViewportFlag.get();
}
void ModelInstance::internal_update_model_matrix() {
    auto* model = m_Parent.getComponent<ComponentModel>();
    if (model) {
        Engine::priv::ComponentModel_Functions::CalculateRadius(*model);
    }
    Math::setFinalModelMatrix(m_ModelMatrix, m_Position, m_Orientation, m_Scale);
}
Entity ModelInstance::parent() const {
    return m_Parent; 
}
void ModelInstance::setStage(RenderStage::Stage stage, ComponentModel& componentModel) {
    m_Stage = stage;
    componentModel.setStage(stage, m_Index);
}
void ModelInstance::show() {
    m_Visible = true; 
}
void ModelInstance::hide() {
    m_Visible = false; 
}
bool ModelInstance::visible() const {
    return m_Visible; 
}
bool ModelInstance::passedRenderCheck() const {
    return m_PassedRenderCheck; 
}
void ModelInstance::setPassedRenderCheck(bool passed) {
    m_PassedRenderCheck = passed;
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
    internal_update_model_matrix();
}
void ModelInstance::setOrientation(const glm::quat& orientation) {
    m_Orientation = orientation;
    internal_update_model_matrix();
}
void ModelInstance::setOrientation(float x, float y, float z) {
    Math::setRotation(m_Orientation, x, y, z);
    internal_update_model_matrix();
}
void ModelInstance::setScale(float scale) {
    m_Scale = glm::vec3(scale, scale, scale);
    internal_update_model_matrix();
}
void ModelInstance::setScale(float x, float y, float z){
    m_Scale = glm::vec3(x, y, z);
    internal_update_model_matrix();
}
void ModelInstance::translate(float x, float y, float z){
    m_Position += glm::vec3(x, y, z);
    internal_update_model_matrix();
}
void ModelInstance::rotate(float x, float y, float z){
    Math::rotate(m_Orientation, x, y, z);
    internal_update_model_matrix();
}
void ModelInstance::scale(float x, float y, float z){
    m_Scale += glm::vec3(x, y, z); 
    internal_update_model_matrix();
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
const Engine::color_vector_4& ModelInstance::color() const {
    return m_Color; 
}
const Engine::color_vector_4& ModelInstance::godRaysColor() const {
    return m_GodRaysColor; 
}
const glm::mat4& ModelInstance::modelMatrix() const {
    return m_ModelMatrix; 
}
const glm::vec3& ModelInstance::getScale() const {
    return m_Scale; 
}
const glm::vec3& ModelInstance::position() const {
    return m_Position; 
}
const glm::quat& ModelInstance::orientation() const {
    return m_Orientation; 
}
ShaderProgram* ModelInstance::shaderProgram() const {
    return m_ShaderProgram; 
}
Mesh* ModelInstance::mesh() const {
    return m_Mesh; 
}
Material* ModelInstance::material() const {
    return m_Material; 
}
RenderStage::Stage ModelInstance::stage() const {
    return m_Stage; 
}
void ModelInstance::setShaderProgram(Handle shaderProgramHandle, ComponentModel& componentModel) {
    ModelInstance::setShaderProgram(shaderProgramHandle.get<ShaderProgram>(), componentModel);
}
void ModelInstance::setShaderProgram(ShaderProgram* shaderProgram, ComponentModel& componentModel) {
    if (!shaderProgram) { 
        shaderProgram = ShaderProgram::Deferred;
    }
    componentModel.setModel(m_Mesh, m_Material, m_Index, shaderProgram, m_Stage);
}
void ModelInstance::setMesh(Handle meshHandle, ComponentModel& componentModel){
    ModelInstance::setMesh(meshHandle.get<Mesh>(), componentModel);
}
void ModelInstance::setMesh(Mesh* mesh, ComponentModel& componentModel){
    m_AnimationVector.clear();
    componentModel.setModel(mesh, m_Material, m_Index, m_ShaderProgram, m_Stage);
}
void ModelInstance::setMaterial(Handle materialHandle, ComponentModel& componentModel){
    ModelInstance::setMaterial(materialHandle.get<Material>(), componentModel);
}
void ModelInstance::setMaterial(Material* material, ComponentModel& componentModel){
    componentModel.setModel(m_Mesh, material, m_Index, m_ShaderProgram, m_Stage);
}
void ModelInstance::playAnimation(const string& animationName, float start, float end, unsigned int requestedLoops){
    m_AnimationVector.emplace_animation(*m_Mesh, animationName, start, end, requestedLoops);
}
