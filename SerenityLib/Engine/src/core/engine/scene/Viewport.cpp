#include <core/engine/scene/Viewport.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/resources/Engine_Resources.h>

using namespace Engine;
using namespace std;

Viewport::Viewport(const Scene& scene, const Camera& camera):m_Scene(const_cast<Scene&>(scene)){
    setCamera(camera);
    const auto& winSize   = Resources::getWindowSize();
    setViewportDimensions(0, 0, winSize.x, winSize.y);
    activate();
    activate2DAPI();
    setAspectRatioSynced(true);
    setSkyboxVisible(true);

    setDepthMaskValue(50.0f);
    activateDepthMask(false);

    setBackgroundColor(0.0f, 0.0f, 0.0f, 1.0f);
}
Viewport::~Viewport() {
}

const unsigned int& Viewport::id() const {
    return m_ID;
}
const bool& Viewport::isSkyboxVisible() const {
    return m_SkyboxVisible;
}
void Viewport::setSkyboxVisible(const bool& visible) {
    m_SkyboxVisible = visible;
}

const float& Viewport::getDepthMaskValue() const {
    return m_DepthMaskValue;
}
void Viewport::setDepthMaskValue(const float& depth) {
    m_DepthMaskValue = depth;
}

void Viewport::activateDepthMask(const bool& active) {
    m_DepthMaskActive = active;
}
const bool Viewport::isDepthMaskActive() const {
    return m_DepthMaskActive;
}
const glm::vec4& Viewport::getBackgroundColor() const {
    return m_BackgroundColor;
}
void Viewport::setBackgroundColor(const float& r, const float& g, const float& b, const float& a) {
    m_BackgroundColor.r = r;
    m_BackgroundColor.g = g;
    m_BackgroundColor.b = b;
    m_BackgroundColor.a = a;
}

void Viewport::setAspectRatioSynced(const bool& synced) {
    m_AspectRatioSynced = synced;
}
const bool Viewport::isAspectRatioSynced() const {
    return m_AspectRatioSynced;
}

const Scene& Viewport::getScene() const {
    return m_Scene;
}
const Camera& Viewport::getCamera() const {
    return *m_Camera;
}
void Viewport::activate2DAPI(const bool& active) {
    m_Using2DAPI = active;
}
const bool Viewport::isUsing2DAPI() const {
    return m_Using2DAPI;
}
void Viewport::activate(const bool& active) {
    m_Active = active;
}
const bool Viewport::isActive() const {
    return m_Active;
}
bool Viewport::setCamera(const Camera& camera) {
    auto& scene_cameras = epriv::InternalScenePublicInterface::GetCameras(m_Scene);
    if (isInVector(scene_cameras, &camera)) {
        m_Camera = &const_cast<Camera&>(camera);
        return true;
    }
    return false;
}
void Viewport::setViewportDimensions(const unsigned int& x, const unsigned int& y, const unsigned int& width, const unsigned int& height) {
    m_Viewport_Dimensions.x = x;
    m_Viewport_Dimensions.y = y;
    m_Viewport_Dimensions.z = width;
    m_Viewport_Dimensions.w = height;
}
const glm::uvec4& Viewport::getViewportDimensions() const {
    return m_Viewport_Dimensions;
}