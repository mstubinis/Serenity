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


void priv::ComponentCamera_Functions::RebuildProjectionMatrix(ComponentCamera& p_ComCamera) {
    if (p_ComCamera.m_Type == ComponentCamera::Type::Perspective) {
        p_ComCamera.m_ProjectionMatrix = glm::perspective(p_ComCamera.m_Angle, p_ComCamera.m_AspectRatio,p_ComCamera.m_NearPlane,p_ComCamera.m_FarPlane);
    }else{
        p_ComCamera.m_ProjectionMatrix = glm::ortho(p_ComCamera.m_Left,p_ComCamera.m_Right,p_ComCamera.m_Bottom,p_ComCamera.m_Top,p_ComCamera.m_NearPlane,p_ComCamera.m_FarPlane);
    }
}
const glm::mat4 priv::ComponentCamera_Functions::GetViewNoTranslation(Camera& p_Camera) {
    return p_Camera.m_Entity.getComponent<ComponentCamera>()->m_ViewMatrixNoTranslation;
}
const glm::mat4 priv::ComponentCamera_Functions::GetViewInverseNoTranslation(Camera& p_Camera) {
    return glm::inverse(p_Camera.m_Entity.getComponent<ComponentCamera>()->m_ViewMatrixNoTranslation);
}
const glm::mat4 priv::ComponentCamera_Functions::GetViewProjectionNoTranslation(Camera& p_Camera) {
    auto& componentCamera = *p_Camera.m_Entity.getComponent<ComponentCamera>();
    return componentCamera.m_ProjectionMatrix * componentCamera.m_ViewMatrixNoTranslation;
}
const glm::mat4 priv::ComponentCamera_Functions::GetViewProjectionInverseNoTranslation(Camera& p_Camera) {
    auto& componentCamera = *p_Camera.m_Entity.getComponent<ComponentCamera>();
    return glm::inverse(componentCamera.m_ProjectionMatrix * componentCamera.m_ViewMatrixNoTranslation);
}
const glm::vec3 priv::ComponentCamera_Functions::GetViewVectorNoTranslation(Camera& p_Camera) {
    auto& viewMatrixNoTranslation = p_Camera.m_Entity.getComponent<ComponentCamera>()->m_ViewMatrixNoTranslation;
    return glm::vec3(viewMatrixNoTranslation[0][2], viewMatrixNoTranslation[1][2], viewMatrixNoTranslation[2][2]);
}


#pragma region Component

ComponentCamera::ComponentCamera(const Entity& p_Entity, const float p_AngleDegrees, const float p_AspectRatio, const float p_NearPlane, const float p_FarPlane) : ComponentBaseClass(p_Entity) {
    const auto one  = static_cast<decimal>(1.0);
    const auto zero = static_cast<decimal>(0.0);

    m_Eye                     = glm_vec3(zero);
	m_Up                      = glm_vec3(zero, one, zero);
    m_Forward                 = glm_vec3(zero, zero, -one);
    m_Angle                   = glm::radians(p_AngleDegrees);
	m_AspectRatio             = p_AspectRatio;
	m_NearPlane               = p_NearPlane;
	m_FarPlane                = p_FarPlane;
	m_Bottom                  = 0.0f;
	m_Top                     = 0.0f;
    m_ProjectionMatrix        = glm::perspective(m_Angle, m_AspectRatio, m_NearPlane, m_FarPlane);
    m_ViewMatrix              = glm::lookAt(m_Eye, m_Forward, m_Up);
	m_ViewMatrixNoTranslation = m_ViewMatrix;
    m_Type                    = Type::Perspective;
}
ComponentCamera::ComponentCamera(const Entity& p_Entity, const float p_Left, const float p_Right, const float p_Bottom, const float p_Top, const float p_NearPlane, const float p_FarPlane) : ComponentBaseClass(p_Entity) {
    const auto one  = static_cast<decimal>(1.0);
    const auto zero = static_cast<decimal>(0.0);
    
    m_Eye                     = glm_vec3(zero);
    m_Up                      = glm_vec3(zero, one, zero);
    m_Forward                 = glm_vec3(zero, zero, -one);
    m_Left                    = p_Left;
	m_Right                   = p_Right;
	m_Bottom                  = p_Bottom;
	m_Top                     = p_Top;
	m_NearPlane               = p_NearPlane;
	m_FarPlane                = p_FarPlane;
    m_ProjectionMatrix        = glm::ortho(m_Left, m_Right, m_Bottom, m_Top, m_NearPlane, m_FarPlane);
    m_ViewMatrix              = glm::lookAt(m_Eye, m_Forward, m_Up);
	m_ViewMatrixNoTranslation = m_ViewMatrix;
    m_Type                    = Type::Orthographic;
}
ComponentCamera::~ComponentCamera() {
}
void ComponentCamera::resize(const unsigned int p_Width, const unsigned int p_Height) {
    if (m_Type == Type::Perspective) {
        m_AspectRatio = p_Width / static_cast<float>(p_Height);
    }
    priv::ComponentCamera_Functions::RebuildProjectionMatrix(*this);
}
const unsigned int ComponentCamera::pointIntersectTest(const glm_vec3& p_Position) const {
    const auto zero = static_cast<decimal>(0.0);
    for (unsigned int i = 0; i < 6; ++i) {
        const auto d = m_FrustumPlanes[i].x * p_Position.x + m_FrustumPlanes[i].y * p_Position.y + m_FrustumPlanes[i].z * p_Position.z + m_FrustumPlanes[i].w;
        if (d > zero)
            return 0; //outside
    }
    return 1;//inside
}
const unsigned int ComponentCamera::sphereIntersectTest(const glm_vec3& p_Position, const float& p_Radius) const {
    unsigned int res = 1; //inside the viewing frustum
    const auto zero = static_cast<decimal>(0.0);
    const auto two = static_cast<decimal>(2.0);
    if (p_Radius <= zero)
		return 0;
    for (int i = 0; i < 6; ++i) {
        const auto d = m_FrustumPlanes[i].x * p_Position.x + m_FrustumPlanes[i].y * p_Position.y + m_FrustumPlanes[i].z * p_Position.z + m_FrustumPlanes[i].w;
        if (d > p_Radius * two)
			return 0; //outside the viewing frustrum
        else if (d > zero)
			res = 2; //intersecting the viewing plane
    }
    return res;
}
void ComponentCamera::lookAt(const glm_vec3& p_Eye, const glm_vec3& p_Center, const glm_vec3& p_Up) {
    m_Eye                     = p_Eye;
    m_Up                      = p_Up;
    m_Forward                 = glm::normalize(m_Eye - p_Center);
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
void ComponentCamera::setAngle(const float p_Angle) { 
	m_Angle = p_Angle;
	priv::ComponentCamera_Functions::RebuildProjectionMatrix(*this); 
}
void ComponentCamera::setAspect(const float p_AspectRatio) {
	m_AspectRatio = p_AspectRatio;
	priv::ComponentCamera_Functions::RebuildProjectionMatrix(*this); 
}
void ComponentCamera::setNear(const float p_NearPlane) { 
	m_NearPlane = p_NearPlane;
	priv::ComponentCamera_Functions::RebuildProjectionMatrix(*this); 
}
void ComponentCamera::setFar(const float p_FarPlane) { 
	m_FarPlane = p_FarPlane;
	priv::ComponentCamera_Functions::RebuildProjectionMatrix(*this); 
}

#pragma endregion

#pragma region System

struct priv::ComponentCamera_UpdateFunction final { void operator()(void* p_ComponentPool, const double& p_Dt, Scene& p_Scene) const {
	auto& pool = *(ECSComponentPool<Entity, ComponentCamera>*)p_ComponentPool;
	auto& components = pool.data();
    auto lamda_update = [&](pair<size_t, size_t>& pair_) {
        for (size_t j = pair_.first; j <= pair_.second; ++j) {
            ComponentCamera& b = components[j];
            Math::extractViewFrustumPlanesHartmannGribbs(b.m_ProjectionMatrix * b.m_ViewMatrix, b.m_FrustumPlanes);//update view frustrum 
        }
    };
	auto split = priv::threading::splitVectorPairs(components);
    for (auto& pair : split) {
        priv::Core::m_Engine->m_ThreadManager.add_job_ref_engine_controlled(lamda_update, pair);
    }
    priv::Core::m_Engine->m_ThreadManager.wait_for_all_engine_controlled();
}};
struct priv::ComponentCamera_ComponentAddedToEntityFunction final {void operator()(void* p_ComponentCamera, Entity& p_Entity) const {
}};
struct priv::ComponentCamera_EntityAddedToSceneFunction final {void operator()(void* p_ComponentPool, Entity& p_Entity, Scene& p_Scene) const {
}};
struct priv::ComponentCamera_SceneEnteredFunction final {void operator()(void* p_ComponentPool, Scene& p_Scene) const {
}};
struct priv::ComponentCamera_SceneLeftFunction final {void operator()(void* p_ComponentPool, Scene& p_Scene) const {
}};

ComponentCamera_System_CI::ComponentCamera_System_CI() {
    setUpdateFunction(ComponentCamera_UpdateFunction());
    setOnComponentAddedToEntityFunction(ComponentCamera_ComponentAddedToEntityFunction());
    setOnEntityAddedToSceneFunction(ComponentCamera_EntityAddedToSceneFunction());
    setOnSceneEnteredFunction(ComponentCamera_SceneEnteredFunction());
    setOnSceneLeftFunction(ComponentCamera_SceneLeftFunction());
}

#pragma endregion