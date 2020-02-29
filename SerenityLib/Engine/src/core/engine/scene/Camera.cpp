#include <core/engine/scene/Camera.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/resources/Engine_Resources.h>
#include <glm/gtx/norm.hpp>

using namespace Engine;
using namespace boost;
using namespace std;

Camera::Camera(const float angle, const float aspectRatio, const float Near, const float Far, Scene* scene) : EntityWrapper(*scene){//create a perspective camera
    if (!scene) {
        scene = Resources::getCurrentScene();
    }
    m_Entity.addComponent<ComponentCamera>(angle, aspectRatio, Near, Far);
    m_Entity.addComponent<ComponentLogic2>();
    m_Entity.addComponent<ComponentBody>();

    auto& cam   = *m_Entity.getComponent<ComponentCamera>();
    auto& logic = *m_Entity.getComponent<ComponentLogic2>();
    auto& body  = *m_Entity.getComponent<ComponentBody>();

    cam.lookAt(glm_vec3(0.0), glm_vec3(0.0) + body.forward(), body.up());
    logic.setUserPointer(this);
}
Camera::Camera(const float left, const float right, const float bottom, const float top, const float Near, const float Far, Scene* scene) : EntityWrapper(*scene){//create an orthographic camera
    if (!scene) {
        scene = Resources::getCurrentScene();
    }
    m_Entity.addComponent<ComponentCamera>(left, right, bottom, top, Near, Far);
    m_Entity.addComponent<ComponentLogic2>();
    m_Entity.addComponent<ComponentBody>();

    auto& cam   = *m_Entity.getComponent<ComponentCamera>();
    auto& logic = *m_Entity.getComponent<ComponentLogic2>();
    auto& body  = *m_Entity.getComponent<ComponentBody>();

    cam.lookAt(glm_vec3(0.0), glm_vec3(0.0) + body.forward(), body.up());
    logic.setUserPointer(this);
}
Camera::~Camera(){ 
}
void Camera::setProjectionMatrix(const glm::mat4& projectonMatrix) {
    m_Entity.getComponent<ComponentCamera>()->setProjectionMatrix(projectonMatrix);
}
void Camera::setViewMatrix(const glm::mat4& viewMatrix) {
    m_Entity.getComponent<ComponentCamera>()->setViewMatrix(viewMatrix);
}
void Camera::lookAt(const glm_vec3& eye, const glm_vec3& center, const glm_vec3& up) {
    m_Entity.getComponent<ComponentCamera>()->lookAt(eye, center, up);
}
const glm_vec3 Camera::getPosition() const {
    return m_Entity.getComponent<ComponentBody>()->position(); 
}
const glm::quat Camera::getOrientation() const {
    return glm::conjugate(glm::quat_cast(m_Entity.getComponent<ComponentCamera>()->getView())); 
}
const float Camera::getAngle() const {
    return m_Entity.getComponent<ComponentCamera>()->m_Angle; 
}
const float Camera::getAspect() const {
    return m_Entity.getComponent<ComponentCamera>()->m_AspectRatio; 
}
const float Camera::getNear() const {
    return m_Entity.getComponent<ComponentCamera>()->m_NearPlane; 
}
const float Camera::getFar() const {
    return m_Entity.getComponent<ComponentCamera>()->m_FarPlane; 
}
void Camera::setAngle(const float Angle) const {
    m_Entity.getComponent<ComponentCamera>()->setAngle(Angle); 
}
void Camera::setAspect(const float Aspect) const {
    m_Entity.getComponent<ComponentCamera>()->setAspect(Aspect); 
}
void Camera::setNear(const float Near_) const {
    m_Entity.getComponent<ComponentCamera>()->setNear(Near_);
}
void Camera::setFar(const float Far_) const {
    m_Entity.getComponent<ComponentCamera>()->setFar(Far_);
}
const glm::mat4 Camera::getViewProjectionInverse() const {
    return m_Entity.getComponent<ComponentCamera>()->getViewProjectionInverse(); 
}
const glm::mat4 Camera::getProjection() const {
    return m_Entity.getComponent<ComponentCamera>()->getProjection(); 
}
const glm::mat4 Camera::getView() const {
    return m_Entity.getComponent<ComponentCamera>()->getView(); 
}
const glm::mat4 Camera::getViewInverse() const {
    return m_Entity.getComponent<ComponentCamera>()->getViewInverse(); 
}
const glm::mat4 Camera::getProjectionInverse() const {
    return m_Entity.getComponent<ComponentCamera>()->getProjectionInverse(); 
}
const glm::mat4 Camera::getViewProjection() const {
    return m_Entity.getComponent<ComponentCamera>()->getViewProjection();
}
const glm::vec3 Camera::getViewVector() const {
    return m_Entity.getComponent<ComponentCamera>()->getViewVector(); 
}
const glm_vec3 Camera::forward() const {
    return m_Entity.getComponent<ComponentCamera>()->forward(); 
}
const glm_vec3 Camera::right() const {
    return m_Entity.getComponent<ComponentCamera>()->right(); 
}
const glm_vec3 Camera::up() const {
    return m_Entity.getComponent<ComponentCamera>()->up(); 
}
const decimal Camera::getDistance(const Entity& e) const {
    auto& b = *e.getComponent<ComponentBody>();
    return glm::distance(b.position(),getPosition());
}
const decimal Camera::getDistance(const glm_vec3& objPos) const {
    return glm::distance(objPos,getPosition()); 
}
const decimal Camera::getDistanceSquared(const Entity& e) const {
    auto& b = *e.getComponent<ComponentBody>();
    return glm::distance2(b.position(), getPosition());
}
const decimal Camera::getDistanceSquared(const glm_vec3& objPos) const {
    return glm::distance2(objPos, getPosition()); 
}
const decimal Camera::getDistanceSquared(const Entity& e, const glm_vec3& this_pos) const {
    auto& b = *e.getComponent<ComponentBody>();
    return glm::distance2(b.position(), this_pos);
}
const decimal Camera::getDistanceSquared(const glm_vec3& objPos, const glm_vec3& this_pos) const {
    return glm::distance2(objPos, this_pos);
}
const unsigned int Camera::sphereIntersectTest(const glm_vec3& pos, const float radius) const {
    return m_Entity.getComponent<ComponentCamera>()->sphereIntersectTest(pos,radius); 
}
const unsigned int Camera::pointIntersectTest(const glm_vec3& pos) const {
    return m_Entity.getComponent<ComponentCamera>()->pointIntersectTest(pos); 
}
const bool Camera::rayIntersectSphere(const Entity& entity) const {
    auto* body        = entity.getComponent<ComponentBody>();
    auto* model       = entity.getComponent<ComponentModel>();
    auto& cameraBody  = *m_Entity.getComponent<ComponentBody>();
    auto& thisCamera  = *m_Entity.getComponent<ComponentCamera>();
    auto radius       = 0.0f;
    if(model) 
        radius = model->radius();
    if (!body)
        return false;
    return Math::rayIntersectSphere(body->position(), radius, cameraBody.position(), thisCamera.getViewVector());
}