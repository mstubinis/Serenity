#include <ecs/ComponentCamera.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/threading/ThreadingModule.h>
#include <core/engine/system/Engine.h>
#include <core/engine/scene/Camera.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace Engine;
using namespace Engine::priv;
using namespace std;


void priv::ComponentCamera_Functions::RebuildProjectionMatrix(ComponentCamera& componentCamera) {
    if (componentCamera.m_Type == ComponentCamera::CameraType::Perspective) {
        componentCamera.m_ProjectionMatrix 
            = glm::perspective(componentCamera.m_Angle, componentCamera.m_AspectRatio, componentCamera.m_NearPlane, componentCamera.m_FarPlane);
    }else{
        componentCamera.m_ProjectionMatrix 
            = glm::ortho(componentCamera.m_Left, componentCamera.m_Right, componentCamera.m_Bottom, componentCamera.m_Top, componentCamera.m_NearPlane, componentCamera.m_FarPlane);
    }
}
const glm::mat4 priv::ComponentCamera_Functions::GetViewNoTranslation(const Camera& camera) {
    return camera.getComponent<ComponentCamera>()->m_ViewMatrixNoTranslation;
}
const glm::mat4 priv::ComponentCamera_Functions::GetViewInverseNoTranslation(const Camera& camera) {
    return glm::inverse(camera.getComponent<ComponentCamera>()->m_ViewMatrixNoTranslation);
}
const glm::mat4 priv::ComponentCamera_Functions::GetViewProjectionNoTranslation(const Camera& camera) {
    const auto& componentCamera = *camera.getComponent<ComponentCamera>();
    return componentCamera.m_ProjectionMatrix * componentCamera.m_ViewMatrixNoTranslation;
}
const glm::mat4 priv::ComponentCamera_Functions::GetViewProjectionInverseNoTranslation(const Camera& camera) {
    const auto& componentCamera = *camera.getComponent<ComponentCamera>();
    return glm::inverse(componentCamera.m_ProjectionMatrix * componentCamera.m_ViewMatrixNoTranslation);
}
const glm::vec3 priv::ComponentCamera_Functions::GetViewVectorNoTranslation(const Camera& camera) {
    const auto& viewMatrixNoTranslation = camera.getComponent<ComponentCamera>()->m_ViewMatrixNoTranslation;
    return glm::vec3(viewMatrixNoTranslation[0][2], viewMatrixNoTranslation[1][2], viewMatrixNoTranslation[2][2]);
}


#pragma region Component

ComponentCamera::ComponentCamera(Entity entity, float angleDegrees, float aspectRatio, float nearPlane, float farPlane) {
    m_Owner = entity;
    m_Angle                   = glm::radians(angleDegrees);
	m_AspectRatio             = aspectRatio;
	m_NearPlane               = nearPlane;
	m_FarPlane                = farPlane;

    setProjectionMatrix(glm::perspective(m_Angle, m_AspectRatio, m_NearPlane, m_FarPlane));
    setViewMatrix(glm::lookAt(m_Eye, m_Forward, m_Up));

    m_Type                    = CameraType::Perspective;
}
ComponentCamera::ComponentCamera(Entity entity, float left, float right, float bottom, float top, float nearPlane, float farPlane) {
    m_Owner = entity;
    m_Left                    = left;
	m_Right                   = right;
	m_Bottom                  = bottom;
	m_Top                     = top;
	m_NearPlane               = nearPlane;
	m_FarPlane                = farPlane;

    setProjectionMatrix(glm::ortho(m_Left, m_Right, m_Bottom, m_Top, m_NearPlane, m_FarPlane));
    setViewMatrix(glm::lookAt(m_Eye, m_Forward, m_Up));

    m_Type                    = CameraType::Orthographic;
}
ComponentCamera::~ComponentCamera() {
}
ComponentCamera::ComponentCamera(ComponentCamera&& other) noexcept {
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
    m_Type                    = std::move(other.m_Type);
}
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
        m_Type                    = std::move(other.m_Type);
    }
    return *this;
}

void ComponentCamera::resize(const unsigned int width, const unsigned int height) {
    if (m_Type == CameraType::Perspective) {
        m_AspectRatio = width / static_cast<float>(height);
    }
    priv::ComponentCamera_Functions::RebuildProjectionMatrix(*this);
}
unsigned int ComponentCamera::pointIntersectTest(const glm_vec3& position) const {
    const auto zero = static_cast<decimal>(0.0);
    for (unsigned int i = 0; i < 6; ++i) {
        const auto d = m_FrustumPlanes[i].x * position.x + m_FrustumPlanes[i].y * position.y + m_FrustumPlanes[i].z * position.z + m_FrustumPlanes[i].w;
        if (d > zero) {
            return 0; //outside
        }
    }
    return 1; //inside
}
unsigned int ComponentCamera::sphereIntersectTest(const glm_vec3& position, const float radius) const { 
    unsigned int res = 1; //inside the viewing frustum
    const auto zero  = static_cast<decimal>(0.0);
    const auto two   = static_cast<decimal>(2.0);
    if (radius <= zero) {
        return 0;
    }
    for (int i = 0; i < 6; ++i) {
        const auto d = m_FrustumPlanes[i].x * position.x + m_FrustumPlanes[i].y * position.y + m_FrustumPlanes[i].z * position.z + m_FrustumPlanes[i].w;
        if (d > radius * two) {
            return 0; //outside the viewing frustrum
        }else if (d > zero) {
            res = 2; //intersecting the viewing plane
        }
    }
    return res;
}
void ComponentCamera::setViewMatrix(const glm::mat4& viewMatrix) {
    m_ViewMatrix = viewMatrix;
    m_ViewMatrixNoTranslation = viewMatrix;
    m_ViewMatrixNoTranslation[3][0] = 0.0001f;
    m_ViewMatrixNoTranslation[3][1] = 0.0001f;
    m_ViewMatrixNoTranslation[3][2] = 0.0001f;
}
void ComponentCamera::setProjectionMatrix(const glm::mat4& projectionMatrix) {
    m_ProjectionMatrix = projectionMatrix;
}
void ComponentCamera::lookAt(const glm_vec3& eye, const glm_vec3& center, const glm_vec3& up) {
    m_Eye                     = eye;
    m_Up                      = up;
    m_Forward                 = glm::normalize(m_Eye - center);

    setViewMatrix(glm::lookAt(m_Eye, m_Eye - m_Forward, m_Up));
}
glm_vec3 ComponentCamera::forward() const {
    return m_Forward;
}
glm_vec3 ComponentCamera::right() const {
    return glm::normalize(glm_vec3(m_ViewMatrixNoTranslation[0][0], m_ViewMatrixNoTranslation[1][0], m_ViewMatrixNoTranslation[2][0]));
}
glm_vec3 ComponentCamera::up() const {
    return (m_Up); //normalize later?
}


glm::mat4 ComponentCamera::getProjection() const {
	return m_ProjectionMatrix; 
}
glm::mat4 ComponentCamera::getProjectionInverse() const {
	return glm::inverse(m_ProjectionMatrix); 
}
glm::mat4 ComponentCamera::getView() const {
	return m_ViewMatrix; 
}
glm::mat4 ComponentCamera::getViewInverse() const {
	return glm::inverse(m_ViewMatrix); 
}
glm::mat4 ComponentCamera::getViewProjection() const {
	return m_ProjectionMatrix * m_ViewMatrix; 
}
glm::mat4 ComponentCamera::getViewProjectionInverse() const {
	return glm::inverse(m_ProjectionMatrix * m_ViewMatrix); 
}
glm::vec3 ComponentCamera::getViewVector() const {
	return glm::normalize(glm::vec3(m_ViewMatrix[0][2], m_ViewMatrix[1][2], m_ViewMatrix[2][2]));
}
float ComponentCamera::getAngle() const {
	return m_Angle; 
}
float ComponentCamera::getAspect() const {
	return m_AspectRatio; 
}
float ComponentCamera::getNear() const {
	return m_NearPlane; 
}
float ComponentCamera::getFar() const {
	return m_FarPlane; 
}
void ComponentCamera::setAngle(const float angle) { 
	m_Angle = angle;
	priv::ComponentCamera_Functions::RebuildProjectionMatrix(*this); 
}
void ComponentCamera::setAspect(const float aspectRatio) {
	m_AspectRatio = aspectRatio;
	priv::ComponentCamera_Functions::RebuildProjectionMatrix(*this); 
}
void ComponentCamera::setNear(const float nearPlane) { 
	m_NearPlane = nearPlane;
	priv::ComponentCamera_Functions::RebuildProjectionMatrix(*this); 
}
void ComponentCamera::setFar(const float farPlane) { 
	m_FarPlane = farPlane;
	priv::ComponentCamera_Functions::RebuildProjectionMatrix(*this); 
}

#pragma endregion

#pragma region System

struct priv::ComponentCamera_UpdateFunction final { void operator()(void* system, void* componentPool, const float dt, Scene& scene) const {
	auto& pool                  = *static_cast<ECSComponentPool<Entity, ComponentCamera>*>(componentPool);
	auto& components            = pool.data();
    auto lamda_update_component = [&](ComponentCamera& b, size_t i, size_t k) {
        Math::extractViewFrustumPlanesHartmannGribbs(b.m_ProjectionMatrix * b.m_ViewMatrix, b.m_FrustumPlanes);//update frustrum planes 
    };
    if (components.size() < 50) {
        for (size_t i = 0; i < components.size(); ++i) {
            lamda_update_component(components[i], i, 0);
        }
    }else{
        Engine::priv::threading::addJobSplitVectored(lamda_update_component, components, true, 0);
    }
}};
struct priv::ComponentCamera_ComponentAddedToEntityFunction final {void operator()(void* system, void* componentCamera, Entity entity) const {
}};
struct priv::ComponentCamera_ComponentRemovedFromEntityFunction final { void operator()(void* system, Entity entity) const {
}};
struct priv::ComponentCamera_EntityAddedToSceneFunction final {void operator()(void* system, void* componentPool, Entity entity, Scene& scene) const {
}};
struct priv::ComponentCamera_SceneEnteredFunction final {void operator()(void* system, void* componentPool, Scene& scene) const {
}};
struct priv::ComponentCamera_SceneLeftFunction final {void operator()(void* system, void* componentPool, Scene& scene) const {
}};

ComponentCamera_System_CI::ComponentCamera_System_CI() {
    setUpdateFunction(ComponentCamera_UpdateFunction());
    setOnComponentAddedToEntityFunction(ComponentCamera_ComponentAddedToEntityFunction());
    setOnComponentRemovedFromEntityFunction(ComponentCamera_ComponentRemovedFromEntityFunction());
    setOnEntityAddedToSceneFunction(ComponentCamera_EntityAddedToSceneFunction());
    setOnSceneEnteredFunction(ComponentCamera_SceneEnteredFunction());
    setOnSceneLeftFunction(ComponentCamera_SceneLeftFunction());
}

#pragma endregion