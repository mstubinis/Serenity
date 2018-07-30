#include "Engine.h"
#include "Scene.h"
#include "Camera.h"
#include "Engine_Resources.h"
#include "Engine_Math.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <boost/make_shared.hpp>

using namespace Engine;
using namespace boost;
using namespace std;

class Camera::impl final{
    public:
        void _baseInit(Scene* scene, Camera* super){
            scene->addEntity(super);

            super->m_BasicBody = new ComponentBasicBody();
            super->m_Camera->lookAt(super->m_BasicBody->position(),super->m_BasicBody->position() + super->m_BasicBody->forward(),super->m_BasicBody->up());
            if(scene == nullptr)
                scene = Resources::getCurrentScene();

            super->addComponent(super->m_BasicBody);
            super->addComponent(super->m_Camera);	
        }
        void _init(float& angle, float& aspectRatio, float& _near, float& _far,Scene* scene,Camera* super){
            super->m_Camera = new ComponentCamera(angle,aspectRatio,_near,_far);
            _baseInit(scene,super);
        }
        void _init(float& left, float& right, float& bottom, float& top, float& _near, float& _far,Scene* scene,Camera* super){		
            super->m_Camera = new ComponentCamera(left,right,bottom,top,_near,_far);
            _baseInit(scene,super);
        }
};

Camera::Camera(float angle, float aspectRatio, float _near, float _far,Scene* scene):Entity(),m_i(new impl){//create a perspective camera
    m_i->_init(angle,aspectRatio,_near,_far,scene,this);
}
Camera::Camera(float left, float right, float bottom, float top, float _near, float _far,Scene* scene):Entity(),m_i(new impl){//create an orthographic camera
    m_i->_init(left,right,bottom,top,_near,_far,scene,this);
}
Camera::~Camera(){ 
}
glm::vec3 Camera::getPosition(){ return m_BasicBody->position(); }
glm::quat Camera::getOrientation(){ return glm::conjugate(glm::quat_cast(m_Camera->getView())); }
const float Camera::getAngle() const{ return m_Camera->_angle; }
const float Camera::getAspect() const{ return m_Camera->_aspectRatio; }
const float Camera::getNear() const{ return m_Camera->_nearPlane; }
const float Camera::getFar() const{ return m_Camera->_farPlane; }
void Camera::setAngle(float _Angle){ m_Camera->_angle = _Angle; }
void Camera::setAspect(float _Aspect){ m_Camera->_aspectRatio = _Aspect; }
void Camera::setNear(float _near){ m_Camera->setNear(_near); }
void Camera::setFar(float _far){ m_Camera->setFar(_far); }
glm::mat4 Camera::getViewProjectionInverse(){ return m_Camera->getViewProjectionInverse(); }
glm::mat4 Camera::getProjection(){ return m_Camera->getProjection(); }
glm::mat4 Camera::getView(){ return m_Camera->getView(); }
glm::mat4 Camera::getViewInverse(){ return m_Camera->getViewInverse(); }
glm::mat4 Camera::getProjectionInverse(){ return m_Camera->getProjectionInverse(); }
glm::mat4 Camera::getViewProjection(){ return m_Camera->getViewProjection(); }
glm::vec3 Camera::getViewVector(){ return m_Camera->getViewVector(); }
glm::vec3 Camera::forward(){ return m_BasicBody->forward(); }
glm::vec3 Camera::right(){ return m_BasicBody->right(); }
glm::vec3 Camera::up(){ return m_BasicBody->up(); }

float Camera::getDistance(Entity* e){
    epriv::ComponentBodyBaseClass* b = e->getComponent<epriv::ComponentBodyBaseClass>();
    return glm::distance(b->position(),getPosition());
}
float Camera::getDistance(glm::vec3 objPos){
    return glm::distance(objPos,getPosition());
}
bool Camera::sphereIntersectTest(glm::vec3 pos, float radius){
    return m_Camera->sphereIntersectTest(pos,radius);
}
bool Camera::rayIntersectSphere(Entity* entity){
    epriv::ComponentBodyBaseClass& body = *(entity->getComponent<epriv::ComponentBodyBaseClass>());
    ComponentModel* model = entity->getComponent<ComponentModel>();
    float radius = 0.0f;
    if(model) radius = model->radius();
    return Engine::Math::rayIntersectSphere(body.position(),radius,m_BasicBody->position(),m_Camera->getViewVector());
}