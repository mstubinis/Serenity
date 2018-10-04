#include "Camera.h"
#include "Scene.h"
#include "Engine_Math.h"

using namespace Engine;
using namespace boost;
using namespace std;

class Camera::impl final{
    public:
        void _baseInit(Scene* scene, Camera& super){
            if(!scene)
                scene = Resources::getCurrentScene();   
            scene->addEntity(super);
            super.m_Body = new OLD_ComponentBody();
            super.addComponent(super.m_Body);
            super.m_Camera->lookAt(super.m_Body->position(),super.m_Body->position() + super.m_Body->forward(),super.m_Body->up());      
            super.addComponent(super.m_Camera); 
        }
        void _init(float& angle, float& aspectRatio, float& _near, float& _far,Scene* scene,Camera& super){
            super.m_Camera = new OLD_ComponentCamera(angle,aspectRatio,_near,_far);
            _baseInit(scene,super);
        }
        void _init(float& left, float& right, float& bottom, float& top, float& _near, float& _far,Scene* scene,Camera& super){		
            super.m_Camera = new OLD_ComponentCamera(left,right,bottom,top,_near,_far);
            _baseInit(scene,super);
        }
};

Camera::Camera(float angle, float aspectRatio, float _near, float _far,Scene* scene):m_i(new impl){//create a perspective camera
    m_i->_init(angle,aspectRatio,_near,_far,scene,*this);
}
Camera::Camera(float left, float right, float bottom, float top, float _near, float _far,Scene* scene):m_i(new impl){//create an orthographic camera
    m_i->_init(left,right,bottom,top,_near,_far,scene,*this);
}
Camera::~Camera(){ 
}
const glm::vec3 Camera::getPosition(){ return m_Body->position(); }
glm::quat Camera::getOrientation(){ return glm::conjugate(glm::quat_cast(m_Camera->getView())); }
const float Camera::getAngle() const{ return m_Camera->_angle; }
const float Camera::getAspect() const{ return m_Camera->_aspectRatio; }
const float Camera::getNear() const{ return m_Camera->_nearPlane; }
const float Camera::getFar() const{ return m_Camera->_farPlane; }
void Camera::setAngle(float _Angle){ m_Camera->_angle = _Angle; }
void Camera::setAspect(float _Aspect){ m_Camera->_aspectRatio = _Aspect; }
void Camera::setNear(float _near){ m_Camera->setNear(_near); }
void Camera::setFar(float _far){ m_Camera->setFar(_far); }
const glm::mat4 Camera::getViewProjectionInverse(){ return m_Camera->getViewProjectionInverse(); }
const glm::mat4 Camera::getProjection(){ return m_Camera->getProjection(); }
const glm::mat4 Camera::getView(){ return m_Camera->getView(); }
const glm::mat4 Camera::getViewInverse(){ return m_Camera->getViewInverse(); }
const glm::mat4 Camera::getProjectionInverse(){ return m_Camera->getProjectionInverse(); }
const glm::mat4 Camera::getViewProjection(){ return m_Camera->getViewProjection(); }
const glm::vec3 Camera::getViewVector(){ return m_Camera->getViewVector(); }
const glm::vec3 Camera::forward(){ return m_Body->forward(); }
const glm::vec3 Camera::right(){ return m_Body->right(); }
const glm::vec3 Camera::up(){ return m_Body->up(); }
float Camera::getDistance(OLD_Entity* e){
    auto* b = e->getComponent<OLD_ComponentBody>();
    return glm::distance(b->position(),getPosition());
}
float Camera::getDistance(glm::vec3 objPos){ return glm::distance(objPos,getPosition()); }
uint Camera::sphereIntersectTest(glm::vec3 pos, float radius){ return m_Camera->sphereIntersectTest(pos,radius); }
uint Camera::pointIntersectTest(glm::vec3 pos){ return m_Camera->pointIntersectTest(pos); }
bool Camera::rayIntersectSphere(OLD_Entity* entity){
    auto& body = *(entity->getComponent<OLD_ComponentBody>());
    auto* model = entity->getComponent<OLD_ComponentModel>();
    float radius = 0.0f;
    if(model) radius = model->radius();
    return Math::rayIntersectSphere(body.position(),radius,m_Body->position(),m_Camera->getViewVector());
}

