
#include <serenity/scene/Camera.h>
#include <serenity/scene/Scene.h>
#include <serenity/math/Engine_Math.h>
#include <serenity/resources/Engine_Resources.h>

CameraLogicFunctionPtr CAMERA_DEFAULT_MOUSELOOK_FUNCTION = [](const CameraLogicComponent* component, const float dt) {
    auto& camera          = *static_cast<Camera*>(component->getUserPointer());
    auto cameraComponent  = camera.getComponent<ComponentCamera>();
    auto transform        = camera.getComponent<ComponentTransform>();
};
CameraLogicFunctionPtr CAMERA_DEFAULT_MOUSELOOK_NO_GIMBLE_LOCK_FUNCTION = [](const CameraLogicComponent* component, const float dt) {
    auto& camera          = *static_cast<Camera*>(component->getUserPointer());
    auto cameraComponent  = camera.getComponent<ComponentCamera>();
    auto transform        = camera.getComponent<ComponentTransform>();
    const auto& mouse     = Engine::getMouseDifference();

    const auto rotSpeedX   = (mouse.x * dt) * 0.4f;
    const auto rotSpeedY   = -(mouse.y * dt) * 0.4f;
    auto transSpeed        = static_cast<decimal>(5.0f * dt);
    const auto transSpeed2 = static_cast<decimal>(5.0f * dt);

    if (Engine::isKeyDown(KeyboardKey::LeftShift) || Engine::isKeyDown(KeyboardKey::RightShift)) {
        transSpeed *= static_cast<decimal>(6.0);
    }

    if (Engine::isMouseButtonDown(MouseButton::Left)) {
        if (std::abs(mouse.x) > 0.05f) {
            transform->rotate(0.0f, rotSpeedX, 0.0f);
        }
        if (std::abs(mouse.y) > 0.05f) {
            transform->rotate(rotSpeedY, 0.0f, 0.0f);
        }
    }
    if (Engine::isKeyDown(KeyboardKey::W)) {
        transform->translate(0, 0, -transSpeed);
    }else if (Engine::isKeyDown(KeyboardKey::S)) {
        transform->translate(0, 0, transSpeed);
    }
    if (Engine::isKeyDown(KeyboardKey::A)) {
        transform->translate(-transSpeed, 0, 0);
    }else if (Engine::isKeyDown(KeyboardKey::D)) {
        transform->translate(transSpeed, 0, 0);
    }

    if (Engine::isKeyDown(KeyboardKey::Q)) {
        transform->rotate(0.0f, 0.0f, static_cast<float>(transSpeed2) * 0.5f);
    }else if (Engine::isKeyDown(KeyboardKey::E)) {
        transform->rotate(0.0f, 0.0f, static_cast<float>(-transSpeed2) * 0.5f);
    }
    cameraComponent->lookAt(transform->getWorldPosition(), transform->getWorldPosition() + transform->getForward(), transform->getUp());
};


Camera::Camera(Scene* scene, float angle, float aspectRatio, float Near, float Far)
    : Entity{ (!scene) ? *Engine::Resources::getCurrentScene() : *scene }
{
    if (!scene) {
        scene = Engine::Resources::getCurrentScene();
    }
    addComponent<ComponentCamera>(angle, aspectRatio, Near, Far);
    addComponent<CameraLogicComponent>();
    addComponent<ComponentTransform>();

    auto camera    = getComponent<ComponentCamera>();
    auto logic     = getComponent<CameraLogicComponent>();
    auto transform = getComponent<ComponentTransform>();

    camera->lookAt(glm::vec3{ 0.0f }, glm::vec3{ 0.0f } + transform->getForward(), transform->getUp());
    logic->setUserPointer(this);
}
Camera::Camera(Scene* scene, float left, float right, float bottom, float top, float Near, float Far)
    : Entity{ (!scene) ? *Engine::Resources::getCurrentScene() : *scene }
{
    if (!scene) {
        scene = Engine::Resources::getCurrentScene();
    }
    addComponent<ComponentCamera>(left, right, bottom, top, Near, Far);
    addComponent<CameraLogicComponent>();
    addComponent<ComponentTransform>();

    auto camera     = getComponent<ComponentCamera>();
    auto logic      = getComponent<CameraLogicComponent>();
    auto transform  = getComponent<ComponentTransform>();

    camera->lookAt(glm::vec3{ 0.0f }, glm::vec3{ 0.0f } + camera->getForward(), camera->getUp());
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
    return getComponent<ComponentTransform>()->getPosition();
}
glm::quat Camera::getRotation() const noexcept {
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
glm::vec3 Camera::getForward() const noexcept {
    return getComponent<ComponentCamera>()->getForward(); 
}
glm::vec3 Camera::getRight() const noexcept {
    return getComponent<ComponentCamera>()->getRight(); 
}
glm::vec3 Camera::getUp() const noexcept {
    return getComponent<ComponentCamera>()->getUp(); 
}
decimal Camera::getDistance(Entity otherEntity) const noexcept {
    auto otherEntityBody = otherEntity.getComponent<ComponentTransform>();
    return glm::distance(otherEntityBody->getPosition(), getPosition());
}
decimal Camera::getDistance(const glm_vec3& otherPosition) const noexcept {
    return glm::distance(otherPosition, getPosition());
}
decimal Camera::getDistanceSquared(Entity otherEntity) const noexcept {
    auto otherEntityBody = otherEntity.getComponent<ComponentTransform>();
    return glm::distance2(otherEntityBody->getPosition(), getPosition());
}
decimal Camera::getDistanceSquared(const glm_vec3& otherPosition) const noexcept {
    return glm::distance2(otherPosition, getPosition());
}
decimal Camera::getDistanceSquared(Entity otherEntity, const glm_vec3& thisPosition) const noexcept {
    auto otherEntityBody = otherEntity.getComponent<ComponentTransform>();
    return glm::distance2(otherEntityBody->getPosition(), thisPosition);
}
decimal Camera::getDistanceSquared(const glm_vec3& otherPosition, const glm_vec3& thisPosition) const noexcept {
    return glm::distance2(otherPosition, thisPosition);
}
uint32_t Camera::sphereIntersectTest(const glm_vec3& otherWorldPosition, float otherRadius) const noexcept {
    return getComponent<ComponentCamera>()->sphereIntersectTest(otherWorldPosition, otherRadius);
}
uint32_t Camera::pointIntersectTest(const glm_vec3& otherWorldPosition) const noexcept {
    return getComponent<ComponentCamera>()->pointIntersectTest(otherWorldPosition);
}
bool Camera::rayIntersectSphere(Entity entity) const noexcept {
    auto entityTransform = entity.getComponent<ComponentTransform>();
    auto entityModel     = entity.getComponent<ComponentModel>();
    float entityRadius   = 0.0f;
    if (entityModel) {
        entityRadius     = entityModel->getRadius();
    }
    if (!entityTransform) {
        return false;
    }
    return Engine::Math::rayIntersectSphere(entityTransform->getPosition(), entityRadius, getPosition(), getViewVector());
}