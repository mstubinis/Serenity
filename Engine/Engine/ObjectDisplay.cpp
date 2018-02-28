#include "ObjectDisplay.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "Camera.h"
#include "Material.h"
#include "Mesh.h"
#include "ShaderProgram.h"
#include "Scene.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace Engine;
using namespace std;

struct DefaultObjectDisplayBindFunctor{void operator()(BindableResource* r) const {
    ObjectDisplay* o = (ObjectDisplay*)r;

    Renderer::sendUniform4fSafe("Object_Color",o->getColor());
    Renderer::sendUniform3fSafe("Gods_Rays_Color",o->getGodsRaysColor());
}};
struct DefaultObjectDisplayUnbindFunctor{void operator()(BindableResource* r) const {
}};
DefaultObjectDisplayBindFunctor DEFAULT_BIND_FUNCTOR;
DefaultObjectDisplayUnbindFunctor DEFAULT_UNBIND_FUNCTOR;

ObjectDisplay::ObjectDisplay(string mesh, string mat, glm::vec3 pos, glm::vec3 scl, string n,Scene* scene):ObjectBasic(pos,scl,n,scene){
    m_Radius = 0;
	m_PassedRenderCheck = false;
    m_Visible = true;
    m_Shadeless = false;
    m_BoundingBoxRadius = glm::vec3(0);
    if(mesh != "" && mat != ""){
        MeshInstance* item = new MeshInstance(name(),mesh,mat);
        m_MeshInstances.push_back(item);
    }
    m_Color = glm::vec4(1);
    m_GodsRaysColor = glm::vec3(0);
    calculateRadius();

    setCustomBindFunctor(DEFAULT_BIND_FUNCTOR);
    setCustomUnbindFunctor(DEFAULT_UNBIND_FUNCTOR);
}
ObjectDisplay::~ObjectDisplay(){
}
void ObjectDisplay::update(float dt){
    ObjectBasic::update(dt);
    for(auto meshInstance:m_MeshInstances){
        meshInstance->update(dt);
    }
}
void ObjectDisplay::checkRender(Camera* camera){
    if(!m_Visible || !camera->sphereIntersectTest(getPosition(),m_Radius) || camera->getDistance(this) > m_Radius * Object::m_VisibilityThreshold){
        m_PassedRenderCheck = false;
		return;
    }
    m_PassedRenderCheck = true;
}
void ObjectDisplay::calculateRadius(){
    if(m_MeshInstances.size() == 0){
        m_BoundingBoxRadius = glm::vec3(0.0f);
        m_Radius = 0;
        return;
    }
    float maxLength = 0;
    for(auto meshInstance:m_MeshInstances){
        float length = 0;
        glm::mat4 m = meshInstance->model();
        glm::vec3 localPosition = glm::vec3(m[3][0],m[3][1],m[3][2]);
        length = glm::length(localPosition) + meshInstance->mesh()->getRadius() * Engine::Math::Max(meshInstance->getScale());
        if(length > maxLength){
            maxLength = length;
        }
    }
    m_BoundingBoxRadius = maxLength * m_Scale;
    m_Radius = Engine::Math::Max(m_BoundingBoxRadius);
}

void ObjectDisplay::setMesh(Mesh* mesh){ for(auto entry:m_MeshInstances){ entry->setMesh(mesh); } }
void ObjectDisplay::setMesh(const string& mesh){ for(auto entry:m_MeshInstances){ entry->setMesh(Resources::getMesh(mesh)); } }
void ObjectDisplay::setMaterial(Material* material){ for(auto entry:m_MeshInstances){ entry->setMaterial(material); } }
void ObjectDisplay::setMaterial(const string& material){ for(auto entry:m_MeshInstances){ entry->setMaterial(Resources::getMaterial(material)); } }

void ObjectDisplay::setColor(float r, float g, float b,float a){ Engine::Math::setColor(m_Color,r,g,b,a); }
void ObjectDisplay::setColor(glm::vec4 c){ setColor(c.x,c.y,c.z,c.w); }
void ObjectDisplay::setGodsRaysColor(float r, float g, float b){ Engine::Math::setColor(m_GodsRaysColor,r,g,b); }
void ObjectDisplay::setGodsRaysColor(glm::vec3 c){ setGodsRaysColor(c.x,c.y,c.z); }
void ObjectDisplay::setVisible(bool v){ m_Visible = v; }

void ObjectDisplay::setScale(float x,float y,float z){
    ObjectBasic::setScale(x,y,z);
    calculateRadius();
}
void ObjectDisplay::setScale(glm::vec3 s){ ObjectDisplay::setScale(s.x,s.y,s.z); }

void ObjectDisplay::scale(float x, float y,float z){
    ObjectBasic::scale(x,y,z);
    calculateRadius(); 
}
void ObjectDisplay::scale(glm::vec3 s){ ObjectDisplay::scale(s.x,s.y,s.z); }
bool ObjectDisplay::rayIntersectSphere(Camera* c){
    if(c == nullptr) c = Resources::getCurrentScene()->getActiveCamera();
    return c->rayIntersectSphere(this);
}
bool ObjectDisplay::rayIntersectSphere(glm::vec3 A, glm::vec3 rayVector){
    return Engine::Math::rayIntersectSphere(getPosition(),getRadius(),A,rayVector);
}
void ObjectDisplay::playAnimation(const string& animName,float startTime){
    for(auto meshInstance:m_MeshInstances){
        if(meshInstance->mesh()->animationData().count(animName)){
            meshInstance->playAnimation(animName,startTime);
        }
    }
}
void ObjectDisplay::playAnimation(const string& animName,float startTime,float endTime,uint requestedLoops){
    for(auto meshInstance:m_MeshInstances){
        if(meshInstance->mesh()->animationData().count(animName)){
            if(endTime < 0){
                endTime = meshInstance->mesh()->animationData().at(animName)->duration();
            }
            meshInstance->playAnimation(animName,startTime,endTime,requestedLoops);
        }
    }
}
void ObjectDisplay::suspend(){
    for(auto meshInstance:m_MeshInstances){
        if(meshInstance->mesh() != nullptr){
            meshInstance->mesh()->decrementUseCount();
            if(meshInstance->mesh()->useCount() == 0){
                meshInstance->mesh()->unload();
            }
        }
        if(meshInstance->material() != nullptr){
            meshInstance->material()->decrementUseCount();
            if(meshInstance->material()->useCount() == 0){
                meshInstance->material()->unload();
            }
        }
    }
}
void ObjectDisplay::resume(){
    for(auto meshInstance:m_MeshInstances){
        if(meshInstance->mesh() != nullptr){
            meshInstance->mesh()->incrementUseCount();
            meshInstance->mesh()->load();
        }
        if(meshInstance->material() != nullptr){
            meshInstance->material()->incrementUseCount();
            meshInstance->material()->load();
        }
    }
}
