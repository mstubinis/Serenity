#include <ecs/ComponentCamera.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/threading/Engine_ThreadManager.h>
#include <core/engine/system/Engine.h>
#include <core/engine/scene/Camera.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace Engine;
using namespace Engine::priv;
using namespace std;


void priv::ComponentCamera_Functions::RebuildProjectionMatrix(ComponentCamera& componentCamera) {
    if (componentCamera.m_Type == ComponentCamera::Type::Perspective) {
        componentCamera.m_ProjectionMatrix 
            = glm::perspective(componentCamera.m_Angle, componentCamera.m_AspectRatio, componentCamera.m_NearPlane, componentCamera.m_FarPlane);
    }else{
        componentCamera.m_ProjectionMatrix 
            = glm::ortho(componentCamera.m_Left, componentCamera.m_Right, componentCamera.m_Bottom, componentCamera.m_Top, componentCamera.m_NearPlane, componentCamera.m_FarPlane);
    }
}
const glm::mat4 priv::ComponentCamera_Functions::GetViewNoTranslation(Camera& camera) {
    return camera.m_Entity.getComponent<ComponentCamera>()->m_ViewMatrixNoTranslation;
}
const glm::mat4 priv::ComponentCamera_Functions::GetViewInverseNoTranslation(Camera& camera) {
    return glm::inverse(camera.m_Entity.getComponent<ComponentCamera>()->m_ViewMatrixNoTranslation);
}
const glm::mat4 priv::ComponentCamera_Functions::GetViewProjectionNoTranslation(Camera& camera) {
    const auto& componentCamera = *camera.m_Entity.getComponent<ComponentCamera>();
    return componentCamera.m_ProjectionMatrix * componentCamera.m_ViewMatrixNoTranslation;
}
const glm::mat4 priv::ComponentCamera_Functions::GetViewProjectionInverseNoTranslation(Camera& camera) {
    const auto& componentCamera = *camera.m_Entity.getComponent<ComponentCamera>();
    return glm::inverse(componentCamera.m_ProjectionMatrix * componentCamera.m_ViewMatrixNoTranslation);
}
const glm::vec3 priv::ComponentCamera_Functions::GetViewVectorNoTranslation(Camera& camera) {
    const auto& viewMatrixNoTranslation = camera.m_Entity.getComponent<ComponentCamera>()->m_ViewMatrixNoTranslation;
    return glm::vec3(viewMatrixNoTranslation[0][2], viewMatrixNoTranslation[1][2], viewMatrixNoTranslation[2][2]);
}


#pragma region Component

ComponentCamera::ComponentCamera(const Entity& entity, const float angleDegrees, const float aspectRatio, const float nearPlane, const float farPlane) : ComponentBaseClass(entity) {
    const auto one  = static_cast<decimal>(1.0);
    const auto zero = static_cast<decimal>(0.0);

    m_Eye                     = glm_vec3(zero);
	m_Up                      = glm_vec3(zero, one, zero);
    m_Forward                 = glm_vec3(zero, zero, -one);
    m_Angle                   = glm::radians(angleDegrees);
	m_AspectRatio             = aspectRatio;
	m_NearPlane               = nearPlane;
	m_FarPlane                = farPlane;
	m_Bottom                  = 0.0f;
	m_Top                     = 0.0f;
    m_ProjectionMatrix        = glm::perspective(m_Angle, m_AspectRatio, m_NearPlane, m_FarPlane);
    m_ViewMatrix              = glm::lookAt(m_Eye, m_Forward, m_Up);
	m_ViewMatrixNoTranslation = m_ViewMatrix;
    m_Type                    = Type::Perspective;
}
ComponentCamera::ComponentCamera(const Entity& entity, const float left, const float right, const float bottom, const float top, const float nearPlane, const float farPlane) : ComponentBaseClass(entity) {
    const auto one  = static_cast<decimal>(1.0);
    const auto zero = static_cast<decimal>(0.0);
    
    m_Eye                     = glm_vec3(zero);
    m_Up                      = glm_vec3(zero, one, zero);
    m_Forward                 = glm_vec3(zero, zero, -one);
    m_Left                    = left;
	m_Right                   = right;
	m_Bottom                  = bottom;
	m_Top                     = top;
	m_NearPlane               = nearPlane;
	m_FarPlane                = farPlane;
    m_ProjectionMatrix        = glm::ortho(m_Left, m_Right, m_Bottom, m_Top, m_NearPlane, m_FarPlane);
    m_ViewMatrix              = glm::lookAt(m_Eye, m_Forward, m_Up);
	m_ViewMatrixNoTranslation = m_ViewMatrix;
    m_Type                    = Type::Orthographic;
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
    if (m_Type == Type::Perspective) {
        m_AspectRatio = width / static_cast<float>(height);
    }
    priv::ComponentCamera_Functions::RebuildProjectionMatrix(*this);
}
const unsigned int ComponentCamera::pointIntersectTest(const glm_vec3& position) const {
    const auto zero = static_cast<decimal>(0.0);
    for (unsigned int i = 0; i < 6; ++i) {
        const auto d = m_FrustumPlanes[i].x * position.x + m_FrustumPlanes[i].y * position.y + m_FrustumPlanes[i].z * position.z + m_FrustumPlanes[i].w;
        if (d > zero)
            return 0; //outside
    }
    return 1;//inside
}
const unsigned int ComponentCamera::sphereIntersectTest(const glm_vec3& position, const float& radius) const {
    unsigned int res = 1; //inside the viewing frustum
    const auto zero = static_cast<decimal>(0.0);
    const auto two = static_cast<decimal>(2.0);
    if (radius <= zero)
		return 0;
    for (int i = 0; i < 6; ++i) {
        const auto d = m_FrustumPlanes[i].x * position.x + m_FrustumPlanes[i].y * position.y + m_FrustumPlanes[i].z * position.z + m_FrustumPlanes[i].w;
        if (d > radius * two)
			return 0; //outside the viewing frustrum
        else if (d > zero)
			res = 2; //intersecting the viewing plane
    }
    return res;
}
void ComponentCamera::lookAt(const glm_vec3& eye, const glm_vec3& center, const glm_vec3& up) {
    m_Eye                     = eye;
    m_Up                      = up;
    m_Forward                 = glm::normalize(m_Eye - center);
    m_ViewMatrix              = glm::lookAt(m_Eye, m_Eye - m_Forward, m_Up);
    m_ViewMatrixNoTranslation = glm::lookAt(glm_vec3(static_cast<decimal>(0.0)), -m_Forward, m_Up);
}

const glm_vec3 ComponentCamera::forward() const {
    return m_Forward;
}
const glm_vec3 ComponentCamera::right() const {
    return glm::normalize(glm_vec3(m_ViewMatrixNoTranslation[0][0], m_ViewMatrixNoTranslation[1][0], m_ViewMatrixNoTranslation[2][0]));
}
const glm_vec3 ComponentCamera::up() const {
    return (m_Up); //normalize later?
}


const glm::mat4 ComponentCamera::getProjection() const {
	return m_ProjectionMatrix; 
}
const glm::mat4 ComponentCamera::getProjectionInverse() const {
	return glm::inverse(m_ProjectionMatrix); 
}
const glm::mat4 ComponentCamera::getView() const {
	return m_ViewMatrix; 
}
const glm::mat4 ComponentCamera::getViewInverse() const {
	return glm::inverse(m_ViewMatrix); 
}
const glm::mat4 ComponentCamera::getViewProjection() const {
	return m_ProjectionMatrix * m_ViewMatrix; 
}
const glm::mat4 ComponentCamera::getViewProjectionInverse() const {
	return glm::inverse(m_ProjectionMatrix * m_ViewMatrix); 
}
const glm::vec3 ComponentCamera::getViewVector() const {
	return glm::normalize(glm::vec3(m_ViewMatrix[0][2], m_ViewMatrix[1][2], m_ViewMatrix[2][2]));
}
const float ComponentCamera::getAngle() const {
	return m_Angle; 
}
const float ComponentCamera::getAspect() const {
	return m_AspectRatio; 
}
const float ComponentCamera::getNear() const {
	return m_NearPlane; 
}
const float ComponentCamera::getFar() const {
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

struct priv::ComponentCamera_UpdateFunction final { void operator()(void* componentPool, const float& dt, Scene& scene) const {
	auto& pool = *static_cast<ECSComponentPool<Entity, ComponentCamera>*>(componentPool);
	auto& components = pool.data();
    auto lamda_update_component = [&](ComponentCamera& b, const size_t& i) {
        Math::extractViewFrustumPlanesHartmannGribbs(b.m_ProjectionMatrix * b.m_ViewMatrix, b.m_FrustumPlanes);//update view frustrum 
    };
    priv::Core::m_Engine->m_ThreadManager.add_job_engine_controlled_split_vectored(lamda_update_component, components, true);
}};
struct priv::ComponentCamera_ComponentAddedToEntityFunction final {void operator()(void* componentCamera, Entity& entity) const {
}};
struct priv::ComponentCamera_EntityAddedToSceneFunction final {void operator()(void* componentPool, Entity& p_Entity, Scene& scene) const {
}};
struct priv::ComponentCamera_SceneEnteredFunction final {void operator()(void* componentPool, Scene& scene) const {
}};
struct priv::ComponentCamera_SceneLeftFunction final {void operator()(void* componentPool, Scene& scene) const {
}};

ComponentCamera_System_CI::ComponentCamera_System_CI() {
    setUpdateFunction(ComponentCamera_UpdateFunction());
    setOnComponentAddedToEntityFunction(ComponentCamera_ComponentAddedToEntityFunction());
    setOnEntityAddedToSceneFunction(ComponentCamera_EntityAddedToSceneFunction());
    setOnSceneEnteredFunction(ComponentCamera_SceneEnteredFunction());
    setOnSceneLeftFunction(ComponentCamera_SceneLeftFunction());
}

#pragma endregion