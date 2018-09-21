#include "MeshInstance.h"
#include "Components.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "Engine_BuiltInResources.h"
#include "Engine_Math.h"
#include "Mesh.h"
#include "Material.h"
#include "Camera.h"
#include "Scene.h"
#include "ShaderProgram.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace Engine;
using namespace std;

namespace Engine {
    namespace epriv {
        struct MeshInstanceAnimation final : private Engine::epriv::noncopyable {
            friend struct Engine::epriv::DefaultMeshInstanceBindFunctor;
            friend struct Engine::epriv::DefaultMeshInstanceUnbindFunctor;

            uint m_CurrentLoops, m_RequestedLoops;
            float m_CurrentTime, m_StartTime, m_EndTime;
            string m_AnimationName;
            Mesh* m_Mesh;

            MeshInstanceAnimation(Mesh* _mesh, const std::string& _animName, float _startTime, float _endTime, uint _requestedLoops = 1) {
                m_CurrentLoops = 0;
                m_RequestedLoops = _requestedLoops;
                m_CurrentTime = 0;
                m_StartTime = _startTime;
                m_AnimationName = _animName;
                m_Mesh = _mesh;
                if (_endTime < 0) {
                    m_EndTime = m_Mesh->animationData().at(_animName)->duration();
                }else{
                    m_EndTime = _endTime;
                }
            }
        };
    };
};
class MeshInstance::impl{
    public:  
        vector<epriv::MeshInstanceAnimation*> m_AnimationQueue;
        Entity* m_Entity;
        ShaderP* m_ShaderProgram;
        Mesh* m_Mesh;
        Material* m_Material;
        glm::vec3 m_Position, m_Scale, m_GodRaysColor;
        glm::quat m_Orientation;
        glm::mat4 m_Model;
        glm::vec4 m_Color;
        bool m_PassedRenderCheck, m_Visible;
        void _init(Mesh* mesh,Material* mat,glm::vec3& pos,glm::quat& rot,glm::vec3& scl,MeshInstance* super,Entity* entity,ShaderP* program){
            m_PassedRenderCheck = false;
            m_Visible = true;
            m_Entity = entity;
            _setShaderProgram(program,super);
            _setMaterial(mat,super);
            _setMesh(mesh,super);
            
            m_Color = glm::vec4(1.0f);
            m_GodRaysColor = glm::vec3(0.0f);
            m_Position = pos;
            m_Orientation = rot;
            m_Scale = scl;
            _updateModelMatrix();
        }
        void _setShaderProgram(ShaderP* program, MeshInstance* super) {
            if (!program) {
                program = epriv::InternalShaderPrograms::Deferred;
            }
            m_ShaderProgram = program;
        }
        void _setMesh(Mesh* mesh,MeshInstance* super){
            m_Mesh = mesh;
        }
        void _setMaterial(Material* mat,MeshInstance* super){
            m_Material = mat;
        }
        void _destruct(MeshInstance* super){
            SAFE_DELETE_VECTOR(m_AnimationQueue);
        }
        void _setPosition(float x, float y, float z){ m_Position.x = x; m_Position.y = y; m_Position.z = z; _updateModelMatrix(); }
        void _setScale(float x, float y,float z){ m_Scale.x = x; m_Scale.y = y; m_Scale.z = z; _updateModelMatrix(); }
        void _translate(float x, float y, float z){ m_Position.x += x; m_Position.y += y; m_Position.z += z; _updateModelMatrix(); }
        void _scale(float x,float y,float z){ m_Scale.x += x; m_Scale.y += y; m_Scale.z += z; _updateModelMatrix(); }
        void _rotate(float x,float y,float z){
            float threshold = 0;
            if(abs(x) > threshold) m_Orientation = m_Orientation * (glm::angleAxis(-x, glm::vec3(1,0,0)));   //pitch
            if(abs(y) > threshold) m_Orientation = m_Orientation * (glm::angleAxis(-y, glm::vec3(0,1,0)));   //yaw
            if(abs(z) > threshold) m_Orientation = m_Orientation * (glm::angleAxis(z,  glm::vec3(0,0,1)));   //roll
            _updateModelMatrix();
        }
        void _updateModelMatrix(){
            m_Model = glm::mat4(1.0f);
            glm::mat4 translationMatrix = glm::translate(m_Position);
            glm::mat4 rotationMatrix = glm::mat4_cast(m_Orientation);
            glm::mat4 scaleMatrix = glm::scale(m_Scale);
            m_Model = translationMatrix * rotationMatrix * scaleMatrix * m_Model;
        }
};

struct epriv::DefaultMeshInstanceBindFunctor{void operator()(EngineResource* r) const {
    MeshInstance::impl& i = *((MeshInstance*)r)->m_i;
    glm::vec3 camPos = Resources::getCurrentScene()->getActiveCamera()->getPosition();
    Entity* parent = i.m_Entity;
    auto& body = *(parent->getComponent<ComponentBody>());
    glm::mat4 parentModel = body.modelMatrix();

    vector<MeshInstanceAnimation*>& animationQueue = i.m_AnimationQueue;
    Renderer::sendUniform4fSafe("Object_Color",i.m_Color);
    Renderer::sendUniform3fSafe("Gods_Rays_Color",i.m_GodRaysColor);
    if(animationQueue.size() > 0){
        vector<glm::mat4> transforms;
        //process the animation here
        for(uint j = 0; j < animationQueue.size(); ++j){
            auto& a = *(animationQueue.at(j));
            if(a.m_Mesh == i.m_Mesh){
                a.m_CurrentTime += Resources::dt();
                a.m_Mesh->playAnimation(transforms,a.m_AnimationName,a.m_CurrentTime);
                if(a.m_CurrentTime >= a.m_EndTime){
                    a.m_CurrentTime = 0;
                    ++a.m_CurrentLoops;
                }
            }
        }
        Renderer::sendUniform1iSafe("AnimationPlaying",1);
        Renderer::sendUniformMatrix4fvSafe("gBones[0]",transforms,transforms.size());

        //cleanup the animation queue
        for (auto it = animationQueue.cbegin(); it != animationQueue.cend();){
            MeshInstanceAnimation* anim = (*it);
            auto& a = *(anim);
            if (a.m_RequestedLoops > 0 && (a.m_CurrentLoops >= a.m_RequestedLoops)){
                SAFE_DELETE(anim); //do we need this?
                it = animationQueue.erase(it);
            }
            else{ ++it; }
        }
    }else{
        Renderer::sendUniform1iSafe("AnimationPlaying",0);
    }
    glm::mat4 model = parentModel * i.m_Model; //might need to reverse this order.
    model[3][0] -= camPos.x;
    model[3][1] -= camPos.y;
    model[3][2] -= camPos.z;

    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
     
    Renderer::sendUniformMatrix4fSafe("Model",model);
    Renderer::sendUniformMatrix3fSafe("NormalMatrix",normalMatrix);
}};
struct epriv::DefaultMeshInstanceUnbindFunctor{void operator()(EngineResource* r) const {
}};

epriv::DefaultMeshInstanceBindFunctor   DEFAULT_BIND_FUNCTOR;
epriv::DefaultMeshInstanceUnbindFunctor DEFAULT_UNBIND_FUNCTOR;


MeshInstance::MeshInstance(Entity* entity, Mesh* mesh,Material* mat, ShaderP* program, glm::vec3 pos,glm::quat rot,glm::vec3 scl):m_i(new impl){
    m_i->_init(mesh,mat,pos,rot,scl,this,entity, program);
    setCustomBindFunctor(DEFAULT_BIND_FUNCTOR);
    setCustomUnbindFunctor(DEFAULT_UNBIND_FUNCTOR);
}
MeshInstance::MeshInstance(Entity* entity,Handle mesh,Handle mat, ShaderP* program, glm::vec3 pos,glm::quat rot,glm::vec3 scl):m_i(new impl){
    Mesh* _mesh = (Mesh*)mesh.get();
    Material* _mat = (Material*)mat.get();
    m_i->_init(_mesh,_mat,pos,rot,scl,this,entity, program);
    setCustomBindFunctor(DEFAULT_BIND_FUNCTOR);
    setCustomUnbindFunctor(DEFAULT_UNBIND_FUNCTOR);
}
MeshInstance::MeshInstance(Entity* entity,Mesh* mesh,Handle mat, ShaderP* program, glm::vec3 pos,glm::quat rot,glm::vec3 scl):m_i(new impl){
    Material* _mat = (Material*)mat.get();
    m_i->_init(mesh,_mat,pos,rot,scl,this,entity, program);
    setCustomBindFunctor(DEFAULT_BIND_FUNCTOR);
    setCustomUnbindFunctor(DEFAULT_UNBIND_FUNCTOR);
}
MeshInstance::MeshInstance(Entity* entity,Handle mesh,Material* mat, ShaderP* program, glm::vec3 pos,glm::quat rot,glm::vec3 scl):m_i(new impl){
    Mesh* _mesh = (Mesh*)mesh.get();
    m_i->_init(_mesh,mat,pos,rot,scl,this,entity, program);
    setCustomBindFunctor(DEFAULT_BIND_FUNCTOR);
    setCustomUnbindFunctor(DEFAULT_UNBIND_FUNCTOR);
}
MeshInstance::~MeshInstance(){ m_i->_destruct(this); }
void MeshInstance::show() { m_i->m_Visible = true; }
void MeshInstance::hide() { m_i->m_Visible = false; }
bool MeshInstance::visible() { return m_i->m_Visible; }
bool MeshInstance::passedRenderCheck(){ return m_i->m_PassedRenderCheck; }
void MeshInstance::setPassedRenderCheck(bool b) { m_i->m_PassedRenderCheck = b; }
void MeshInstance::setColor(float r,float g,float b,float a){ Engine::Math::setColor(m_i->m_Color,r,g,b,a); }
void MeshInstance::setColor(glm::vec4 color){ MeshInstance::setColor(color.r,color.g,color.b,color.a); }
void MeshInstance::setGodRaysColor(float r,float g,float b){ Engine::Math::setColor(m_i->m_GodRaysColor,r,g,b); }
void MeshInstance::setGodRaysColor(glm::vec3 color){ MeshInstance::setGodRaysColor(color.r,color.g,color.b); }
void MeshInstance::setPosition(float x, float y, float z){ m_i->_setPosition(x,y,z); }
void MeshInstance::setScale(float x,float y,float z){ m_i->_setScale(x,y,z); }
void MeshInstance::translate(float x, float y, float z){ m_i->_translate(x,y,z); }
void MeshInstance::rotate(float x, float y, float z){ m_i->_rotate(x,y,z); }
void MeshInstance::scale(float x,float y,float z){ m_i->_scale(x,y,z); }
void MeshInstance::setPosition(glm::vec3 v){ m_i->_setPosition(v.x,v.y,v.z); }
void MeshInstance::setScale(glm::vec3 v){ m_i->_setScale(v.x,v.y,v.z); }
void MeshInstance::translate(glm::vec3 v){ m_i->_translate(v.x,v.y,v.z); }
void MeshInstance::rotate(glm::vec3 v){ m_i->_rotate(v.x,v.y,v.z); }
void MeshInstance::scale(glm::vec3 v){ m_i->_scale(v.x,v.y,v.z); }
Entity* MeshInstance::parent(){ return m_i->m_Entity; }
glm::vec4& MeshInstance::color(){ return m_i->m_Color; }
glm::vec3& MeshInstance::godRaysColor(){ return m_i->m_GodRaysColor; }
glm::mat4& MeshInstance::model(){ return m_i->m_Model; }
glm::vec3& MeshInstance::getScale(){ return m_i->m_Scale; }
glm::vec3& MeshInstance::position(){ return m_i->m_Position; }
glm::quat& MeshInstance::orientation(){ return m_i->m_Orientation; }
ShaderP* MeshInstance::shaderProgram() { return m_i->m_ShaderProgram; }
Mesh* MeshInstance::mesh(){ return m_i->m_Mesh; }
Material* MeshInstance::material(){ return m_i->m_Material; }
void MeshInstance::setShaderProgram(Handle& shaderPHandle) { m_i->_setShaderProgram(((ShaderP*)shaderPHandle.get()), this); }
void MeshInstance::setShaderProgram(ShaderP* shaderProgram) { m_i->_setShaderProgram(shaderProgram, this); }
void MeshInstance::setMesh(Handle& meshHandle){ m_i->_setMesh(((Mesh*)meshHandle.get()),this); }
void MeshInstance::setMesh(Mesh* m){ m_i->_setMesh(m,this); }
void MeshInstance::setMaterial(Handle& materialHandle){ m_i->_setMaterial(((Material*)materialHandle.get()),this); }
void MeshInstance::setMaterial(Material* m){ m_i->_setMaterial(m,this); }
void MeshInstance::setOrientation(glm::quat o){ m_i->m_Orientation = o; }
void MeshInstance::setOrientation(float x,float y,float z){ 
    if(abs(x) > 0.001f) m_i->m_Orientation =                      (glm::angleAxis(-x, glm::vec3(1,0,0)));
    if(abs(y) > 0.001f) m_i->m_Orientation = m_i->m_Orientation * (glm::angleAxis(-y, glm::vec3(0,1,0)));
    if(abs(z) > 0.001f) m_i->m_Orientation = m_i->m_Orientation * (glm::angleAxis(z,  glm::vec3(0,0,1)));
    m_i->_updateModelMatrix();
}
void MeshInstance::playAnimation(const string& animName,float start,float end,uint reqLoops){
    epriv::MeshInstanceAnimation* anim = nullptr;
    Mesh* _mesh = mesh();
    anim = new epriv::MeshInstanceAnimation(_mesh,animName, start, end, reqLoops);
    m_i->m_AnimationQueue.push_back(anim);
}