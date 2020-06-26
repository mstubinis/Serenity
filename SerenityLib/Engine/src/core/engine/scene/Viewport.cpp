#include <core/engine/scene/Viewport.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/renderer/Renderer.h>
#include <core/engine/resources/Engine_Resources.h>

using namespace Engine;
using namespace std;

Viewport::Viewport() {
    m_RenderFlags = ViewportRenderingFlag::_ALL;
    activate();
    setAspectRatioSynced(true);
    activateDepthMask(false);
}
Viewport::Viewport(const Scene& scene, const Camera& camera) : Viewport() {
    auto winSize  = Resources::getWindowSize();
    m_Scene       = &const_cast<Scene&>(scene);

    setCamera(camera);
    setViewportDimensions(0.0f, 0.0f, static_cast<float>(winSize.x), static_cast<float>(winSize.y));
}
Viewport::Viewport(Viewport&& other) noexcept {
    m_Scene                  = std::exchange(other.m_Scene, nullptr);
    m_Camera                 = std::exchange(other.m_Camera, nullptr);
    m_Viewport_Dimensions    = std::move(other.m_Viewport_Dimensions);
    m_BackgroundColor        = std::move(other.m_BackgroundColor);
    m_StateFlags             = std::move(other.m_StateFlags);
    m_DepthMaskValue         = std::move(other.m_DepthMaskValue);
    m_ID                     = std::move(other.m_ID);
    m_RenderFlags            = std::move(other.m_RenderFlags);
}
Viewport& Viewport::operator=(Viewport&& other) noexcept {
    if (&other != this) {
        m_Scene                  = std::exchange(other.m_Scene, nullptr);
        m_Camera                 = std::exchange(other.m_Camera, nullptr);
        m_Viewport_Dimensions    = std::move(other.m_Viewport_Dimensions);
        m_BackgroundColor        = std::move(other.m_BackgroundColor);
        m_StateFlags             = std::move(other.m_StateFlags);
        m_DepthMaskValue         = std::move(other.m_DepthMaskValue);
        m_ID                     = std::move(other.m_ID);
        m_RenderFlags            = std::move(other.m_RenderFlags);
    }
    return *this;
}


void Viewport::activateDepthMask(bool active) {
    (active) ? m_StateFlags.add(StateFlags::DepthMaskActive) : m_StateFlags.remove(StateFlags::DepthMaskActive);
}
bool Viewport::isDepthMaskActive() const {
    return m_StateFlags.has(StateFlags::DepthMaskActive);
}
void Viewport::setBackgroundColor(float r, float g, float b, float a) {
    m_BackgroundColor.r = r;
    m_BackgroundColor.g = g;
    m_BackgroundColor.b = b;
    m_BackgroundColor.a = a;
}
void Viewport::setAspectRatioSynced(bool synced) {
    (synced) ? m_StateFlags.add(StateFlags::AspectRatioSynced) : m_StateFlags.remove(StateFlags::AspectRatioSynced);
}
bool Viewport::isAspectRatioSynced() const {
    return m_StateFlags.has(StateFlags::AspectRatioSynced);
}
void Viewport::activate(bool active) {
    (active) ? m_StateFlags.add(StateFlags::Active) : m_StateFlags.remove(StateFlags::Active);
}
bool Viewport::isActive() const {
    return m_StateFlags.has(StateFlags::Active);
}
void Viewport::setCamera(const Camera& camera) {
    m_Camera = &const_cast<Camera&>(camera);
}
void Viewport::setViewportDimensions(float x, float y, float width, float height) {
    m_Viewport_Dimensions.x = x;
    m_Viewport_Dimensions.y = y;
    m_Viewport_Dimensions.z = width;
    m_Viewport_Dimensions.w = height;
}
void Viewport::setRenderFlag(ViewportRenderingFlag::Flag flag) {
    m_RenderFlags = flag;
}
void Viewport::addRenderFlag(ViewportRenderingFlag::Flag flag) {
    m_RenderFlags.add(flag);
}
void Viewport::removeRenderFlag(ViewportRenderingFlag::Flag flag) {
    m_RenderFlags.remove(flag);
}