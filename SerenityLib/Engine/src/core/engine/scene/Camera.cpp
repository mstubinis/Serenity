#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/resources/Engine_Resources.h>

Camera::Camera(float angle, float aspectRatio, float Near, float Far, Scene* scene) 
    : Entity{ *scene }
{
    if (!scene) {
        scene = Engine::Resources::getCurrentScene();
    }
    addComponent<ComponentCamera>(angle, aspectRatio, Near, Far);
    addComponent<ComponentLogic1>();
    addComponent<ComponentBody>();

    auto* cam   = getComponent<ComponentCamera>();
    auto* logic = getComponent<ComponentLogic1>();
    auto* body  = getComponent<ComponentBody>();

    cam->lookAt(glm_vec3(0.0), glm_vec3(0.0) + body->forward(), body->up());
    logic->setUserPointer(this);
}
Camera::Camera(float left, float right, float bottom, float top, float Near, float Far, Scene* scene) 
    : Entity{ *scene }
{
    if (!scene) {
        scene = Engine::Resources::getCurrentScene();
    }
    addComponent<ComponentCamera>(left, right, bottom, top, Near, Far);
    addComponent<ComponentLogic1>();
    addComponent<ComponentBody>();

    auto* cam   = getComponent<ComponentCamera>();
    auto* logic = getComponent<ComponentLogic1>();
    auto* body  = getComponent<ComponentBody>();

    cam->lookAt(glm_vec3(0.0), glm_vec3(0.0) + body->forward(), body->up());
    logic->setUserPointer(this);
}
Camera::~Camera() { 
}
void Camera::setProjectionMatrix(const glm::mat4& projectonMatrix) noexcept {
    getComponent<ComponentCamera>()->setProjectionMatrix(projectonMatrix);
}
void Camera::setViewMatrix(const glm::mat4& viewMatrix) noexcept {
    getComponent<ComponentCamera>()->setViewMatrix(viewMatrix);
}
void Camera::lookAt(const glm_vec3& eye, const glm_vec3& center, const glm_vec3& up) noexcept {
    getComponent<ComponentCamera>()->lookAt(eye, center, up);
}
glm_vec3 Camera::getPosition() const noexcept {
    return getComponent<ComponentBody>()->getPosition();
}
glm::quat Camera::getOrientation() const noexcept {
    return glm::conjugate(glm::quat_cast(getComponent<ComponentCamera>()->getView())); 
}
float Camera::getAngle() const noexcept {
    return getComponent<ComponentCamera>()->m_Angle; 
}
float Camera::getAspect() const noexcept {
    return getComponent<ComponentCamera>()->m_AspectRatio; 
}
float Camera::getNear() const noexcept {
    return getComponent<ComponentCamera>()->m_NearPlane; 
}
float Camera::getFar() const noexcept {
    return getComponent<ComponentCamera>()->m_FarPlane; 
}
void Camera::setAngle(float Angle) noexcept {
    getComponent<ComponentCamera>()->setAngle(Angle); 
}
void Camera::setAspect(float Aspect) noexcept {
    getComponent<ComponentCamera>()->setAspect(Aspect); 
}
void Camera::setNear(float inNear) noexcept {
    getComponent<ComponentCamera>()->setNear(inNear);
}
void Camera::setFar(float inFar) noexcept {
    getComponent<ComponentCamera>()->setFar(inFar);
}
glm::mat4 Camera::getViewProjectionInverse() const noexcept {
    return getComponent<ComponentCamera>()->getViewProjectionInverse(); 
}
glm::mat4 Camera::getProjection() const noexcept {
    return getComponent<ComponentCamera>()->getProjection(); 
}
glm::mat4 Camera::getView() const noexcept {
    return getComponent<ComponentCamera>()->getView(); 
}
glm::mat4 Camera::getViewInverse() const noexcept {
    return getComponent<ComponentCamera>()->getViewInverse(); 
}
glm::mat4 Camera::getProjectionInverse() const noexcept {
    return getComponent<ComponentCamera>()->getProjectionInverse(); 
}
glm::mat4 Camera::getViewProjection() const noexcept {
    return getComponent<ComponentCamera>()->getViewProjection();
}
glm::vec3 Camera::getViewVector() const noexcept {
    return getComponent<ComponentCamera>()->getViewVector(); 
}
glm_vec3 Camera::forward() const noexcept {
    return getComponent<ComponentCamera>()->forward(); 
}
glm_vec3 Camera::right() const noexcept {
    return getComponent<ComponentCamera>()->right(); 
}
glm_vec3 Camera::up() const noexcept {
    return getComponent<ComponentCamera>()->up(); 
}
decimal Camera::getDistance(Entity otherEntity) const noexcept {
    ComponentBody* otherEntityBody = otherEntity.getComponent<ComponentBody>();
    return glm::distance(otherEntityBody->getPosition(), getPosition());
}
decimal Camera::getDistance(const glm_vec3& otherPosition) const noexcept {
    return glm::distance(otherPosition, getPosition());
}
decimal Camera::getDistanceSquared(Entity otherEntity) const noexcept {
    ComponentBody* otherEntityBody = otherEntity.getComponent<ComponentBody>();
    return glm::distance2(otherEntityBody->getPosition(), getPosition());
}
decimal Camera::getDistanceSquared(const glm_vec3& otherPosition) const noexcept {
    return glm::distance2(otherPosition, getPosition());
}
decimal Camera::getDistanceSquared(Entity otherEntity, const glm_vec3& thisPosition) const noexcept {
    ComponentBody* otherEntityBody = otherEntity.getComponent<ComponentBody>();
    return glm::distance2(otherEntityBody->getPosition(), thisPosition);
}
decimal Camera::getDistanceSquared(const glm_vec3& otherPosition, const glm_vec3& thisPosition) const noexcept {
    return glm::distance2(otherPosition, thisPosition);
}
unsigned int Camera::sphereIntersectTest(const glm_vec3& otherPosition, float otherRadius) const noexcept {
    return getComponent<ComponentCamera>()->sphereIntersectTest(otherPosition, otherRadius);
}
unsigned int Camera::pointIntersectTest(const glm_vec3& otherPosition) const noexcept {
    return getComponent<ComponentCamera>()->pointIntersectTest(otherPosition);
}
bool Camera::rayIntersectSphere(Entity entity) const noexcept {
    ComponentBody* entityBody   = entity.getComponent<ComponentBody>();
    ComponentModel* entityModel = entity.getComponent<ComponentModel>();
    float entityRadius          = 0.0f;
    if (entityModel) {
        entityRadius            = entityModel->radius();
    }
    if (!entityBody) {
        return false;
    }
    return Engine::Math::rayIntersectSphere(entityBody->getPosition(), entityRadius, getPosition(), getViewVector());
}