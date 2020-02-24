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

#include <core/engine/lights/SunLight.h>
#include <core/engine/lights/DirectionalLight.h>
#include <core/engine/lights/PointLight.h>
#include <core/engine/lights/SpotLight.h>
#include <core/engine/lights/RodLight.h>
#include <core/engine/scene/Skybox.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/renderer/pipelines/IRenderingPipeline.h>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace Engine;
using namespace std;


unsigned int ModelInstance::m_ViewportFlagDefault = ViewportFlag::All;

namespace Engine::priv {
    struct DefaultModelInstanceBindFunctor {void operator()(ModelInstance* i, const Engine::priv::Renderer* renderer) const {
        const auto& stage             = i->stage();
        auto& scene                   = *Resources::getCurrentScene();
        const glm::vec3 camPos        = scene.getActiveCamera()->getPosition();
        auto& body                    = *(i->m_Parent.getComponent<ComponentBody>());
        const glm::mat4 parentModel   = body.modelMatrixRendering();
        auto& animationVector         = i->m_AnimationVector;

        Engine::Renderer::sendUniform1Safe("Object_Color", i->m_Color.toPackedInt());
        Engine::Renderer::sendUniform1Safe("Gods_Rays_Color", i->m_GodRaysColor.toPackedInt());

        if (stage == RenderStage::ForwardTransparentTrianglesSorted || stage == RenderStage::ForwardTransparent || stage == RenderStage::ForwardOpaque) {
            auto& lights     = priv::InternalScenePublicInterface::GetLights(scene);
            int maxLights    = glm::min(static_cast<int>(lights.size()), MAX_LIGHTS_PER_PASS);
            Engine::Renderer::sendUniform1Safe("numLights", maxLights);
            for (int i = 0; i < maxLights; ++i) {
                auto& light            = *lights[i];
                const auto& lightType  = light.type();
                const auto start       = "light[" + to_string(i) + "].";
                switch (lightType) {
                    case LightType::Sun: {
                        SunLight& s          = static_cast<SunLight&>(light);
                        auto& body           = *s.getComponent<ComponentBody>();
                        const glm::vec3 pos  = body.position();
                        Engine::Renderer::sendUniform4Safe((start + "DataA").c_str(), s.getAmbientIntensity(), s.getDiffuseIntensity(), s.getSpecularIntensity(), 0.0f);
                        Engine::Renderer::sendUniform4Safe((start + "DataC").c_str(), 0.0f, pos.x, pos.y, pos.z);
                        Engine::Renderer::sendUniform4Safe((start + "DataD").c_str(), s.color().x, s.color().y, s.color().z, static_cast<float>(lightType));
                        break;
                    }case LightType::Directional: {
                        DirectionalLight& d       = static_cast<DirectionalLight&>(light);
                        auto& body                = *d.getComponent<ComponentBody>();
                        const glm::vec3& _forward = body.forward();
                        Engine::Renderer::sendUniform4Safe((start + "DataA").c_str(), d.getAmbientIntensity(), d.getDiffuseIntensity(), d.getSpecularIntensity(), _forward.x);
                        Engine::Renderer::sendUniform4Safe((start + "DataB").c_str(), _forward.y, _forward.z, 0.0f, 0.0f);
                        Engine::Renderer::sendUniform4Safe((start + "DataD").c_str(), d.color().x, d.color().y, d.color().z, static_cast<float>(lightType));
                        break;
                    }case LightType::Point: {
                        PointLight& p       = static_cast<PointLight&>(light);
                        auto& body          = *p.getComponent<ComponentBody>();
                        const glm::vec3 pos = body.position();
                        Engine::Renderer::sendUniform4Safe((start + "DataA").c_str(), p.getAmbientIntensity(), p.getDiffuseIntensity(), p.getSpecularIntensity(), 0.0f);
                        Engine::Renderer::sendUniform4Safe((start + "DataB").c_str(), 0.0f, 0.0f, p.getConstant(), p.getLinear());
                        Engine::Renderer::sendUniform4Safe((start + "DataC").c_str(), p.getExponent(), pos.x, pos.y, pos.z);
                        Engine::Renderer::sendUniform4Safe((start + "DataD").c_str(), p.color().x, p.color().y, p.color().z, static_cast<float>(lightType));
                        Engine::Renderer::sendUniform4Safe((start + "DataE").c_str(), 0.0f, 0.0f, static_cast<float>(p.getAttenuationModel()), 0.0f);
                        break;
                    }case LightType::Spot: {
                        SpotLight& s                = static_cast<SpotLight&>(light);
                        auto& body                  = *s.getComponent<ComponentBody>();
                        const glm::vec3 pos         = body.position();
                        const glm::vec3& _forward   = body.forward();
                        Engine::Renderer::sendUniform4Safe((start + "DataA").c_str(), s.getAmbientIntensity(), s.getDiffuseIntensity(), s.getSpecularIntensity(), _forward.x);
                        Engine::Renderer::sendUniform4Safe((start + "DataB").c_str(), _forward.y, _forward.z, s.getConstant(), s.getLinear());
                        Engine::Renderer::sendUniform4Safe((start + "DataC").c_str(), s.getExponent(), pos.x, pos.y, pos.z);
                        Engine::Renderer::sendUniform4Safe((start + "DataD").c_str(), s.color().x, s.color().y, s.color().z, static_cast<float>(lightType));
                        Engine::Renderer::sendUniform4Safe((start + "DataE").c_str(), s.getCutoff(), s.getCutoffOuter(), static_cast<float>(s.getAttenuationModel()), 0.0f);
                        break;
                    }case LightType::Rod: {
                        RodLight& r                   = static_cast<RodLight&>(light);
                        auto& body                    = *r.getComponent<ComponentBody>();
                        const glm::vec3& pos          = body.position();
                        const float cullingDistance   = r.rodLength() + (r.getCullingRadius() * 2.0f);
                        const float half              = r.rodLength() / 2.0f;
                        const glm::vec3 firstEndPt    = pos + (glm::vec3(body.forward()) * half);
                        const glm::vec3 secndEndPt    = pos - (glm::vec3(body.forward()) * half);
                        Engine::Renderer::sendUniform4Safe((start + "DataA").c_str(), r.getAmbientIntensity(), r.getDiffuseIntensity(), r.getSpecularIntensity(), firstEndPt.x);
                        Engine::Renderer::sendUniform4Safe((start + "DataB").c_str(), firstEndPt.y, firstEndPt.z, r.getConstant(), r.getLinear());
                        Engine::Renderer::sendUniform4Safe((start + "DataC").c_str(), r.getExponent(), secndEndPt.x, secndEndPt.y, secndEndPt.z);
                        Engine::Renderer::sendUniform4Safe((start + "DataD").c_str(), r.color().x, r.color().y, r.color().z, static_cast<float>(lightType));
                        Engine::Renderer::sendUniform4Safe((start + "DataE").c_str(), r.rodLength(), 0.0f, static_cast<float>(r.getAttenuationModel()), 0.0f);
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
            Engine::Renderer::sendUniformMatrix4vSafe("gBones[0]", animationVector.m_Transforms, static_cast<unsigned int>(animationVector.m_Transforms.size()));
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
    return !(flags & (1 << viewport.id()) || flags == 0) ? false : true;
}



ModelInstance::ModelInstance(Entity& parent, Mesh* mesh, Material* mat, ShaderProgram* program) : m_Parent(parent){
    internal_init(mesh, mat, program);
    setCustomBindFunctor(priv::DefaultModelInstanceBindFunctor());
    setCustomUnbindFunctor(priv::DefaultModelInstanceUnbindFunctor());
}
ModelInstance::ModelInstance(Entity& parent, Handle mesh, Handle mat, ShaderProgram* program) : ModelInstance(parent, (Mesh*)mesh.get(), (Material*)mat.get(), program) {
}
ModelInstance::ModelInstance(Entity& parent, Mesh* mesh, Handle mat, ShaderProgram* program) : ModelInstance(parent, mesh, (Material*)mat.get(), program) {
}
ModelInstance::ModelInstance(Entity& parent, Handle mesh, Material* mat, ShaderProgram* program) : ModelInstance(parent, (Mesh*)mesh.get(), mat, program) {
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

void ModelInstance::bind(const Engine::priv::Renderer& renderer) {
    m_CustomBindFunctor(this, &renderer);
}
void ModelInstance::unbind(const Engine::priv::Renderer& renderer) {
    m_CustomUnbindFunctor(this, &renderer);
}

void ModelInstance::setDefaultViewportFlag(const unsigned int flag) {
    m_ViewportFlagDefault = flag;
}
void ModelInstance::setDefaultViewportFlag(const ViewportFlag::Flag flag) {
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
const size_t& ModelInstance::index() const {
    return m_Index;
}
const ModelDrawingMode::Mode& ModelInstance::getDrawingMode() const {
    return m_DrawingMode;
}
void ModelInstance::setDrawingMode(const ModelDrawingMode::Mode& drawMode) {
    m_DrawingMode = drawMode;
}

void ModelInstance::forceRender(const bool forced) {
    m_ForceRender = forced;
}
const bool ModelInstance::isForceRendered() const {
    return m_ForceRender;
}
void ModelInstance::setViewportFlag(const unsigned int flag) {
    m_ViewportFlag = flag;
}
void ModelInstance::addViewportFlag(const unsigned int flag) {
    m_ViewportFlag.add(flag);
}
void ModelInstance::removeViewportFlag(const unsigned int flag) {
    m_ViewportFlag.remove(flag);
}
void ModelInstance::setViewportFlag(const ViewportFlag::Flag flag) {
    m_ViewportFlag = flag;
}
void ModelInstance::addViewportFlag(const ViewportFlag::Flag flag) {
    m_ViewportFlag.add(flag);
}
void ModelInstance::removeViewportFlag(const ViewportFlag::Flag flag) {
    m_ViewportFlag.remove(flag);
}
const unsigned int& ModelInstance::getViewportFlags() const {
    return m_ViewportFlag.get();
}
void ModelInstance::internal_update_model_matrix() {
    auto* model = m_Parent.getComponent<ComponentModel>();
    if (model) {
        Engine::priv::ComponentModel_Functions::CalculateRadius(*model);
    }
    Math::setFinalModelMatrix(m_ModelMatrix, m_Position, m_Orientation, m_Scale);
}
void* ModelInstance::getUserPointer() const {
    return m_UserPointer; 
}
void ModelInstance::setUserPointer(void* UserPointer) {
    m_UserPointer = UserPointer;
}
const Entity& ModelInstance::parent() const {
    return m_Parent; 
}
//void ModelInstance::setStage(const RenderStage::Stage& stage) {
//    m_Stage = stage;
//}
void ModelInstance::setStage(const RenderStage::Stage& stage, ComponentModel& componentModel) {
    m_Stage = stage;
    componentModel.setStage(stage, m_Index);
}
void ModelInstance::show() {
    m_Visible = true; 
}
void ModelInstance::hide() {
    m_Visible = false; 
}
const bool& ModelInstance::visible() const {
    return m_Visible; 
}
const bool& ModelInstance::passedRenderCheck() const {
    return m_PassedRenderCheck; 
}
void ModelInstance::setPassedRenderCheck(const bool& b) {
    m_PassedRenderCheck = b; 
}
void ModelInstance::setColor(const float& r, const float& g, const float& b, const float& a) {
    m_Color = Engine::color_vector_4(r, g, b, a);
}
void ModelInstance::setColor(const unsigned char& r, const unsigned char& g, const unsigned char& b, const unsigned char& a) {
    m_Color = Engine::color_vector_4(r, g, b, a);
}
void ModelInstance::setColor(const glm::vec4& color){
    ModelInstance::setColor(color.r, color.g, color.b, color.a);
}
void ModelInstance::setColor(const glm::vec3& color) {
    ModelInstance::setColor(color.r, color.g, color.b, 1.0f);
}
void ModelInstance::setGodRaysColor(const float& r, const float& g, const float& b) {
    m_GodRaysColor = Engine::color_vector_4(r, g, b, m_GodRaysColor.a());
}
void ModelInstance::setGodRaysColor(const glm::vec3& color){
    ModelInstance::setGodRaysColor(color.r, color.g, color.b);
}
void ModelInstance::setPosition(const float& x, const float& y, const float& z){
    m_Position = glm::vec3(x, y, z);
    internal_update_model_matrix();
}
void ModelInstance::setOrientation(const glm::quat& orientation) {
    m_Orientation = orientation;
    internal_update_model_matrix();
}
void ModelInstance::setOrientation(const float& x, const float& y, const float& z) {
    Math::setRotation(m_Orientation, x, y, z);
    internal_update_model_matrix();
}
void ModelInstance::setScale(const float& scale) {
    m_Scale = glm::vec3(scale, scale, scale);
    internal_update_model_matrix();
}
void ModelInstance::setScale(const float& x, const float& y, const float& z){
    m_Scale = glm::vec3(x, y, z);
    internal_update_model_matrix();
}
void ModelInstance::translate(const float& x, const float& y, const float& z){
    m_Position += glm::vec3(x, y, z);
    internal_update_model_matrix();
}
void ModelInstance::rotate(const float& x, const float& y, const float& z){
    Math::rotate(m_Orientation, x, y, z);
    internal_update_model_matrix();
}
void ModelInstance::scale(const float& x, const float& y, const float& z){
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
const RenderStage::Stage& ModelInstance::stage() const {
    return m_Stage; 
}
void ModelInstance::setShaderProgram(const Handle& shaderProgramHandle, ComponentModel& componentModel) {
    ModelInstance::setShaderProgram(((ShaderProgram*)shaderProgramHandle.get()), componentModel);
}
void ModelInstance::setShaderProgram(ShaderProgram* shaderProgram, ComponentModel& componentModel) {
    if (!shaderProgram) { 
        shaderProgram = ShaderProgram::Deferred;
    }
    componentModel.setModel(m_Mesh, m_Material, m_Index, shaderProgram, m_Stage);
}
void ModelInstance::setMesh(const Handle& meshHandle, ComponentModel& componentModel){
    ModelInstance::setMesh(((Mesh*)meshHandle.get()), componentModel);
}
void ModelInstance::setMesh(Mesh* mesh, ComponentModel& componentModel){
    m_AnimationVector.clear();
    componentModel.setModel(mesh, m_Material, m_Index, m_ShaderProgram, m_Stage);
}
void ModelInstance::setMaterial(const Handle& materialHandle, ComponentModel& componentModel){
    ModelInstance::setMaterial(((Material*)materialHandle.get()), componentModel);
}
void ModelInstance::setMaterial(Material* material, ComponentModel& componentModel){
    componentModel.setModel(m_Mesh, material, m_Index, m_ShaderProgram, m_Stage);
}
void ModelInstance::playAnimation(const string& animationName, const float& start, const float& end, const unsigned int& requestedLoops){
    m_AnimationVector.emplace_animation(*m_Mesh, animationName, start, end, requestedLoops);
}
