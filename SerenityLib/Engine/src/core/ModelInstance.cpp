#include <core/ModelInstance.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/resources/Engine_BuiltInResources.h>
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

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace Engine;
using namespace std;

unsigned int ModelInstance::m_ViewportFlagDefault = ViewportFlag::All;

namespace Engine {
    namespace epriv {
        struct ModelInstanceAnimation final{
            uint m_CurrentLoops, m_RequestedLoops;
            float m_CurrentTime, m_StartTime, m_EndTime;
            string m_AnimationName;
            Mesh* m_Mesh;

            ModelInstanceAnimation(Mesh& _mesh, const std::string& _animName, float _startTime, float _endTime, uint _requestedLoops = 1) {
                m_CurrentLoops = 0;
                m_RequestedLoops = _requestedLoops;
                m_CurrentTime = 0;
                m_StartTime = _startTime;
                m_AnimationName = _animName;
                m_Mesh = &_mesh;
                if (_endTime < 0) {
                    m_EndTime = _mesh.animationData().at(_animName).duration();
                }else{
                    m_EndTime = _endTime;
                }
            }
        };
        struct DefaultModelInstanceBindFunctor {void operator()(EngineResource* r) const {
            auto& i = *static_cast<ModelInstance*>(r);
            const auto& stage = i.stage();
            auto& scene = *Resources::getCurrentScene();
            Camera& cam = *scene.getActiveCamera();
            glm::vec3 camPos = cam.getPosition();
            Entity& parent = i.m_Parent;
            auto& body = *(parent.getComponent<ComponentBody>());
            glm::mat4 parentModel = body.modelMatrixRendering();

            auto& animationQueue = i.m_AnimationQueue;
            Renderer::sendUniform4Safe("Object_Color", i.m_Color);
            Renderer::sendUniform3Safe("Gods_Rays_Color", i.m_GodRaysColor);

            if (stage == RenderStage::ForwardTransparentTrianglesSorted || stage == RenderStage::ForwardTransparent || stage == RenderStage::ForwardOpaque) {
                auto& lights = epriv::InternalScenePublicInterface::GetLights(scene);
                int maxLights = glm::min(static_cast<int>(lights.size()), MAX_LIGHTS_PER_PASS);
                Renderer::sendUniform1Safe("numLights", maxLights);
                for (int i = 0; i < maxLights; ++i) {
                    auto& light = *lights[i];
                    const auto& lightType = light.type();
                    const auto start = "light[" + to_string(i) + "].";
                    switch (lightType) {
                        case LightType::Sun: {
                            SunLight& s = static_cast<SunLight&>(light);
                            auto& body = *s.getComponent<ComponentBody>();
                            const glm::vec3& pos = body.position();
                            Renderer::sendUniform4Safe((start + "DataA").c_str(), s.getAmbientIntensity(), s.getDiffuseIntensity(), s.getSpecularIntensity(), 0.0f);
                            Renderer::sendUniform4Safe((start + "DataC").c_str(), 0.0f, pos.x, pos.y, pos.z);
                            Renderer::sendUniform4Safe((start + "DataD").c_str(), s.color().x, s.color().y, s.color().z, static_cast<float>(lightType));
                            break;
                        }case LightType::Directional: {
                            DirectionalLight& d = static_cast<DirectionalLight&>(light);
                            auto& body = *d.getComponent<ComponentBody>();
                            const glm::vec3& _forward = body.forward();
                            Renderer::sendUniform4Safe((start + "DataA").c_str(), d.getAmbientIntensity(), d.getDiffuseIntensity(), d.getSpecularIntensity(), _forward.x);
                            Renderer::sendUniform4Safe((start + "DataB").c_str(), _forward.y, _forward.z, 0.0f, 0.0f);
                            Renderer::sendUniform4Safe((start + "DataD").c_str(), d.color().x, d.color().y, d.color().z, static_cast<float>(lightType));
                            break;
                        }case LightType::Point: {
                            PointLight& p = static_cast<PointLight&>(light);
                            auto& body = *p.getComponent<ComponentBody>();
                            const glm::vec3& pos = body.position();
                            Renderer::sendUniform4Safe((start + "DataA").c_str(), p.getAmbientIntensity(), p.getDiffuseIntensity(), p.getSpecularIntensity(), 0.0f);
                            Renderer::sendUniform4Safe((start + "DataB").c_str(), 0.0f, 0.0f, p.getConstant(), p.getLinear());
                            Renderer::sendUniform4Safe((start + "DataC").c_str(), p.getExponent(), pos.x, pos.y, pos.z);
                            Renderer::sendUniform4Safe((start + "DataD").c_str(), p.color().x, p.color().y, p.color().z, static_cast<float>(lightType));
                            Renderer::sendUniform4Safe((start + "DataE").c_str(), 0.0f, 0.0f, static_cast<float>(p.getAttenuationModel()), 0.0f);
                            break;
                        }case LightType::Spot: {
                            SpotLight& s = static_cast<SpotLight&>(light);
                            auto& body = *s.getComponent<ComponentBody>();
                            const glm::vec3& pos = body.position();
                            const glm::vec3 _forward = body.forward();
                            Renderer::sendUniform4Safe((start + "DataA").c_str(), s.getAmbientIntensity(), s.getDiffuseIntensity(), s.getSpecularIntensity(), _forward.x);
                            Renderer::sendUniform4Safe((start + "DataB").c_str(), _forward.y, _forward.z, s.getConstant(), s.getLinear());
                            Renderer::sendUniform4Safe((start + "DataC").c_str(), s.getExponent(), pos.x, pos.y, pos.z);
                            Renderer::sendUniform4Safe((start + "DataD").c_str(), s.color().x, s.color().y, s.color().z, static_cast<float>(lightType));
                            Renderer::sendUniform4Safe((start + "DataE").c_str(), s.getCutoff(), s.getCutoffOuter(), static_cast<float>(s.getAttenuationModel()), 0.0f);
                            break;
                        }case LightType::Rod: {
                            RodLight& r = static_cast<RodLight&>(light);
                            auto& body = *r.getComponent<ComponentBody>();
                            const glm::vec3& pos = body.position();
                            const float cullingDistance = r.rodLength() + (r.getCullingRadius() * 2.0f);
                            const float half = r.rodLength() / 2.0f;
                            const glm::vec3& firstEndPt = pos + (glm::vec3(body.forward()) * half);
                            const glm::vec3& secndEndPt = pos - (glm::vec3(body.forward()) * half);
                            Renderer::sendUniform4Safe((start + "DataA").c_str(), r.getAmbientIntensity(), r.getDiffuseIntensity(), r.getSpecularIntensity(), firstEndPt.x);
                            Renderer::sendUniform4Safe((start + "DataB").c_str(), firstEndPt.y, firstEndPt.z, r.getConstant(), r.getLinear());
                            Renderer::sendUniform4Safe((start + "DataC").c_str(), r.getExponent(), secndEndPt.x, secndEndPt.y, secndEndPt.z);
                            Renderer::sendUniform4Safe((start + "DataD").c_str(), r.color().x, r.color().y, r.color().z, static_cast<float>(lightType));
                            Renderer::sendUniform4Safe((start + "DataE").c_str(), r.rodLength(), 0.0f, static_cast<float>(r.getAttenuationModel()), 0.0f);
                            break;
                        }default: {
                            break;
                        }
                    }
                }
                Skybox* skybox = scene.skybox();
                Renderer::sendUniform4Safe("ScreenData", epriv::Core::m_Engine->m_RenderManager._getGIPackedData(), Renderer::Settings::getGamma(), 0.0f, 0.0f);
                auto maxTextures = epriv::Core::m_Engine->m_RenderManager.OpenGLStateMachine.getMaxTextureUnits() - 1;
                if (skybox && skybox->texture()->numAddresses() >= 3) {
                    Renderer::sendTextureSafe("irradianceMap", skybox->texture()->address(1), maxTextures - 2, GL_TEXTURE_CUBE_MAP);
                    Renderer::sendTextureSafe("prefilterMap", skybox->texture()->address(2), maxTextures - 1, GL_TEXTURE_CUBE_MAP);
                    Renderer::sendTextureSafe("brdfLUT", *Texture::BRDF, maxTextures);
                }else{
                    Renderer::sendTextureSafe("irradianceMap", Texture::Black->address(0), maxTextures - 2, GL_TEXTURE_2D);
                    Renderer::sendTextureSafe("prefilterMap", Texture::Black->address(0), maxTextures - 1, GL_TEXTURE_2D);
                    Renderer::sendTextureSafe("brdfLUT", *Texture::BRDF, maxTextures);
                }
            }
            if (animationQueue.size() > 0) {
                vector<glm::mat4> transforms;
                //process the animation here
                for (size_t j = 0; j < animationQueue.size(); ++j) {
                    auto& a = *(animationQueue[j]);
                    if (a.m_Mesh == i.m_Mesh) {
                        a.m_CurrentTime += (float)Resources::dt();
                        a.m_Mesh->playAnimation(transforms, a.m_AnimationName, a.m_CurrentTime);
                        if (a.m_CurrentTime >= a.m_EndTime) {
                            a.m_CurrentTime = 0;
                            ++a.m_CurrentLoops;
                        }
                    }
                }
                Renderer::sendUniform1Safe("AnimationPlaying", 1);
                Renderer::sendUniformMatrix4vSafe("gBones[0]", transforms, static_cast<uint>(transforms.size()));
                //cleanup the animation queue
                for (auto it = animationQueue.cbegin(); it != animationQueue.cend();) {
                    ModelInstanceAnimation* anim = (*it);
                    auto& a = *((*it));
                    if (a.m_RequestedLoops > 0 && (a.m_CurrentLoops >= a.m_RequestedLoops)) {
                        SAFE_DELETE(anim); //do we need this?
                        it = animationQueue.erase(it);
                    }
                    else { ++it; }
                }
            }else{
                Renderer::sendUniform1Safe("AnimationPlaying", 0);
            }
            glm::mat4 modelMatrix = parentModel * i.m_ModelMatrix;

            //world space normals
            glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));

            //view space normals
            //glm::mat4 view = cam.getView();
            //glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(view * model)));

            Renderer::sendUniformMatrix4Safe("Model", modelMatrix);
            Renderer::sendUniformMatrix3Safe("NormalMatrix", normalMatrix);
        }};
        struct DefaultModelInstanceUnbindFunctor {void operator()(EngineResource* r) const {
            //auto& i = *static_cast<ModelInstance*>(r);
        }};
    };
};
const bool epriv::InternalModelInstancePublicInterface::IsViewportValid(ModelInstance& modelInstance, Viewport& viewport) {
    const auto flags = modelInstance.getViewportFlags();
    return (!((flags & (1 << viewport.id())) || flags == 0)) ? false : true;
}



ModelInstance::ModelInstance(Entity& parent, Mesh* mesh, Material* mat, ShaderProgram* program):m_Parent(parent),BindableResource(ResourceType::Empty){
    internalInit(mesh, mat, program);
    setCustomBindFunctor(epriv::DefaultModelInstanceBindFunctor());
    setCustomUnbindFunctor(epriv::DefaultModelInstanceUnbindFunctor());
}
ModelInstance::ModelInstance(Entity& parent, Handle mesh, Handle mat, ShaderProgram* program) :ModelInstance(parent, (Mesh*)mesh.get(), (Material*)mat.get(), program) {
}
ModelInstance::ModelInstance(Entity& parent, Mesh* mesh, Handle mat, ShaderProgram* program) : ModelInstance(parent, mesh, (Material*)mat.get(), program) {
}
ModelInstance::ModelInstance(Entity& parent, Handle mesh, Material* mat, ShaderProgram* program) : ModelInstance(parent, (Mesh*)mesh.get(), mat, program) {
}
ModelInstance::~ModelInstance() {
    SAFE_DELETE_VECTOR(m_AnimationQueue);
}

void ModelInstance::setDefaultViewportFlag(const unsigned int flag) {
    m_ViewportFlagDefault = flag;
}
void ModelInstance::setDefaultViewportFlag(const ViewportFlag::Flag flag) {
    m_ViewportFlagDefault = static_cast<unsigned int>(flag);
}

void ModelInstance::internalInit(Mesh* mesh, Material* mat, ShaderProgram* program) {
    if (!program) {
        program = ShaderProgram::Deferred;
    }
    m_DrawingMode       = ModelDrawingMode::Triangles;
    m_ViewportFlag      = ModelInstance::m_ViewportFlagDefault;
    m_UserPointer       = nullptr;
    m_Stage             = RenderStage::GeometryOpaque;
    m_PassedRenderCheck = false;
    m_ForceRender       = false;
    m_Visible           = true;
    m_ShaderProgram     = program;
    m_Material          = mat;
    m_Mesh              = mesh;
    m_Color             = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    m_GodRaysColor      = glm::vec3(0.0f, 0.0f, 0.0f);
    m_Position          = glm::vec3(0.0f, 0.0f, 0.0f);
    m_Orientation       = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    m_Scale             = glm::vec3(1.0f, 1.0f, 1.0f);
    m_Index             = 0;

    internalUpdateModelMatrix();
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
    m_ViewportFlag = m_ViewportFlag | flag;
}
void ModelInstance::setViewportFlag(const ViewportFlag::Flag flag) {
    m_ViewportFlag = static_cast<unsigned int>(flag);
}
void ModelInstance::addViewportFlag(const ViewportFlag::Flag flag) {
    m_ViewportFlag = m_ViewportFlag | static_cast<unsigned int>(flag);
}
const unsigned int ModelInstance::getViewportFlags() const {
    return m_ViewportFlag;
}
void ModelInstance::internalUpdateModelMatrix() {
    auto* model = m_Parent.getComponent<ComponentModel>();
    if(model)
        Engine::epriv::ComponentModel_Functions::CalculateRadius(*model);
    Math::setFinalModelMatrix(m_ModelMatrix, m_Position, m_Orientation, m_Scale);
}
void* ModelInstance::getUserPointer() const {
    return m_UserPointer; 
}
void ModelInstance::setUserPointer(void* t) {
    m_UserPointer = t; 
}
Entity& ModelInstance::parent() {
    return m_Parent; 
}
void ModelInstance::setStage(const RenderStage::Stage& stage) {
    m_Stage = stage;
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
    Math::setColor(m_Color, r, g, b, a);
}
void ModelInstance::setColor(const glm::vec4& color){
    setColor(color.r, color.g, color.b, color.a);
}
void ModelInstance::setColor(const glm::vec3& color) {
    setColor(color.r, color.g, color.b, 1.0f); 
}
void ModelInstance::setGodRaysColor(const float& r, const float& g, const float& b) {
    Math::setColor(m_GodRaysColor, r, g, b);
}
void ModelInstance::setGodRaysColor(const glm::vec3& color){
    setGodRaysColor(color.r, color.g, color.b);
}
void ModelInstance::setPosition(const float& x, const float& y, const float& z){
    m_Position = glm::vec3(x, y, z);
    internalUpdateModelMatrix();
}
void ModelInstance::setOrientation(const glm::quat& orientation) {
    m_Orientation = orientation;
    internalUpdateModelMatrix();
}
void ModelInstance::setOrientation(const float& x, const float& y, const float& z) {
    Math::setRotation(m_Orientation, x, y, z);
    internalUpdateModelMatrix();
}
void ModelInstance::setScale(const float& scale) {
    m_Scale = glm::vec3(scale, scale, scale);
    internalUpdateModelMatrix();
}
void ModelInstance::setScale(const float& x, const float& y, const float& z){
    m_Scale = glm::vec3(x, y, z);
    internalUpdateModelMatrix();
}
void ModelInstance::translate(const float& x, const float& y, const float& z){
    m_Position += glm::vec3(x, y, z);
    internalUpdateModelMatrix();
}
void ModelInstance::rotate(const float& x, const float& y, const float& z){
    Math::rotate(m_Orientation, x, y, z);
    internalUpdateModelMatrix();
}
void ModelInstance::scale(const float& x, const float& y, const float& z){
    m_Scale += glm::vec3(x, y, z); 
    internalUpdateModelMatrix();
}
void ModelInstance::setPosition(const glm::vec3& v){
    setPosition(v.x, v.y, v.z);
}
void ModelInstance::setScale(const glm::vec3& v){
    setScale(v.x, v.y, v.z);
}
void ModelInstance::translate(const glm::vec3& v){
    translate(v.x, v.y, v.z);
}
void ModelInstance::rotate(const glm::vec3& v){
    rotate(v.x, v.y, v.z);
}
void ModelInstance::scale(const glm::vec3& v) {
    scale(v.x, v.y, v.z);
}
const glm::vec4& ModelInstance::color() const {
    return m_Color; 
}
const glm::vec3& ModelInstance::godRaysColor() const {
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
ShaderProgram* ModelInstance::shaderProgram() {
    return m_ShaderProgram; 
}
Mesh* ModelInstance::mesh() {
    return m_Mesh; 
}
Material* ModelInstance::material() {
    return m_Material; 
}
const RenderStage::Stage& ModelInstance::stage() const {
    return m_Stage; 
}
void ModelInstance::setShaderProgram(const Handle& shaderPHandle, ComponentModel& componentModel) {
    setShaderProgram(((ShaderProgram*)shaderPHandle.get()), componentModel);
}
void ModelInstance::setShaderProgram(ShaderProgram* shaderProgram, ComponentModel& componentModel) {
    if (!shaderProgram) { 
        shaderProgram = ShaderProgram::Deferred;
    }
    componentModel.setModel(m_Mesh, m_Material, 0, shaderProgram, m_Stage);
}
void ModelInstance::setMesh(const Handle& meshHandle, ComponentModel& componentModel){
    setMesh(((Mesh*)meshHandle.get()), componentModel);
}
void ModelInstance::setMesh(Mesh* mesh, ComponentModel& componentModel){
    componentModel.setModel(mesh, m_Material, 0, m_ShaderProgram, m_Stage);
}
void ModelInstance::setMaterial(const Handle& materialHandle, ComponentModel& componentModel){
    setMaterial(((Material*)materialHandle.get()), componentModel);
}
void ModelInstance::setMaterial(Material* material, ComponentModel& componentModel){
    componentModel.setModel(m_Mesh, material, 0, m_ShaderProgram, m_Stage);
}
void ModelInstance::playAnimation(const string& animName, const float& start, const float& end, const uint& reqLoops){
    m_AnimationQueue.push_back(new epriv::ModelInstanceAnimation(*mesh(), animName, start, end, reqLoops));
}
