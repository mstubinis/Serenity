#include <core/engine/scene/Camera.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/resources/Engine_Resources.h>
#include <glm/gtx/norm.hpp>

using namespace Engine;
using namespace boost;
using namespace std;

Camera::Camera(const float angle, const float aspectRatio, const float Near, const float Far, Scene* scene) : Entity(*scene){//create a perspective camera
    if (!scene) {
        scene = Resources::getCurrentScene();
    }
    addComponent<ComponentCamera>(angle, aspectRatio, Near, Far);
    addComponent<ComponentLogic1>();
    addComponent<ComponentBody>();

    auto& cam   = *getComponent<ComponentCamera>();
    auto& logic = *getComponent<ComponentLogic1>();
    auto& body  = *getComponent<ComponentBody>();

    cam.lookAt(glm_vec3(0.0), glm_vec3(0.0) + body.forward(), body.up());
    logic.setUserPointer(this);
}
Camera::Camera(const float left, const float right, const float bottom, const float top, const float Near, const float Far, Scene* scene) : Entity(*scene){//create an orthographic camera
    if (!scene) {
        scene = Resources::getCurrentScene();
    }
    addComponent<ComponentCamera>(left, right, bottom, top, Near, Far);
    addComponent<ComponentLogic1>();
    addComponent<ComponentBody>();

    auto& cam   = *getComponent<ComponentCamera>();
    auto& logic = *getComponent<ComponentLogic1>();
    auto& body  = *getComponent<ComponentBody>();

    cam.lookAt(glm_vec3(0.0), glm_vec3(0.0) + body.forward(), body.up());
    logic.setUserPointer(this);
}
Camera::~Camera(){ 
}
void Camera::setProjectionMatrix(const glm::mat4& projectonMatrix) {
    getComponent<ComponentCamera>()->setProjectionMatrix(projectonMatrix);
}
void Camera::setViewMatrix(const glm::mat4& viewMatrix) {
    getComponent<ComponentCamera>()->setViewMatrix(viewMatrix);
}
void Camera::lookAt(const glm_vec3& eye, const glm_vec3& center, const glm_vec3& up) {
    getComponent<ComponentCamera>()->lookAt(eye, center, up);
}
const glm_vec3 Camera::getPosition() const {
    return getComponent<ComponentBody>()->position(); 
}
const glm::quat Camera::getOrientation() const {
    return glm::conjugate(glm::quat_cast(getComponent<ComponentCamera>()->getView())); 
}
const float Camera::getAngle() const {
    return getComponent<ComponentCamera>()->m_Angle; 
}
const float Camera::getAspect() const {
    return getComponent<ComponentCamera>()->m_AspectRatio; 
}
const float Camera::getNear() const {
    return getComponent<ComponentCamera>()->m_NearPlane; 
}
const float Camera::getFar() const {
    return getComponent<ComponentCamera>()->m_FarPlane; 
}
void Camera::setAngle(const float Angle) const {
    getComponent<ComponentCamera>()->setAngle(Angle); 
}
void Camera::setAspect(const float Aspect) const {
    getComponent<ComponentCamera>()->setAspect(Aspect); 
}
void Camera::setNear(const float Near_) const {
    getComponent<ComponentCamera>()->setNear(Near_);
}
void Camera::setFar(const float Far_) const {
    getComponent<ComponentCamera>()->setFar(Far_);
}
const glm::mat4 Camera::getViewProjectionInverse() const {
    return getComponent<ComponentCamera>()->getViewProjectionInverse(); 
}
const glm::mat4 Camera::getProjection() const {
    return getComponent<ComponentCamera>()->getProjection(); 
}
const glm::mat4 Camera::getView() const {
    return getComponent<ComponentCamera>()->getView(); 
}
const glm::mat4 Camera::getViewInverse() const {
    return getComponent<ComponentCamera>()->getViewInverse(); 
}
const glm::mat4 Camera::getProjectionInverse() const {
    return getComponent<ComponentCamera>()->getProjectionInverse(); 
}
const glm::mat4 Camera::getViewProjection() const {
    return getComponent<ComponentCamera>()->getViewProjection();
}
const glm::vec3 Camera::getViewVector() const {
    return getComponent<ComponentCamera>()->getViewVector(); 
}
const glm_vec3 Camera::forward() const {
    return getComponent<ComponentCamera>()->forward(); 
}
const glm_vec3 Camera::right() const {
    return getComponent<ComponentCamera>()->right(); 
}
const glm_vec3 Camera::up() const {
    return getComponent<ComponentCamera>()->up(); 
}
const decimal Camera::getDistance(const Entity& otherEntity) const {
    auto& otherEntityBody = *otherEntity.getComponent<ComponentBody>();
    return glm::distance(otherEntityBody.position(), getPosition());
}
const decimal Camera::getDistance(const glm_vec3& otherPosition) const {
    return glm::distance(otherPosition, getPosition());
}
const decimal Camera::getDistanceSquared(const Entity& otherEntity) const {
    auto& otherEntityBody = *otherEntity.getComponent<ComponentBody>();
    return glm::distance2(otherEntityBody.position(), getPosition());
}
const decimal Camera::getDistanceSquared(const glm_vec3& otherPosition) const {
    return glm::distance2(otherPosition, getPosition());
}
const decimal Camera::getDistanceSquared(const Entity& otherEntity, const glm_vec3& thisPosition) const {
    auto& otherEntityBody = *otherEntity.getComponent<ComponentBody>();
    return glm::distance2(otherEntityBody.position(), thisPosition);
}
const decimal Camera::getDistanceSquared(const glm_vec3& otherPosition, const glm_vec3& thisPosition) const {
    return glm::distance2(otherPosition, thisPosition);
}
const unsigned int Camera::sphereIntersectTest(const glm_vec3& otherPosition, const float otherRadius) const {
    return getComponent<ComponentCamera>()->sphereIntersectTest(otherPosition, otherRadius);
}
const unsigned int Camera::pointIntersectTest(const glm_vec3& otherPosition) const {
    return getComponent<ComponentCamera>()->pointIntersectTest(otherPosition);
}
const bool Camera::rayIntersectSphere(const Entity& entity) const {
    auto* entityBody     = entity.getComponent<ComponentBody>();
    auto* entityModel    = entity.getComponent<ComponentModel>();
    auto& thisBody       = *getComponent<ComponentBody>();
    float entityRadius   = 0.0f;
    if(entityModel)
        entityRadius     = entityModel->radius();
    if (!entityBody)
        return false;
    return Math::rayIntersectSphere(entityBody->position(), entityRadius, thisBody.position(), getViewVector());
}