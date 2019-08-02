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
#include <core/engine/shaders/ShaderProgram.h>
#include <ecs/Components.h>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace Engine;
using namespace std;

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
            ModelInstance& i = *(ModelInstance*)r;
            Camera& cam = *Resources::getCurrentScene()->getActiveCamera();
            glm::vec3 camPos = cam.getPosition();
            Entity& parent = i.m_Parent;
            auto& body = *(parent.getComponent<ComponentBody>());
            glm::mat4 parentModel = body.modelMatrix();

            auto& animationQueue = i.m_AnimationQueue;
            Renderer::sendUniform4Safe("Object_Color", i.m_Color);
            Renderer::sendUniform3Safe("Gods_Rays_Color", i.m_GodRaysColor);
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
                Renderer::sendUniformMatrix4vSafe("gBones[0]", transforms, transforms.size());

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
            }
            else {
                Renderer::sendUniform1Safe("AnimationPlaying", 0);
            }
            glm::mat4 modelMatrix = parentModel * i.m_ModelMatrix; //might need to reverse this order.

            //world space normals
            glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));

            //view space normals
            //glm::mat4 view = cam.getView();
            //glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(view * model)));

            Renderer::sendUniformMatrix4Safe("Model", modelMatrix);
            Renderer::sendUniformMatrix3Safe("NormalMatrix", normalMatrix);
        }};
        struct DefaultModelInstanceUnbindFunctor {void operator()(EngineResource* r) const {

        }};


    };
};

ModelInstance::ModelInstance(Entity& parent, Mesh* mesh, Material* mat, ShaderProgram* program) {
    internalInit(mesh, mat, parent, program);
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

void ModelInstance::internalInit(Mesh* mesh, Material* mat, Entity& parent, ShaderProgram* program) {
    if (!program) {
        program = ShaderProgram::Deferred;
    }
    m_UserPointer       = nullptr;
    m_Stage             = RenderStage::GeometryOpaque;
    m_PassedRenderCheck = false;
    m_Visible           = true;
    m_Parent            = parent;
    m_ShaderProgram     = program;
    m_Material          = mat;
    m_Mesh              = mesh;
    m_Color             = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    m_GodRaysColor      = glm::vec3(0.0f, 0.0f, 0.0f);
    m_Position          = glm::vec3(0.0f, 0.0f, 0.0f);
    m_Orientation       = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    m_Scale             = glm::vec3(1.0f, 1.0f, 1.0f);

    internalUpdateModelMatrix();
}
void ModelInstance::internalUpdateModelMatrix() {
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
