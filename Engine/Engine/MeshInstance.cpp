#include "MeshInstance.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "Mesh.h"
#include "Material.h"
#include "Object.h"
#include "Camera.h"
#include "Engine_AnimationProcessor.h"
#include "Object.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <boost/weak_ptr.hpp>

using namespace Engine;

struct DefaultMeshInstanceBindFunctor{void operator()(EngineResource* r) const {
    MeshInstance* i = static_cast<MeshInstance*>(r);
    boost::weak_ptr<Object> o = Resources::getObjectPtr(i->parent()->name());
    Object* obj = o.lock().get();
    std::vector<MeshInstanceAnimation>& q = i->animationQueue();
    if(q.size() > 0){
        std::vector<glm::mat4> transforms;
        AnimationProcessor processor = AnimationProcessor();
        processor.process(i,q,transforms);
    }
    else{
        Renderer::sendUniform1iSafe("AnimationPlaying",0);
    }
    Renderer::sendUniformMatrix4f("Model",glm::mat4(obj->getModel()) * i->model());  
    i->render();
    Renderer::sendUniform1iSafe("AnimationPlaying",0); //this is needed here. cant seem to find out why...

}};
struct DefaultMeshInstanceUnbindFunctor{void operator()(EngineResource* r) const {
}};

class MeshInstance::impl{
    public:
        static DefaultMeshInstanceUnbindFunctor DEFAULT_UNBIND_FUNCTOR;
        static DefaultMeshInstanceBindFunctor DEFAULT_BIND_FUNCTOR;

        std::vector<MeshInstanceAnimation> m_AnimationQueue;

        Mesh* m_Mesh;
		Material* m_Material;
        glm::vec3 m_Position;
        glm::quat m_Orientation;
        glm::vec3 m_Scale;
        glm::mat4 m_Model;
        bool m_NeedsUpdate;
        Object* m_Parent;
        void _init(Mesh* mesh,Material* mat,glm::vec3& pos,glm::quat& rot,glm::vec3& scl,MeshInstance* super,std::string& parentName){
            _setMesh(mesh);
            _setMaterial(mat);

            m_Position = pos;
            m_Orientation = rot;
            m_Scale = scl;
            _updateModelMatrix();
            m_NeedsUpdate = false;

            std::string n = m_Mesh->name() + "_" + m_Material->name();
            n = Resources::Detail::ResourceManagement::_incrementName(Resources::Detail::ResourceManagement::m_RenderedItems,n);
            m_Parent = Resources::getObject(parentName);

            super->setName(n);
            super->setCustomBindFunctor(MeshInstance::impl::DEFAULT_BIND_FUNCTOR);
            super->setCustomUnbindFunctor(MeshInstance::impl::DEFAULT_UNBIND_FUNCTOR);
        }
        void _setMesh(Mesh* mesh){
            m_Mesh = mesh;
            m_Mesh->incrementUseCount();
        }
        void _setMaterial(Material* mat){
			m_Material = mat;
            m_Material->incrementUseCount();
        }
        void _destruct(MeshInstance* super){
			for(auto meshInstance:m_Material->getMeshes()){
				if(meshInstance->mesh() == m_Mesh){
					meshInstance->removeMeshInstance(m_Parent->name(),super);
					break;
				}
			}
        }
        void _setPosition(float x, float y, float z){ m_Position.x = x; m_Position.y = y; m_Position.z = z; m_NeedsUpdate = true; }
        void _setScale(float x, float y,float z){ m_Scale.x = x; m_Scale.y = y; m_Scale.z = z; m_NeedsUpdate = true; }
        void _translate(float x, float y, float z){ m_Position.x += x; m_Position.y += y; m_Position.z += z; m_NeedsUpdate = true; }
        void _scale(float x,float y,float z){ m_Scale.x += x; m_Scale.y += y; m_Scale.z += z; m_NeedsUpdate = true; }
        void _rotate(float x,float y,float z){
            float threshold = 0;
            if(abs(x) > threshold) m_Orientation = m_Orientation * (glm::angleAxis(-x, glm::vec3(1,0,0)));   //pitch
            if(abs(y) > threshold) m_Orientation = m_Orientation * (glm::angleAxis(-y, glm::vec3(0,1,0)));   //yaw
            if(abs(z) > threshold) m_Orientation = m_Orientation * (glm::angleAxis(z,  glm::vec3(0,0,1)));   //roll
            m_NeedsUpdate = true;
        }
        void _updateModelMatrix(){
            if(m_NeedsUpdate){
                m_Model = glm::mat4(1);
                m_Model = glm::translate(m_Model, m_Position);
                m_Model *= glm::mat4_cast(m_Orientation);
                m_Model = glm::scale(m_Model,m_Scale);
                m_NeedsUpdate = false;
            }
        }
};
DefaultMeshInstanceBindFunctor MeshInstance::impl::DEFAULT_BIND_FUNCTOR;
DefaultMeshInstanceUnbindFunctor MeshInstance::impl::DEFAULT_UNBIND_FUNCTOR;

MeshInstance::MeshInstance(std::string& parentName, Mesh* mesh,Material* mat,glm::vec3& pos,glm::quat& rot,glm::vec3& scl):m_i(new impl){
    m_i->_init(mesh,mat,pos,rot,scl,this,parentName);
    Resources::Detail::ResourceManagement::_addToContainer(Resources::Detail::ResourceManagement::m_RenderedItems,name(),boost::shared_ptr<MeshInstance>(this));
	
	mat->addMesh(mesh->name());
	for(auto meshEntry:mat->getMeshes()){
		if(meshEntry->mesh() == mesh){
			meshEntry->addMeshInstance(parentName,this);
		}
	}
}
MeshInstance::MeshInstance(std::string& parentName,std::string mesh,std::string mat,glm::vec3& pos,glm::quat& rot,glm::vec3& scl):m_i(new impl){
    Mesh* _mesh = Resources::getMesh(mesh);
    Material* _mat = Resources::getMaterial(mat);
    m_i->_init(_mesh,_mat,pos,rot,scl,this,parentName);
    Resources::Detail::ResourceManagement::_addToContainer(Resources::Detail::ResourceManagement::m_RenderedItems,name(),boost::shared_ptr<MeshInstance>(this));
	
	_mat->addMesh(_mesh->name());
	for(auto meshEntry:_mat->getMeshes()){
		if(meshEntry->mesh() == _mesh){
			meshEntry->addMeshInstance(parentName,this);
			break;
		}
	}
}
MeshInstance::~MeshInstance(){ m_i->_destruct(this); }

void MeshInstance::setPosition(float x, float y, float z){ m_i->_setPosition(x,y,z); }
void MeshInstance::setScale(float x,float y,float z){ m_i->_setScale(x,y,z); }
void MeshInstance::translate(float x, float y, float z){ m_i->_translate(x,y,z); }
void MeshInstance::rotate(float x, float y, float z){ m_i->_rotate(x,y,z); }
void MeshInstance::scale(float x,float y,float z){ m_i->_scale(x,y,z); }

void MeshInstance::setPosition(glm::vec3& v){ m_i->_setPosition(v.x,v.y,v.z); }
void MeshInstance::setScale(glm::vec3& v){ m_i->_setScale(v.x,v.y,v.z); }
void MeshInstance::translate(glm::vec3& v){ m_i->_translate(v.x,v.y,v.z); }
void MeshInstance::rotate(glm::vec3& v){ m_i->_rotate(v.x,v.y,v.z); }
void MeshInstance::scale(glm::vec3& v){ m_i->_scale(v.x,v.y,v.z); }

glm::mat4& MeshInstance::model(){ return m_i->m_Model; }
glm::vec3& MeshInstance::getScale(){ return m_i->m_Scale; }
glm::vec3& MeshInstance::position(){ return m_i->m_Position; }
glm::quat& MeshInstance::orientation(){ return m_i->m_Orientation; }
Mesh* MeshInstance::mesh(){ return m_i->m_Mesh; }
Material* MeshInstance::material(){ return m_i->m_Material; }

void MeshInstance::setMesh(std::string n){ m_i->m_Mesh = Resources::getMesh(n); }
void MeshInstance::setMesh(Mesh* m){ m_i->m_Mesh = m; }
void MeshInstance::setMaterial(std::string n){ m_i->m_Material = Resources::getMaterial(n); }
void MeshInstance::setMaterial(Material* m){ m_i->m_Material = m; }
void MeshInstance::setOrientation(glm::quat& o){ m_i->m_Orientation = o; }
void MeshInstance::setOrientation(float x,float y,float z){ 
    float threshold = 0;
    if(abs(x) > threshold) m_i->m_Orientation = (glm::angleAxis(-x, glm::vec3(1,0,0)));   //pitch
    if(abs(y) > threshold) m_i->m_Orientation = m_i->m_Orientation * (glm::angleAxis(-y, glm::vec3(0,1,0)));   //yaw
    if(abs(z) > threshold) m_i->m_Orientation = m_i->m_Orientation * (glm::angleAxis(z,  glm::vec3(0,0,1)));   //roll
    m_i->_updateModelMatrix();
}
Object* MeshInstance::parent(){ return m_i->m_Parent; }

void MeshInstance::update(float dt){
    m_i->_updateModelMatrix();
}
void MeshInstance::render(){
	m_i->m_Mesh->render();
}
void MeshInstance::playAnimation(const std::string& animName,float startTime){
    MeshInstanceAnimation anim(mesh(),animName,startTime,mesh()->animationData().at(animName)->duration());
    m_i->m_AnimationQueue.push_back(anim);
}
void MeshInstance::playAnimation(const std::string& animName,float startTime,float endTime,uint requestedLoops){
    MeshInstanceAnimation anim(mesh(),animName,startTime,endTime,requestedLoops);
    m_i->m_AnimationQueue.push_back(anim);
}
std::vector<MeshInstanceAnimation>& MeshInstance::animationQueue(){ return m_i->m_AnimationQueue; }

MeshInstanceAnimation::MeshInstanceAnimation(Mesh* _mesh,std::string _animName,float _startTime,float _duration){
    animName = _animName;
    startTime = _startTime;
    endTime = _duration;
    mesh = _mesh;
    currentLoops = 0;
    currentTime = 0;
    requestedLoops = 1;
}
MeshInstanceAnimation::MeshInstanceAnimation(Mesh* _mesh,std::string _animName,float _startTime,float _endTime,uint _requestedLoops){
    animName = _animName;
    startTime = _startTime;
    endTime = _endTime;
    mesh = _mesh;
    currentLoops = 0;
    currentTime = 0;
    requestedLoops = _requestedLoops;
}
MeshInstanceAnimation::~MeshInstanceAnimation(){
}