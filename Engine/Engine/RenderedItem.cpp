#include "RenderedItem.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "Mesh.h"
#include "Material.h"
#include "Object.h"
#include "Camera.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <boost/weak_ptr.hpp>

using namespace Engine;

struct DefaultRenderedItemBindFunctor{void operator()(EngineResource* r) const {
    RenderedItem* i = static_cast<RenderedItem*>(r);

    boost::weak_ptr<Object> o = Resources::getObjectPtr(*i->parentPtr().lock().get());
    if(exists(o)){
        Object* obj = o.lock().get();
        Camera* c = Resources::getActiveCamera();
        if(obj->passedRenderCheck()){
            Renderer::sendUniformMatrix4f("Model",glm::mat4(o.lock().get()->getModel()) * i->model());
            i->mesh()->render();
        }
    }
}};
struct DefaultRenderedItemUnbindFunctor{void operator()(EngineResource* r) const {
    //RenderedItem* i = static_cast<RenderedItem*>(r);
}};

class RenderedItem::impl{
    public:
        static DefaultRenderedItemUnbindFunctor DEFAULT_UNBIND_FUNCTOR;
        static DefaultRenderedItemBindFunctor DEFAULT_BIND_FUNCTOR;

        Mesh* m_Mesh;
        Material* m_Material;
        glm::vec3 m_Position;
        glm::quat m_Orientation;
        glm::vec3 m_Scale;
        glm::mat4 m_Model;
        bool m_NeedsUpdate;
        boost::weak_ptr<std::string> m_ParentPtr;
        void _init(Mesh* mesh,Material* mat,glm::vec3& pos,glm::quat& rot,glm::vec3& scl,RenderedItem* super,boost::shared_ptr<std::string>& parentNamePtr){
            m_Mesh = mesh;
            m_Material = mat;
            m_Position = pos;
            m_Orientation = rot;
            m_Scale = scl;
            _updateModelMatrix();
            m_NeedsUpdate = false;

            std::string n = m_Mesh->name() + "_" + m_Material->name();
            n = Resources::Detail::ResourceManagement::_incrementName(Resources::Detail::ResourceManagement::m_RenderedItems,n);
            m_ParentPtr = boost::dynamic_pointer_cast<std::string>(parentNamePtr);

            super->setName(n);
            super->setCustomBindFunctor(RenderedItem::impl::DEFAULT_BIND_FUNCTOR);
            super->setCustomUnbindFunctor(RenderedItem::impl::DEFAULT_UNBIND_FUNCTOR);
        }
        void _destruct(){
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
DefaultRenderedItemBindFunctor RenderedItem::impl::DEFAULT_BIND_FUNCTOR;
DefaultRenderedItemUnbindFunctor RenderedItem::impl::DEFAULT_UNBIND_FUNCTOR;

RenderedItem::RenderedItem(boost::shared_ptr<std::string>& parentNamePtr, Mesh* mesh,Material* mat,glm::vec3& pos,glm::quat& rot,glm::vec3& scl):m_i(new impl()){
    m_i->_init(mesh,mat,pos,rot,scl,this,parentNamePtr);
    Resources::Detail::ResourceManagement::_addToContainer(Resources::Detail::ResourceManagement::m_RenderedItems,name(),boost::shared_ptr<RenderedItem>(this));
    mat->addObject(name());
}
RenderedItem::RenderedItem(boost::shared_ptr<std::string>& parentNamePtr,std::string mesh,std::string mat,glm::vec3& pos,glm::quat& rot,glm::vec3& scl):m_i(new impl()){
    Mesh* _mesh = Resources::getMesh(mesh);
    Material* _mat = Resources::getMaterial(mat);
    m_i->_init(_mesh,_mat,pos,rot,scl,this,parentNamePtr);
    Resources::Detail::ResourceManagement::_addToContainer(Resources::Detail::ResourceManagement::m_RenderedItems,name(),boost::shared_ptr<RenderedItem>(this));
    _mat->addObject(name());
}
RenderedItem::~RenderedItem(){ m_i->_destruct(); }

void RenderedItem::setPosition(float x, float y, float z){ m_i->_setPosition(x,y,z); }
void RenderedItem::setScale(float x,float y,float z){ m_i->_setScale(x,y,z); }
void RenderedItem::translate(float x, float y, float z){ m_i->_translate(x,y,z); }
void RenderedItem::rotate(float x, float y, float z){ m_i->_rotate(x,y,z); }
void RenderedItem::scale(float x,float y,float z){ m_i->_scale(x,y,z); }

void RenderedItem::setPosition(glm::vec3& v){ m_i->_setPosition(v.x,v.y,v.z); }
void RenderedItem::setScale(glm::vec3& v){ m_i->_setScale(v.x,v.y,v.z); }
void RenderedItem::translate(glm::vec3& v){ m_i->_translate(v.x,v.y,v.z); }
void RenderedItem::rotate(glm::vec3& v){ m_i->_rotate(v.x,v.y,v.z); }
void RenderedItem::scale(glm::vec3& v){ m_i->_scale(v.x,v.y,v.z); }

glm::mat4& RenderedItem::model(){ return m_i->m_Model; }
glm::vec3& RenderedItem::getScale(){ return m_i->m_Scale; }
glm::vec3& RenderedItem::position(){ return m_i->m_Position; }
glm::quat& RenderedItem::orientation(){ return m_i->m_Orientation; }
Mesh* RenderedItem::mesh(){ return m_i->m_Mesh; }
Material* RenderedItem::material(){ return m_i->m_Material; }

void RenderedItem::setMesh(std::string n){
    m_i->m_Mesh = Resources::getMesh(n);
}
void RenderedItem::setMesh(Mesh* m){
    m_i->m_Mesh = m;
}
void RenderedItem::setMaterial(std::string n){
    m_i->m_Material = Resources::getMaterial(n);
}
void RenderedItem::setMaterial(Material* m){
    m_i->m_Material = m;
}
void RenderedItem::setOrientation(glm::quat& o){ m_i->m_Orientation = o; }
void RenderedItem::setOrientation(float x,float y,float z){ 
    float threshold = 0;
    if(abs(x) > threshold) m_i->m_Orientation = (glm::angleAxis(-x, glm::vec3(1,0,0)));   //pitch
    if(abs(y) > threshold) m_i->m_Orientation = m_i->m_Orientation * (glm::angleAxis(-y, glm::vec3(0,1,0)));   //yaw
    if(abs(z) > threshold) m_i->m_Orientation = m_i->m_Orientation * (glm::angleAxis(z,  glm::vec3(0,0,1)));   //roll
    m_i->_updateModelMatrix();
}
std::string& RenderedItem::parent(){ return *(m_i->m_ParentPtr.lock().get()); }
boost::weak_ptr<std::string>& RenderedItem::parentPtr(){ return m_i->m_ParentPtr; }

void RenderedItem::update(float dt){
    m_i->_updateModelMatrix();
}