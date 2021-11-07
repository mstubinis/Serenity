
#include <serenity/scene/Camera.h>
#include <serenity/scene/Scene.h>
#include <serenity/math/Engine_Math.h>
#include <serenity/resources/Engine_Resources.h>
#include <glm/gtx/euler_angles.hpp>

namespace {
    CameraLogicFunctionPtr CAMERA_DEFAULT_MOUSELOOK_FUNCTION = [](const CameraLogicComponent* component, const float dt) {
        auto& camera           = *static_cast<Camera*>(component->getUserPointer());
        auto cameraComponent   = camera.getComponent<ComponentCamera>();
        auto transform         = camera.getComponent<ComponentTransform>();
        const auto& mouse      = Engine::getMouseDifference();

        const auto rotSpeedX   = (mouse.x) * 0.4f * dt;
        const auto rotSpeedY   = -(mouse.y) * 0.4f * dt;
        auto transSpeed        = static_cast<decimal>(5.0f * dt);

        if (Engine::isKeyDown(KeyboardKey::LeftShift) || Engine::isKeyDown(KeyboardKey::RightShift)) {
            transSpeed *= static_cast<decimal>(6.0);
        }

        if (Engine::isKeyDown(KeyboardKey::W)) {
            transform->translate(0, 0, -transSpeed);
        } else if (Engine::isKeyDown(KeyboardKey::S)) {
            transform->translate(0, 0, transSpeed);
        }
        if (Engine::isKeyDown(KeyboardKey::A)) {
            transform->translate(-transSpeed, 0, 0);
        } else if (Engine::isKeyDown(KeyboardKey::D)) {
            transform->translate(transSpeed, 0, 0);
        }


        if (Engine::isMouseButtonDown(MouseButton::Left)) {
            if (std::abs(mouse.x) > 0.005f) {
                camera.m_UserFloats.x += glm::degrees(rotSpeedX);
            }
            if (std::abs(mouse.y) > 0.005f) {
                camera.m_UserFloats.y += glm::degrees(rotSpeedY);
            }
        }
        if (camera.m_UserFloats.y > 89.0f) {
            camera.m_UserFloats.y = 89.0f;
        } else if (camera.m_UserFloats.y < -89.0f) {
            camera.m_UserFloats.y = -89.0f;
        }
        const glm::mat4 rotMatrix   = glm::eulerAngleXYZ(glm::radians(camera.m_UserFloats.y), glm::radians(camera.m_UserFloats.x), glm::radians(0.0f));
        const glm::vec3 fwd         = -glm::vec3(rotMatrix[0][2], rotMatrix[1][2], rotMatrix[2][2]);
        const glm::mat4 viewMatrix  = glm::lookAt(transform->getWorldPosition(), transform->getWorldPosition() + fwd, glm_vec3{ 0.0f, 1.0f, 0.0f });
        const glm::quat orientation = glm::conjugate(glm::toQuat(viewMatrix));

        transform->setRotation(orientation);
        cameraComponent->setViewMatrix(viewMatrix);
    };
    CameraLogicFunctionPtr CAMERA_DEFAULT_MOUSELOOK_NO_GIMBLE_LOCK_FUNCTION = [](const CameraLogicComponent* component, const float dt) {
        auto& camera           = *static_cast<Camera*>(component->getUserPointer());
        auto cameraComponent   = camera.getComponent<ComponentCamera>();
        auto transform         = camera.getComponent<ComponentTransform>();
        const auto& mouse      = Engine::getMouseDifference();

        const auto rotSpeedX   = (mouse.x) * 0.4f * dt;
        const auto rotSpeedY   = -(mouse.y) * 0.4f * dt;
        auto transSpeed        = static_cast<decimal>(5.0f * dt);
        const auto transSpeed2 = static_cast<decimal>(5.0f * dt);

        if (Engine::isKeyDown(KeyboardKey::LeftShift) || Engine::isKeyDown(KeyboardKey::RightShift)) {
            transSpeed *= static_cast<decimal>(6.0);
        }

        if (Engine::isKeyDown(KeyboardKey::W)) {
            transform->translate(0, 0, -transSpeed);
        } else if (Engine::isKeyDown(KeyboardKey::S)) {
            transform->translate(0, 0, transSpeed);
        }
        if (Engine::isKeyDown(KeyboardKey::A)) {
            transform->translate(-transSpeed, 0, 0);
        } else if (Engine::isKeyDown(KeyboardKey::D)) {
            transform->translate(transSpeed, 0, 0);
        }

        if (Engine::isMouseButtonDown(MouseButton::Left)) {
            if (std::abs(mouse.x) > 0.01f) {
                transform->rotateYaw(rotSpeedX);
            }
            if (std::abs(mouse.y) > 0.01f) {
                transform->rotatePitch(rotSpeedY);
            }
        }
        if (Engine::isKeyDown(KeyboardKey::Q)) {
            transform->rotateRoll(static_cast<float>(transSpeed2) * 0.5f);
        } else if (Engine::isKeyDown(KeyboardKey::E)) {
            transform->rotateRoll(static_cast<float>(-transSpeed2) * 0.5f);
        }
        cameraComponent->lookAt(transform->getWorldPosition(), transform->getWorldPosition() + transform->getForward(), transform->getUp());
    };
}

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

    camera->lookAt(glm::vec3{ 0.0f }, glm::vec3{ 0.0f } + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3{ 0.0f, 1.0f, 0.0f });
    logic->setUserPointer(this);
}
Camera::~Camera() { 
}
void Camera::setUpdateFunction(CameraLogicType logicType) noexcept {
    switch (logicType) {
        case CameraLogicType::FPS_Mouselook: {
            setUpdateFunction(CAMERA_DEFAULT_MOUSELOOK_FUNCTION);
            break;
        } case CameraLogicType::SpaceSimulator: {
            setUpdateFunction(CAMERA_DEFAULT_MOUSELOOK_NO_GIMBLE_LOCK_FUNCTION);
            break;
        }
    }
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
float Camera::getAspectRatio() const noexcept {
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
void Camera::setAspectRatio(float Aspect) noexcept {
    getComponent<ComponentCamera>()->setAspectRatio(Aspect); 
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
glm::vec3 Camera::getRight() const noexcept {
    return getComponent<ComponentCamera>()->getRight(); 
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
bool Camera::rayIntersectSphere(Entity entity) const noexcept {
    auto entityTransform = entity.getComponent<ComponentTransform>();
    auto entityModel     = entity.getComponent<ComponentModel>();
    float entityRadius   = entityModel ? entityModel->getRadius() : 0.0f;
    if (!entityTransform) {
        return false;
    }
    return Engine::Math::rayIntersectSphere(entityTransform->getPosition(), entityRadius, getPosition(), getViewVector());
}
bool Camera::rayIntersectSphere(const glm::vec3& position, float radius) const noexcept {
    return Engine::Math::rayIntersectSphere(position, radius, getPosition(), getViewVector());
}