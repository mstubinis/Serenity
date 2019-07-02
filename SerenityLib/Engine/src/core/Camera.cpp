#include <core/Camera.h>
#include <core/Scene.h>
#include <core/engine/Engine_Math.h>

#include <glm/gtx/norm.hpp>

using namespace Engine;
using namespace boost;
using namespace std;

class Camera::impl final{
    public:
        void _baseInit(Scene* scene, Camera& super){
            if(!scene)
                scene = Resources::getCurrentScene();        
            auto* m_Body = super.m_Entity.addComponent<ComponentBody>();
            super.m_Entity.getComponent<ComponentCamera>()->lookAt(m_Body->position(),m_Body->position() + m_Body->forward(),m_Body->up());
            auto* m_Logic = super.m_Entity.addComponent<ComponentLogic2>();
            m_Logic->setUserPointer(&super);
        }
};

Camera::Camera(float angle, float aspectRatio, float _near, float _far,Scene* scene):EntityWrapper(*scene),m_i(new impl){//create a perspective camera
    m_Entity.addComponent<ComponentCamera>(angle, aspectRatio, _near, _far);
    m_i->_baseInit(scene, *this);
}
Camera::Camera(float left, float right, float bottom, float top, float _near, float _far,Scene* scene): EntityWrapper(*scene),m_i(new impl){//create an orthographic camera
    m_Entity.addComponent<ComponentCamera>(left, right, bottom, top, _near, _far);
    m_i->_baseInit(scene, *this);
}
Camera::~Camera(){ 
}
const glm::vec3 Camera::getPosition(){ return m_Entity.getComponent<ComponentBody>()->position(); }
glm::quat Camera::getOrientation(){ return glm::conjugate(glm::quat_cast(m_Entity.getComponent<ComponentCamera>()->getView())); }
const float Camera::getAngle(){ return m_Entity.getComponent<ComponentCamera>()->m_Angle; }
const float Camera::getAspect(){ return m_Entity.getComponent<ComponentCamera>()->m_AspectRatio; }
const float Camera::getNear(){ return m_Entity.getComponent<ComponentCamera>()->m_NearPlane; }
const float Camera::getFar(){ return m_Entity.getComponent<ComponentCamera>()->m_FarPlane; }
void Camera::setAngle(float _Angle){ m_Entity.getComponent<ComponentCamera>()->setAngle(_Angle); }
void Camera::setAspect(float _Aspect){ m_Entity.getComponent<ComponentCamera>()->setAspect(_Aspect); }
void Camera::setNear(float _near){ m_Entity.getComponent<ComponentCamera>()->setNear(_near); }
void Camera::setFar(float _far){ m_Entity.getComponent<ComponentCamera>()->setFar(_far); }
const glm::mat4 Camera::getViewProjectionInverse(){ return m_Entity.getComponent<ComponentCamera>()->getViewProjectionInverse(); }
const glm::mat4 Camera::getProjection(){ return m_Entity.getComponent<ComponentCamera>()->getProjection(); }
const glm::mat4 Camera::getView(){ return m_Entity.getComponent<ComponentCamera>()->getView(); }
const glm::mat4 Camera::getViewInverse(){ return m_Entity.getComponent<ComponentCamera>()->getViewInverse(); }
const glm::mat4 Camera::getProjectionInverse(){ return m_Entity.getComponent<ComponentCamera>()->getProjectionInverse(); }
const glm::mat4 Camera::getViewProjection(){ return m_Entity.getComponent<ComponentCamera>()->getViewProjection(); }
const glm::vec3 Camera::getViewVector(){ return m_Entity.getComponent<ComponentCamera>()->getViewVector(); }
const glm::vec3 Camera::forward(){ return m_Entity.getComponent<ComponentCamera>()->forward(); }
const glm::vec3 Camera::right(){ return m_Entity.getComponent<ComponentCamera>()->right(); }
const glm::vec3 Camera::up(){ return m_Entity.getComponent<ComponentCamera>()->up(); }
float Camera::getDistance(Entity& e){
    auto& b = *e.getComponent<ComponentBody>();
    return glm::distance(b.position(),getPosition());
}
float Camera::getDistance(glm::vec3& objPos){ return glm::distance(objPos,getPosition()); }
float Camera::getDistanceSquared(Entity& e) {
    auto& b = *e.getComponent<ComponentBody>();
    return glm::distance2(b.position(), getPosition());
}
float Camera::getDistanceSquared(glm::vec3& objPos) { return glm::distance2(objPos, getPosition()); }
uint Camera::sphereIntersectTest(glm::vec3& pos, float radius){ return m_Entity.getComponent<ComponentCamera>()->sphereIntersectTest(pos,radius); }
uint Camera::pointIntersectTest(glm::vec3& pos){ return m_Entity.getComponent<ComponentCamera>()->pointIntersectTest(pos); }
bool Camera::rayIntersectSphere(Entity& entity){
    auto& body = *entity.getComponent<ComponentBody>();
    auto* model = entity.getComponent<ComponentModel>();
    auto& thisBody = *m_Entity.getComponent<ComponentBody>();
    auto& thisCamera = *m_Entity.getComponent<ComponentCamera>();
    float radius = 0.0f;
    if(model) radius = model->radius();
    return Math::rayIntersectSphere(body.position(), radius, thisBody.position(), thisCamera.getViewVector());
}
