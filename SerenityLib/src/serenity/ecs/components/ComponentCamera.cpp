#include <serenity/ecs/components/ComponentCamera.h>

#include <serenity/math/Engine_Math.h>

void Engine::priv::ComponentCamera_Functions::RebuildProjectionMatrix(ComponentCamera& component) noexcept {
    if (component.m_Type == ComponentCamera::CameraType::Perspective) {
        component.m_ProjectionMatrix = glm::perspective(component.m_Angle, component.m_AspectRatio, component.m_NearPlane, component.m_FarPlane);
    }else{
        component.m_ProjectionMatrix = glm::ortho(component.m_Left, component.m_Right, component.m_Bottom, component.m_Top, component.m_NearPlane, component.m_FarPlane);
    }
}

#pragma region Component

ComponentCamera::ComponentCamera(Entity entity, float angleDegrees, float aspectRatio, float nearPlane, float farPlane) 
    : m_Angle      { glm::radians(angleDegrees) }
    , m_AspectRatio{ aspectRatio }
    , m_NearPlane  { nearPlane }
    , m_FarPlane   { farPlane }
    , m_Type       { CameraType::Perspective }
{
    setProjectionMatrix(glm::perspective(m_Angle, m_AspectRatio, m_NearPlane, m_FarPlane));
    setViewMatrix(glm::lookAt(m_Eye, m_Forward, m_Up));
}
ComponentCamera::ComponentCamera(Entity entity, float left, float right, float bottom, float top, float nearPlane, float farPlane) 
    : m_Left     { left }
    , m_Right    { right }
    , m_Bottom   { bottom }
    , m_Top      { top }
    , m_NearPlane{ nearPlane }
    , m_FarPlane { farPlane }
    , m_Type     { CameraType::Orthographic }
{
    setProjectionMatrix(glm::ortho(m_Left, m_Right, m_Bottom, m_Top, m_NearPlane, m_FarPlane));
    setViewMatrix(glm::lookAt(m_Eye, m_Forward, m_Up));
}
ComponentCamera::ComponentCamera(const ComponentCamera& other) 
    : m_Eye             { (other.m_Eye) }
    , m_Up              { (other.m_Up) }
    , m_Forward         { (other.m_Forward) }
    , m_Angle           { (other.m_Angle) }
    , m_AspectRatio     { (other.m_AspectRatio) }
    , m_NearPlane       { (other.m_NearPlane) }
    , m_FarPlane        { (other.m_FarPlane) }
    , m_Bottom          { (other.m_Bottom) }
    , m_Top             { (other.m_Top) }
    , m_ProjectionMatrix{ (other.m_ProjectionMatrix) }
    , m_ViewMatrix      { (other.m_ViewMatrix) }
    , m_FrustumPlanes   { (other.m_FrustumPlanes) }
    , m_Type            { (other.m_Type) }
{}
ComponentCamera& ComponentCamera::operator=(const ComponentCamera& other) {
    if (&other != this) {
        m_Eye              = (other.m_Eye);
        m_Up               = (other.m_Up);
        m_Forward          = (other.m_Forward);
        m_Angle            = (other.m_Angle);
        m_AspectRatio      = (other.m_AspectRatio);
        m_NearPlane        = (other.m_NearPlane);
        m_FarPlane         = (other.m_FarPlane);
        m_Bottom           = (other.m_Bottom);
        m_Top              = (other.m_Top);
        m_ProjectionMatrix = (other.m_ProjectionMatrix);
        m_ViewMatrix       = (other.m_ViewMatrix);
        m_FrustumPlanes    = (other.m_FrustumPlanes);
        m_Type             = (other.m_Type);
    }
    return *this;
}
ComponentCamera::ComponentCamera(ComponentCamera&& other) noexcept 
    : m_Eye             { std::move(other.m_Eye) }
    , m_Up              { std::move(other.m_Up) }
    , m_Forward         { std::move(other.m_Forward) }
    , m_Angle           { std::move(other.m_Angle) }
    , m_AspectRatio     { std::move(other.m_AspectRatio) }
    , m_NearPlane       { std::move(other.m_NearPlane) }
    , m_FarPlane        { std::move(other.m_FarPlane) }
    , m_Bottom          { std::move(other.m_Bottom) }
    , m_Top             { std::move(other.m_Top) }
    , m_ProjectionMatrix{ std::move(other.m_ProjectionMatrix) }
    , m_ViewMatrix      { std::move(other.m_ViewMatrix) }
    , m_FrustumPlanes   { std::move(other.m_FrustumPlanes) }
    , m_Type            { std::move(other.m_Type) }
{}
ComponentCamera& ComponentCamera::operator=(ComponentCamera&& other) noexcept {
    m_Eye              = std::move(other.m_Eye);
    m_Up               = std::move(other.m_Up);
    m_Forward          = std::move(other.m_Forward);
    m_Angle            = std::move(other.m_Angle);
    m_AspectRatio      = std::move(other.m_AspectRatio);
    m_NearPlane        = std::move(other.m_NearPlane);
    m_FarPlane         = std::move(other.m_FarPlane);
    m_Bottom           = std::move(other.m_Bottom);
    m_Top              = std::move(other.m_Top);
    m_ProjectionMatrix = std::move(other.m_ProjectionMatrix);
    m_ViewMatrix       = std::move(other.m_ViewMatrix);
    m_FrustumPlanes    = std::move(other.m_FrustumPlanes);
    m_Type             = std::move(other.m_Type);
    return *this;
}

void ComponentCamera::resize(uint32_t width, uint32_t height) noexcept {
    if (m_Type == CameraType::Perspective) {
        m_AspectRatio = width / (float)height;
    }
    Engine::priv::ComponentCamera_Functions::RebuildProjectionMatrix(*this);
}
uint32_t ComponentCamera::pointIntersectTest(const glm_vec3& worldPosition) const noexcept {
    auto zero = (decimal)0.0;
    for (uint32_t i = 0; i < m_FrustumPlanes.size(); ++i) {
        auto d = m_FrustumPlanes[i].x * worldPosition.x + m_FrustumPlanes[i].y * worldPosition.y + m_FrustumPlanes[i].z * worldPosition.z + m_FrustumPlanes[i].w;
        if (d > zero) {
            return 0; //outside
        }
    }
    return 1; //inside
}
uint32_t ComponentCamera::sphereIntersectTest(const glm_vec3& worldPosition, float radius) const noexcept {
    uint32_t res = 1; //inside the viewing frustum
    auto zero    = (decimal)0.0;
    auto two     = (decimal)2.0;
    if (radius <= zero) {
        return 0;
    }
    for (size_t i = 0; i < m_FrustumPlanes.size(); ++i) {
        auto d = m_FrustumPlanes[i].x * worldPosition.x + m_FrustumPlanes[i].y * worldPosition.y + m_FrustumPlanes[i].z * worldPosition.z + m_FrustumPlanes[i].w;
        if (d > radius * two) {
            return 0; //outside the viewing frustrum
        }else if (d > zero) {
            res = 2; //intersecting the viewing plane
        }
    }
    return res;
}
void ComponentCamera::setViewMatrix(const glm::mat4& viewMatrix) noexcept {
    m_ViewMatrix = viewMatrix;
}
void ComponentCamera::setViewMatrix(glm::mat4&& viewMatrix) noexcept {
    m_ViewMatrix = std::move(viewMatrix);
}
void ComponentCamera::lookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up) noexcept {
    m_Eye     = eye;
    m_Up      = up;
    m_Forward = glm::normalize(m_Eye - center);
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
    Engine::priv::ComponentCamera_Functions::RebuildProjectionMatrix(*this);
}
void ComponentCamera::setAspect(float aspectRatio) noexcept {
	m_AspectRatio = aspectRatio;
    Engine::priv::ComponentCamera_Functions::RebuildProjectionMatrix(*this);
}
void ComponentCamera::setNear(float nearPlane) noexcept {
	m_NearPlane = nearPlane;
    Engine::priv::ComponentCamera_Functions::RebuildProjectionMatrix(*this);
}
void ComponentCamera::setFar(float farPlane) noexcept {
	m_FarPlane = farPlane;
    Engine::priv::ComponentCamera_Functions::RebuildProjectionMatrix(*this);
}

#pragma endregion
