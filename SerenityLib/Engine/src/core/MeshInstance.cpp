#include <core/MeshInstance.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/resources/Engine_BuiltInResources.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/mesh/Skeleton.h>
#include <core/Material.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/scene/Scene.h>
#include <core/ShaderProgram.h>
#include <ecs/Components.h>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace Engine;
using namespace std;

namespace Engine {
    namespace epriv {
        struct MeshInstanceAnimation final{
            friend struct DefaultMeshInstanceBindFunctor;
            friend struct DefaultMeshInstanceUnbindFunctor;

            uint m_CurrentLoops, m_RequestedLoops;
            float m_CurrentTime, m_StartTime, m_EndTime;
            string m_AnimationName;
            Mesh* m_Mesh;

            MeshInstanceAnimation(Mesh& _mesh, const std::string& _animName, float _startTime, float _endTime, uint _requestedLoops = 1) {
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

        struct DefaultMeshInstanceBindFunctor {void operator()(EngineResource* r) const {
            MeshInstance& i = *(MeshInstance*)r;
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
                    MeshInstanceAnimation* anim = (*it);
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
        struct DefaultMeshInstanceUnbindFunctor {void operator()(EngineResource* r) const {

        }};


    };
};

MeshInstance::MeshInstance(Entity& parent, Mesh* mesh, Material* mat, ShaderP* program) {
    internalInit(mesh, mat, parent, program);
    setCustomBindFunctor(epriv::DefaultMeshInstanceBindFunctor());
    setCustomUnbindFunctor(epriv::DefaultMeshInstanceUnbindFunctor());
}
MeshInstance::MeshInstance(Entity& parent, Handle mesh, Handle mat, ShaderP* program) :MeshInstance(parent, (Mesh*)mesh.get(), (Material*)mat.get(), program) {
}
MeshInstance::MeshInstance(Entity& parent, Mesh* mesh, Handle mat, ShaderP* program) : MeshInstance(parent, mesh, (Material*)mat.get(), program) {
}
MeshInstance::MeshInstance(Entity& parent, Handle mesh, Material* mat, ShaderP* program) : MeshInstance(parent, (Mesh*)mesh.get(), mat, program) {
}
MeshInstance::~MeshInstance() {
    SAFE_DELETE_VECTOR(m_AnimationQueue);
}

void MeshInstance::internalInit(Mesh* mesh, Material* mat, Entity& parent, ShaderP* program) {
    if (!program) {
        program = epriv::InternalShaderPrograms::Deferred;
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
void MeshInstance::internalUpdateModelMatrix() {
    Math::setFinalModelMatrix(m_ModelMatrix, m_Position, m_Orientation, m_Scale);
}
void* MeshInstance::getUserPointer() const {
    return m_UserPointer; 
}
void MeshInstance::setUserPointer(void* t) {
    m_UserPointer = t; 
}
Entity& MeshInstance::parent() { 
    return m_Parent; 
}
void MeshInstance::setStage(const RenderStage::Stage& stage) {
    m_Stage = stage;
}
void MeshInstance::show() { 
    m_Visible = true; 
}
void MeshInstance::hide() { 
    m_Visible = false; 
}
const bool& MeshInstance::visible() const {
    return m_Visible; 
}
const bool& MeshInstance::passedRenderCheck() const {
    return m_PassedRenderCheck; 
}
void MeshInstance::setPassedRenderCheck(const bool& b) {
    m_PassedRenderCheck = b; 
}
void MeshInstance::setColor(const float& r, const float& g, const float& b, const float& a) {
    Math::setColor(m_Color, r, g, b, a);
}
void MeshInstance::setColor(const glm::vec4& color){
    MeshInstance::setColor(color.r, color.g, color.b, color.a);
}
void MeshInstance::setColor(const glm::vec3& color) {
    MeshInstance::setColor(color.r, color.g, color.b, 1.0f); 
}
void MeshInstance::setGodRaysColor(const float& r, const float& g, const float& b) {
    Math::setColor(m_GodRaysColor, r, g, b);
}
void MeshInstance::setGodRaysColor(const glm::vec3& color){
    MeshInstance::setGodRaysColor(color.r, color.g, color.b);
}
void MeshInstance::setPosition(const float& x, const float& y, const float& z){
    m_Position = glm::vec3(x, y, z);
    internalUpdateModelMatrix();
}
void MeshInstance::setOrientation(const glm::quat& orientation) {
    m_Orientation = orientation;
    internalUpdateModelMatrix();
}
void MeshInstance::setOrientation(const float& x, const float& y, const float& z) {
    Math::setRotation(m_Orientation, x, y, z);
    internalUpdateModelMatrix();
}
void MeshInstance::setScale(const float& x, const float& y, const float& z){
    m_Scale = glm::vec3(x, y, z);
    internalUpdateModelMatrix();
}
void MeshInstance::translate(const float& x, const float& y, const float& z){
    m_Position += glm::vec3(x, y, z);
    internalUpdateModelMatrix();
}
void MeshInstance::rotate(const float& x, const float& y, const float& z){
    Math::rotate(m_Orientation, x, y, z);
    internalUpdateModelMatrix();
}
void MeshInstance::scale(const float& x, const float& y, const float& z){
    m_Scale += glm::vec3(x, y, z); 
    internalUpdateModelMatrix();
}
void MeshInstance::setPosition(const glm::vec3& v){
    setPosition(v.x, v.y, v.z);
}
void MeshInstance::setScale(const glm::vec3& v){
    setScale(v.x, v.y, v.z);
}
void MeshInstance::translate(const glm::vec3& v){
    translate(v.x, v.y, v.z);
}
void MeshInstance::rotate(const glm::vec3& v){
    rotate(v.x, v.y, v.z);
}
void MeshInstance::scale(const glm::vec3& v) {
    scale(v.x, v.y, v.z);
}
const glm::vec4& MeshInstance::color() const {
    return m_Color; 
}
const glm::vec3& MeshInstance::godRaysColor() const {
    return m_GodRaysColor; 
}
const glm::mat4& MeshInstance::modelMatrix() const {
    return m_ModelMatrix; 
}
const glm::vec3& MeshInstance::getScale() const {
    return m_Scale; 
}
const glm::vec3& MeshInstance::position() const {
    return m_Position; 
}
const glm::quat& MeshInstance::orientation() const {
    return m_Orientation; 
}
ShaderP* MeshInstance::shaderProgram() {
    return m_ShaderProgram; 
}
Mesh* MeshInstance::mesh() {
    return m_Mesh; 
}
Material* MeshInstance::material() {
    return m_Material; 
}
const RenderStage::Stage& MeshInstance::stage() const {
    return m_Stage; 
}
void MeshInstance::setShaderProgram(const Handle& shaderPHandle, ComponentModel& componentModel) {
    setShaderProgram(((ShaderP*)shaderPHandle.get()), componentModel);
}
void MeshInstance::setShaderProgram(ShaderP* shaderProgram, ComponentModel& componentModel) {
    if (!shaderProgram) { 
        shaderProgram = epriv::InternalShaderPrograms::Deferred; 
    }
    componentModel.setModel(m_Mesh, m_Material, 0, shaderProgram, m_Stage);
}
void MeshInstance::setMesh(const Handle& meshHandle, ComponentModel& componentModel){
    setMesh(((Mesh*)meshHandle.get()), componentModel);
}
void MeshInstance::setMesh(Mesh* mesh, ComponentModel& componentModel){
    componentModel.setModel(mesh, m_Material, 0, m_ShaderProgram, m_Stage);
}
void MeshInstance::setMaterial(const Handle& materialHandle, ComponentModel& componentModel){
    setMaterial(((Material*)materialHandle.get()), componentModel);
}
void MeshInstance::setMaterial(Material* material, ComponentModel& componentModel){
    componentModel.setModel(m_Mesh, material, 0, m_ShaderProgram, m_Stage);
}
void MeshInstance::playAnimation(const string& animName, const float& start, const float& end, const uint& reqLoops){
    //m_AnimationQueue.emplace_back(*mesh(), animName, start, end, reqLoops);
    m_AnimationQueue.push_back(new epriv::MeshInstanceAnimation(*mesh(), animName, start, end, reqLoops));
}
