#include "core/MeshInstance.h"
#include "core/Components.h"
#include "core/engine/Engine_Resources.h"
#include "core/engine/Engine_Renderer.h"
#include "core/engine/Engine_BuiltInResources.h"
#include "core/engine/Engine_Math.h"
#include "core/Mesh.h"
#include "core/Material.h"
#include "core/Camera.h"
#include "core/Scene.h"
#include "core/ShaderProgram.h"

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
class MeshInstance::impl{
    public:  
        vector<epriv::MeshInstanceAnimation*> m_AnimationQueue;
        OLD_Entity* m_Entity;
        ShaderP* m_ShaderProgram;
        Mesh* m_Mesh;
        Material* m_Material;
        RenderStage::Stage m_Stage;
        glm::vec3 m_Position, m_Scale, m_GodRaysColor;
        glm::quat m_Orientation;
        glm::mat4 m_Model;
        glm::vec4 m_Color;
        bool m_PassedRenderCheck, m_Visible;
        void _init(Mesh* mesh,Material* mat, OLD_Entity& entity,ShaderP* program){
            m_Stage = RenderStage::GeometryOpaque;
            m_PassedRenderCheck = false;
            m_Visible = true;
            m_Entity = &entity;

            _setShaderProgram(program);
            _setMaterial(mat);
            _setMesh(mesh); 

            m_Color = glm::vec4(1.0f);
            m_GodRaysColor = glm::vec3(0.0f);
            m_Position = glm::vec3(0.0f);
            m_Orientation = glm::quat(1.0f,0.0f,0.0f,0.0f);
            m_Scale = glm::vec3(1.0f);
            _updateModelMatrix();
        }
        void _init(Mesh* mesh, Material* mat, Entity& entity, ShaderP* program) {
            m_Stage = RenderStage::GeometryOpaque;
            m_PassedRenderCheck = false;
            m_Visible = true;
            //m_Entity = entity;

            _setShaderProgram(program);
            _setMaterial(mat);
            _setMesh(mesh);

            m_Color = glm::vec4(1.0f);
            m_GodRaysColor = glm::vec3(0.0f);
            m_Position = glm::vec3(0.0f);
            m_Orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
            m_Scale = glm::vec3(1.0f);
            _updateModelMatrix();
        }
        void _setShaderProgram(ShaderP* program) {
            if (!program) { program = epriv::InternalShaderPrograms::Deferred; }
            m_ShaderProgram = program;
        }
        void _setMesh(Mesh* mesh){
            m_Mesh = mesh;
        }
        void _setMaterial(Material* mat){
            m_Material = mat;
        }
        void _destruct(){
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

void epriv::DefaultMeshInstanceBindFunctor::operator()(EngineResource* r) const {
    MeshInstance::impl& i = *((MeshInstance*)r)->m_i;
    glm::vec3 camPos = Resources::getCurrentScene()->getActiveCamera()->getPosition();
    OLD_Entity* parent = i.m_Entity;
    auto& body = *(parent->getComponent<OLD_ComponentBody>());
    glm::mat4 parentModel = body.modelMatrix();

    vector<MeshInstanceAnimation*>& animationQueue = i.m_AnimationQueue;
    Renderer::sendUniform4Safe("Object_Color",i.m_Color);
    Renderer::sendUniform3Safe("Gods_Rays_Color",i.m_GodRaysColor);
    if(animationQueue.size() > 0){
        vector<glm::mat4> transforms;
        //process the animation here
        for(uint j = 0; j < animationQueue.size(); ++j){
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
            auto& a = *(anim);
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

epriv::DefaultMeshInstanceBindFunctor   DEFAULT_BIND_FUNCTOR;
epriv::DefaultMeshInstanceUnbindFunctor DEFAULT_UNBIND_FUNCTOR;


MeshInstance::MeshInstance(OLD_Entity& entity, Mesh* mesh,Material* mat, ShaderP* program):m_i(new impl){
    m_i->_init(mesh,mat,entity, program);
    setCustomBindFunctor(DEFAULT_BIND_FUNCTOR);
    setCustomUnbindFunctor(DEFAULT_UNBIND_FUNCTOR);
}
MeshInstance::MeshInstance(OLD_Entity& entity,Handle mesh,Handle mat, ShaderP* program):m_i(new impl){
    Mesh* _mesh = (Mesh*)mesh.get();
    Material* _mat = (Material*)mat.get();
    m_i->_init(_mesh,_mat,entity, program);
    setCustomBindFunctor(DEFAULT_BIND_FUNCTOR);
    setCustomUnbindFunctor(DEFAULT_UNBIND_FUNCTOR);
}
MeshInstance::MeshInstance(OLD_Entity& entity,Mesh* mesh,Handle mat, ShaderP* program):m_i(new impl){
    Material* _mat = (Material*)mat.get();
    m_i->_init(mesh,_mat,entity, program);
    setCustomBindFunctor(DEFAULT_BIND_FUNCTOR);
    setCustomUnbindFunctor(DEFAULT_UNBIND_FUNCTOR);
}
MeshInstance::MeshInstance(OLD_Entity& entity,Handle mesh,Material* mat, ShaderP* program):m_i(new impl){
    Mesh* _mesh = (Mesh*)mesh.get();
    m_i->_init(_mesh,mat,entity, program);
    setCustomBindFunctor(DEFAULT_BIND_FUNCTOR);
    setCustomUnbindFunctor(DEFAULT_UNBIND_FUNCTOR);
}



MeshInstance::MeshInstance(Entity& entity, Mesh* mesh, Material* mat, ShaderP* program) :m_i(new impl) {
    m_i->_init(mesh, mat, entity, program);
    setCustomBindFunctor(DEFAULT_BIND_FUNCTOR);
    setCustomUnbindFunctor(DEFAULT_UNBIND_FUNCTOR);
}
MeshInstance::MeshInstance(Entity& entity, Handle mesh, Handle mat, ShaderP* program) : m_i(new impl) {
    Mesh* _mesh = (Mesh*)mesh.get();
    Material* _mat = (Material*)mat.get();
    m_i->_init(_mesh, _mat, entity, program);
    setCustomBindFunctor(DEFAULT_BIND_FUNCTOR);
    setCustomUnbindFunctor(DEFAULT_UNBIND_FUNCTOR);
}
MeshInstance::MeshInstance(Entity& entity, Mesh* mesh, Handle mat, ShaderP* program) :m_i(new impl) {
    Material* _mat = (Material*)mat.get();
    m_i->_init(mesh, _mat, entity, program);
    setCustomBindFunctor(DEFAULT_BIND_FUNCTOR);
    setCustomUnbindFunctor(DEFAULT_UNBIND_FUNCTOR);
}
MeshInstance::MeshInstance(Entity& entity, Handle mesh, Material* mat, ShaderP* program) :m_i(new impl) {
    Mesh* _mesh = (Mesh*)mesh.get();
    m_i->_init(_mesh, mat, entity, program);
    setCustomBindFunctor(DEFAULT_BIND_FUNCTOR);
    setCustomUnbindFunctor(DEFAULT_UNBIND_FUNCTOR);
}



MeshInstance::~MeshInstance(){ m_i->_destruct(); }
void MeshInstance::setStage(RenderStage::Stage s) { m_i->m_Stage = s; }
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
OLD_Entity* MeshInstance::parent(){ return m_i->m_Entity; }
glm::vec4& MeshInstance::color(){ return m_i->m_Color; }
glm::vec3& MeshInstance::godRaysColor(){ return m_i->m_GodRaysColor; }
glm::mat4& MeshInstance::model(){ return m_i->m_Model; }
glm::vec3& MeshInstance::getScale(){ return m_i->m_Scale; }
glm::vec3& MeshInstance::position(){ return m_i->m_Position; }
glm::quat& MeshInstance::orientation(){ return m_i->m_Orientation; }
ShaderP* MeshInstance::shaderProgram() { return m_i->m_ShaderProgram; }
Mesh* MeshInstance::mesh(){ return m_i->m_Mesh; }
Material* MeshInstance::material(){ return m_i->m_Material; }
RenderStage::Stage MeshInstance::stage() { return m_i->m_Stage; }
void MeshInstance::setShaderProgram(const Handle& shaderPHandle) { m_i->_setShaderProgram(((ShaderP*)shaderPHandle.get())); }
void MeshInstance::setShaderProgram(ShaderP* shaderProgram) { m_i->_setShaderProgram(shaderProgram); }
void MeshInstance::setMesh(const Handle& meshHandle){ m_i->_setMesh(((Mesh*)meshHandle.get())); }
void MeshInstance::setMesh(Mesh* m){ m_i->_setMesh(m); }
void MeshInstance::setMaterial(const Handle& materialHandle){ m_i->_setMaterial(((Material*)materialHandle.get())); }
void MeshInstance::setMaterial(Material* m){ m_i->_setMaterial(m); }
void MeshInstance::setOrientation(glm::quat o){ m_i->m_Orientation = o; }
void MeshInstance::setOrientation(float x,float y,float z){ 
    if(abs(x) > 0.001f) m_i->m_Orientation =                      (glm::angleAxis(-x, glm::vec3(1,0,0)));
    if(abs(y) > 0.001f) m_i->m_Orientation = m_i->m_Orientation * (glm::angleAxis(-y, glm::vec3(0,1,0)));
    if(abs(z) > 0.001f) m_i->m_Orientation = m_i->m_Orientation * (glm::angleAxis(z,  glm::vec3(0,0,1)));
    m_i->_updateModelMatrix();
}
void MeshInstance::playAnimation(const string& animName,float start,float end,uint reqLoops){
    epriv::MeshInstanceAnimation* anim = nullptr;
    anim = new epriv::MeshInstanceAnimation(*mesh(),animName, start, end, reqLoops);
    m_i->m_AnimationQueue.push_back(anim);
}