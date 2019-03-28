#include <core/MeshInstance.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/resources/Engine_BuiltInResources.h>
#include <core/engine/Engine_Math.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/mesh/Skeleton.h>
#include <core/Material.h>
#include <core/Camera.h>
#include <core/Scene.h>
#include <core/ShaderProgram.h>
#include <ecs/Components.h>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace Engine;
using namespace std;

namespace Engine {
    namespace epriv {
        struct MeshInstanceAnimation final{
            friend struct Engine::epriv::DefaultMeshInstanceBindFunctor;
            friend struct Engine::epriv::DefaultMeshInstanceUnbindFunctor;

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
    };
};


void epriv::DefaultMeshInstanceBindFunctor::operator()(EngineResource* r) const {
    MeshInstance& i = *(MeshInstance*)r;
    glm::vec3 camPos = Resources::getCurrentScene()->getActiveCamera()->getPosition();
    Entity& parent = i.m_Parent;
    auto& body = *(parent.getComponent<ComponentBody>());
    glm::mat4 parentModel = body.modelMatrix();

    auto& animationQueue = i.m_AnimationQueue;
    Renderer::sendUniform4Safe("Object_Color",i.m_Color);
    Renderer::sendUniform3Safe("Gods_Rays_Color",i.m_GodRaysColor);
    if(animationQueue.size() > 0){
        vector<glm::mat4> transforms;
        //process the animation here
        for(size_t j = 0; j < animationQueue.size(); ++j){
            auto& a = *(animationQueue[j]);
            if(a.m_Mesh == i.m_Mesh){
                a.m_CurrentTime += (float)Resources::dt();
                a.m_Mesh->playAnimation(transforms,a.m_AnimationName,a.m_CurrentTime);
                if(a.m_CurrentTime >= a.m_EndTime){
                    a.m_CurrentTime = 0;
                    ++a.m_CurrentLoops;
                }
            }
        }
        Renderer::sendUniform1Safe("AnimationPlaying",1);
        Renderer::sendUniformMatrix4vSafe("gBones[0]",transforms,transforms.size());

        //cleanup the animation queue
        for (auto it = animationQueue.cbegin(); it != animationQueue.cend();){
            MeshInstanceAnimation* anim = (*it);
            auto& a = *((*it));
            if (a.m_RequestedLoops > 0 && (a.m_CurrentLoops >= a.m_RequestedLoops)){
                SAFE_DELETE(anim); //do we need this?
                it = animationQueue.erase(it);
            }
            else{ ++it; }
        }
    }else{
        Renderer::sendUniform1Safe("AnimationPlaying",0);
    }
    glm::mat4 model = parentModel * i.m_Model; //might need to reverse this order.

    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
     
    Renderer::sendUniformMatrix4Safe("Model",model);
    Renderer::sendUniformMatrix3Safe("NormalMatrix",normalMatrix);
};
void epriv::DefaultMeshInstanceUnbindFunctor::operator()(EngineResource* r) const {
};

void MeshInstance::_init(Mesh* mesh, Material* mat, Entity& parent, ShaderP* program) {
    m_UserPointer = nullptr;
    m_Stage = RenderStage::GeometryOpaque;
    m_PassedRenderCheck = false;
    m_Visible = true;
    m_Parent = parent;

    if (!program) { program = epriv::InternalShaderPrograms::Deferred; }
    m_ShaderProgram = program;

    m_Material = mat;

    m_Mesh = mesh;

    m_Color = glm::vec4(1.0f,1.0f,1.0f,1.0f);
    m_GodRaysColor = glm::vec3(0.0f,0.0f,0.0f);
    m_Position = glm::vec3(0.0f,0.0f,0.0f);
    m_Orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    m_Scale = glm::vec3(1.0f,1.0f,1.0f);
    _updateModelMatrix();
}
void MeshInstance::_updateModelMatrix() {
    m_Model = glm::mat4(1.0f);
    glm::mat4 translationMatrix = glm::translate(m_Position);
    glm::mat4 rotationMatrix = glm::mat4_cast(m_Orientation);
    glm::mat4 scaleMatrix = glm::scale(m_Scale);
    m_Model = translationMatrix * rotationMatrix * scaleMatrix * m_Model;
}

MeshInstance::MeshInstance(Entity& parent, Mesh* mesh, Material* mat, ShaderP* program){
    _init(mesh, mat, parent, program);
    setCustomBindFunctor(epriv::DefaultMeshInstanceBindFunctor());
    setCustomUnbindFunctor(epriv::DefaultMeshInstanceUnbindFunctor());
}
MeshInstance::MeshInstance(Entity& parent, Handle mesh, Handle mat, ShaderP* program):MeshInstance(parent,(Mesh*)mesh.get(), (Material*)mat.get(),program){}
MeshInstance::MeshInstance(Entity& parent, Mesh* mesh, Handle mat, ShaderP* program):MeshInstance(parent, mesh, (Material*)mat.get(), program){}
MeshInstance::MeshInstance(Entity& parent, Handle mesh, Material* mat, ShaderP* program):MeshInstance(parent, (Mesh*)mesh.get(), mat, program) {}



MeshInstance::~MeshInstance(){ 
    SAFE_DELETE_VECTOR(m_AnimationQueue);
}
void MeshInstance::setStage(RenderStage::Stage s) { m_Stage = s; }
void MeshInstance::show() { m_Visible = true; }
void MeshInstance::hide() { m_Visible = false; }
bool MeshInstance::visible() { return m_Visible; }
bool MeshInstance::passedRenderCheck(){ return m_PassedRenderCheck; }
void MeshInstance::setPassedRenderCheck(bool b) { m_PassedRenderCheck = b; }
void MeshInstance::setColor(float r, float g, float b, float a) { m_Color = glm::vec4(r, g, b, a); }
void MeshInstance::setColor(glm::vec4& color){ MeshInstance::setColor(color.r,color.g,color.b,color.a); }
void MeshInstance::setColor(glm::vec3& color) { MeshInstance::setColor(color.r, color.g, color.b, 1.0f); }
void MeshInstance::setGodRaysColor(float r, float g, float b) { m_GodRaysColor.r = r; m_GodRaysColor.g = g; m_GodRaysColor.b = b; }
void MeshInstance::setGodRaysColor(glm::vec3& color){ MeshInstance::setGodRaysColor(color.r,color.g,color.b); }
void MeshInstance::setPosition(float x, float y, float z){ m_Position = glm::vec3(x, y, z); _updateModelMatrix(); }
void MeshInstance::setScale(float x,float y,float z){ m_Scale = glm::vec3(x, y, z); _updateModelMatrix(); }
void MeshInstance::translate(float x, float y, float z){ m_Position += glm::vec3(x, y, z); _updateModelMatrix(); }
void MeshInstance::rotate(float x, float y, float z){ 
    float threshold = 0;
    if (abs(x) > threshold) m_Orientation = m_Orientation * (glm::angleAxis(-x, glm::vec3(1, 0, 0)));   //pitch
    if (abs(y) > threshold) m_Orientation = m_Orientation * (glm::angleAxis(-y, glm::vec3(0, 1, 0)));   //yaw
    if (abs(z) > threshold) m_Orientation = m_Orientation * (glm::angleAxis(z, glm::vec3(0, 0, 1)));   //roll
    _updateModelMatrix();
}
void MeshInstance::scale(float x,float y,float z){ m_Scale += glm::vec3(x, y, z); _updateModelMatrix(); }
void MeshInstance::setPosition(glm::vec3& v){ setPosition(v.x,v.y,v.z); }
void MeshInstance::setScale(glm::vec3& v){ setScale(v.x,v.y,v.z); }
void MeshInstance::translate(glm::vec3& v){ translate(v.x,v.y,v.z); }
void MeshInstance::rotate(glm::vec3& v){ rotate(v.x,v.y,v.z); }
void MeshInstance::scale(glm::vec3& v){ scale(v.x,v.y,v.z); }
glm::vec4& MeshInstance::color(){ return m_Color; }
glm::vec3& MeshInstance::godRaysColor(){ return m_GodRaysColor; }
glm::mat4& MeshInstance::model(){ return m_Model; }
glm::vec3& MeshInstance::getScale(){ return m_Scale; }
glm::vec3& MeshInstance::position(){ return m_Position; }
glm::quat& MeshInstance::orientation(){ return m_Orientation; }
ShaderP* MeshInstance::shaderProgram() { return m_ShaderProgram; }
Mesh* MeshInstance::mesh(){ return m_Mesh; }
Material* MeshInstance::material(){ return m_Material; }
RenderStage::Stage MeshInstance::stage() { return m_Stage; }
void MeshInstance::setShaderProgram(const Handle& shaderPHandle, ComponentModel& model) { setShaderProgram(((ShaderP*)shaderPHandle.get()), model); }
void MeshInstance::setShaderProgram(ShaderP* shaderProgram, ComponentModel& model) {
    if (!shaderProgram) { shaderProgram = epriv::InternalShaderPrograms::Deferred; }
    model.setModel(m_Mesh, m_Material, 0, shaderProgram, m_Stage);
}
void MeshInstance::setMesh(const Handle& meshHandle, ComponentModel& model){ setMesh(((Mesh*)meshHandle.get()), model); }
void MeshInstance::setMesh(Mesh* mesh, ComponentModel& model){
    model.setModel(mesh, m_Material, 0, m_ShaderProgram, m_Stage);
}
void MeshInstance::setMaterial(const Handle& materialHandle, ComponentModel& model){ setMaterial(((Material*)materialHandle.get()), model); }
void MeshInstance::setMaterial(Material* material, ComponentModel& model){
    model.setModel(m_Mesh, material, 0, m_ShaderProgram, m_Stage);
}
void MeshInstance::setOrientation(glm::quat& o){ m_Orientation = o; }
void MeshInstance::setOrientation(float x,float y,float z){ 
    if(abs(x) > 0.001f) m_Orientation =                 (glm::angleAxis(-x, glm::vec3(1,0,0)));
    if(abs(y) > 0.001f) m_Orientation = m_Orientation * (glm::angleAxis(-y, glm::vec3(0,1,0)));
    if(abs(z) > 0.001f) m_Orientation = m_Orientation * (glm::angleAxis(z,  glm::vec3(0,0,1)));
    _updateModelMatrix();
}
void MeshInstance::playAnimation(const string& animName,float start,float end,uint reqLoops){
    epriv::MeshInstanceAnimation* anim = nullptr;
    anim = new epriv::MeshInstanceAnimation(*mesh(),animName, start, end, reqLoops);
    m_AnimationQueue.push_back(anim);
}
