#include <core/engine/utils/PrecompiledHeader.h>
#include <ecs/ComponentCamera.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/threading/ThreadingModule.h>
#include <core/engine/system/Engine.h>
#include <core/engine/scene/Camera.h>

using namespace Engine;
using namespace Engine::priv;

void Engine::priv::ComponentCamera_Functions::RebuildProjectionMatrix(ComponentCamera& componentCamera) noexcept {
    if (componentCamera.m_Type == ComponentCamera::CameraType::Perspective) {
        componentCamera.m_ProjectionMatrix = glm::perspective(componentCamera.m_Angle, componentCamera.m_AspectRatio, componentCamera.m_NearPlane, componentCamera.m_FarPlane);
    }else{
        componentCamera.m_ProjectionMatrix = glm::ortho(componentCamera.m_Left, componentCamera.m_Right, componentCamera.m_Bottom, componentCamera.m_Top, componentCamera.m_NearPlane, componentCamera.m_FarPlane);
    }
}
glm::mat4 Engine::priv::ComponentCamera_Functions::GetViewNoTranslation(const Camera& camera) noexcept {
    return camera.getComponent<ComponentCamera>()->m_ViewMatrixNoTranslation;
}
glm::mat4 Engine::priv::ComponentCamera_Functions::GetViewInverseNoTranslation(const Camera& camera) noexcept {
    return glm::inverse(camera.getComponent<ComponentCamera>()->m_ViewMatrixNoTranslation);
}
glm::mat4 Engine::priv::ComponentCamera_Functions::GetViewProjectionNoTranslation(const Camera& camera) noexcept {
    const auto& componentCamera = *camera.getComponent<ComponentCamera>();
    return componentCamera.m_ProjectionMatrix * componentCamera.m_ViewMatrixNoTranslation;
}
glm::mat4 Engine::priv::ComponentCamera_Functions::GetViewProjectionInverseNoTranslation(const Camera& camera) noexcept {
    const auto& componentCamera = *camera.getComponent<ComponentCamera>();
    return glm::inverse(componentCamera.m_ProjectionMatrix * componentCamera.m_ViewMatrixNoTranslation);
}
glm::vec3 Engine::priv::ComponentCamera_Functions::GetViewVectorNoTranslation(const Camera& camera) noexcept {
    const auto& viewMatrixNoTranslation = camera.getComponent<ComponentCamera>()->m_ViewMatrixNoTranslation;
    return glm::vec3(viewMatrixNoTranslation[0][2], viewMatrixNoTranslation[1][2], viewMatrixNoTranslation[2][2]);
}


#pragma region Component

ComponentCamera::ComponentCamera(Entity entity, float angleDegrees, float aspectRatio, float nearPlane, float farPlane) 
    : m_Owner(entity)
    , m_Angle(glm::radians(angleDegrees))
    , m_AspectRatio(aspectRatio)
    , m_NearPlane(nearPlane)
    , m_FarPlane(farPlane)
    , m_Type(CameraType::Perspective)
{
    setProjectionMatrix(glm::perspective(m_Angle, m_AspectRatio, m_NearPlane, m_FarPlane));
    setViewMatrix(glm::lookAt(m_Eye, m_Forward, m_Up));
}
ComponentCamera::ComponentCamera(Entity entity, float left, float right, float bottom, float top, float nearPlane, float farPlane) 
    : m_Owner(entity)
    , m_Left(left)
    , m_Right(right)
    , m_Bottom(bottom)
    , m_Top(top)
    , m_NearPlane(nearPlane)
    , m_FarPlane(farPlane)
    , m_Type(CameraType::Orthographic)
{
    setProjectionMatrix(glm::ortho(m_Left, m_Right, m_Bottom, m_Top, m_NearPlane, m_FarPlane));
    setViewMatrix(glm::lookAt(m_Eye, m_Forward, m_Up));
}
ComponentCamera::ComponentCamera(ComponentCamera&& other) noexcept 
    : m_Owner(std::move(other.m_Owner))
    , m_Eye(std::move(other.m_Eye))
    , m_Up(std::move(other.m_Up))
    , m_Forward(std::move(other.m_Forward))
    , m_Angle(std::move(other.m_Angle))
    , m_AspectRatio(std::move(other.m_AspectRatio))
    , m_NearPlane(std::move(other.m_NearPlane))
    , m_FarPlane(std::move(other.m_FarPlane))
    , m_Bottom(std::move(other.m_Bottom))
    , m_Top(std::move(other.m_Top))
    , m_ProjectionMatrix(std::move(other.m_ProjectionMatrix))
    , m_ViewMatrix(std::move(other.m_ViewMatrix))
    , m_ViewMatrixNoTranslation(std::move(other.m_ViewMatrixNoTranslation))
    , m_FrustumPlanes(std::move(other.m_FrustumPlanes))
    , m_Type(std::move(other.m_Type))
{}
ComponentCamera& ComponentCamera::operator=(ComponentCamera&& other) noexcept {
    if (&other != this) {
        m_Owner                   = std::move(other.m_Owner);
        m_Eye                     = std::move(other.m_Eye);
        m_Up                      = std::move(other.m_Up);
        m_Forward                 = std::move(other.m_Forward);
        m_Angle                   = std::move(other.m_Angle);
        m_AspectRatio             = std::move(other.m_AspectRatio);
        m_NearPlane               = std::move(other.m_NearPlane);
        m_FarPlane                = std::move(other.m_FarPlane);
        m_Bottom                  = std::move(other.m_Bottom);
        m_Top                     = std::move(other.m_Top);
        m_ProjectionMatrix        = std::move(other.m_ProjectionMatrix);
        m_ViewMatrix              = std::move(other.m_ViewMatrix);
        m_ViewMatrixNoTranslation = std::move(other.m_ViewMatrixNoTranslation);
        m_FrustumPlanes           = std::move(other.m_FrustumPlanes);
        m_Type                    = std::move(other.m_Type);
    }
    return *this;
}

void ComponentCamera::resize(unsigned int width, unsigned int height) noexcept {
    if (m_Type == CameraType::Perspective) {
        m_AspectRatio = width / (float)height;
    }
    priv::ComponentCamera_Functions::RebuildProjectionMatrix(*this);
}
unsigned int ComponentCamera::pointIntersectTest(const glm_vec3& position) const noexcept {
    auto zero = (decimal)0.0;
    for (unsigned int i = 0; i < m_FrustumPlanes.size(); ++i) {
        auto d = m_FrustumPlanes[i].x * position.x + m_FrustumPlanes[i].y * position.y + m_FrustumPlanes[i].z * position.z + m_FrustumPlanes[i].w;
        if (d > zero) {
            return 0; //outside
        }
    }
    return 1; //inside
}
unsigned int ComponentCamera::sphereIntersectTest(const glm_vec3& position, float radius) const noexcept {
    unsigned int res = 1; //inside the viewing frustum
    auto zero        = (decimal)0.0;
    auto two         = (decimal)2.0;
    if (radius <= zero) {
        return 0;
    }
    for (size_t i = 0; i < m_FrustumPlanes.size(); ++i) {
        auto d = m_FrustumPlanes[i].x * position.x + m_FrustumPlanes[i].y * position.y + m_FrustumPlanes[i].z * position.z + m_FrustumPlanes[i].w;
        if (d > radius * two) {
            return 0; //outside the viewing frustrum
        }else if (d > zero) {
            res = 2; //intersecting the viewing plane
        }
    }
    return res;
}
void ComponentCamera::setViewMatrix(const glm::mat4& viewMatrix) noexcept {
    m_ViewMatrix                    = viewMatrix;
    m_ViewMatrixNoTranslation       = viewMatrix;
    m_ViewMatrixNoTranslation[3][0] = 0.0001f;
    m_ViewMatrixNoTranslation[3][1] = 0.0001f;
    m_ViewMatrixNoTranslation[3][2] = 0.0001f;
}
void ComponentCamera::setViewMatrix(glm::mat4&& viewMatrix) noexcept {
    m_ViewMatrixNoTranslation = viewMatrix;
    m_ViewMatrix              = std::move(viewMatrix);
    m_ViewMatrixNoTranslation[3][0] = 0.0001f;
    m_ViewMatrixNoTranslation[3][1] = 0.0001f;
    m_ViewMatrixNoTranslation[3][2] = 0.0001f;
}
void ComponentCamera::lookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up) noexcept {
    m_Eye                     = eye;
    m_Up                      = up;
    m_Forward                 = glm::normalize(m_Eye - center);
    setViewMatrix(glm::lookAt(m_Eye, m_Eye - m_Forward, m_Up));
}
void ComponentCamera::lookAt(glm::vec3&& eye, glm::vec3&& center, glm::vec3&& up) noexcept {
    m_Eye     = std::move(eye);
    m_Up      = std::move(up);
    m_Forward = glm::normalize(m_Eye - std::move(center));
    setViewMatrix(glm::lookAt(m_Eye, m_Eye - m_Forward, m_Up));
}
void ComponentCamera::setAngle(float angle) noexcept {
	m_Angle = angle;
	priv::ComponentCamera_Functions::RebuildProjectionMatrix(*this); 
}
void ComponentCamera::setAspect(float aspectRatio) noexcept {
	m_AspectRatio = aspectRatio;
	priv::ComponentCamera_Functions::RebuildProjectionMatrix(*this); 
}
void ComponentCamera::setNear(float nearPlane) noexcept {
	m_NearPlane = nearPlane;
	priv::ComponentCamera_Functions::RebuildProjectionMatrix(*this); 
}
void ComponentCamera::setFar(float farPlane) noexcept {
	m_FarPlane = farPlane;
	priv::ComponentCamera_Functions::RebuildProjectionMatrix(*this); 
}

#pragma endregion

#pragma region System

ComponentCamera_System_CI::ComponentCamera_System_CI() {
    setUpdateFunction([](void* system, void* componentPool, const float dt, Scene& scene) {
        auto* pool       = (ECSComponentPool<ComponentCamera>*)componentPool;
        auto& components = pool->data();
        auto lamda_update_component = [&](ComponentCamera& b, size_t i, size_t k) {
            Math::extractViewFrustumPlanesHartmannGribbs(b.getViewProjection(), b.getFrustrumPlanes());//update frustrum planes 
        };
        if (components.size() < 50) {
            for (size_t i = 0; i < components.size(); ++i) {
                lamda_update_component(components[i], i, 0);
            }
        }else{
            Engine::priv::threading::addJobSplitVectored(lamda_update_component, components, true, 0);
        }
    });
    setOnComponentAddedToEntityFunction([](void* system, void* componentCamera, Entity entity) {
    });
    setOnComponentRemovedFromEntityFunction([](void* system, Entity entity) {
    });
    setOnEntityAddedToSceneFunction([](void* system, void* componentPool, Entity entity, Scene& scene) {
    });
    setOnSceneEnteredFunction([](void* system, void* componentPool, Scene& scene) {
    });
    setOnSceneLeftFunction([](void* system, void* componentPool, Scene& scene) {
    });
}

#pragma endregion