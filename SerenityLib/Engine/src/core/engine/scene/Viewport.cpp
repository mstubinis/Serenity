#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/scene/Viewport.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/renderer/Renderer.h>
#include <core/engine/resources/Engine_Resources.h>

Viewport::Viewport() {
    m_RenderFlags = ViewportRenderingFlag::_ALL;
    activate();
    setAspectRatioSynced(true);
    activateDepthMask(false);
}
Viewport::Viewport(Scene& scene, Camera& camera) 
    : Viewport{}
{
    auto winSize{ Engine::Resources::getWindowSize() };
    m_Scene       = &scene;

    setCamera(camera);
    setViewportDimensions(0.0f, 0.0f, (float)winSize.x, (float)winSize.y);
}
Viewport::Viewport(Viewport&& other) noexcept 
    : m_Scene{ std::move(other.m_Scene) }
    , m_Camera{ std::move(other.m_Camera) }
    , m_Viewport_Dimensions{ std::move(other.m_Viewport_Dimensions) }
    , m_BackgroundColor{ std::move(other.m_BackgroundColor) }
    , m_StateFlags{ std::move(other.m_StateFlags) }
    , m_DepthMaskValue{ std::move(other.m_DepthMaskValue) }
    , m_ID{ std::move(other.m_ID) }
    , m_RenderFlags{ std::move(other.m_RenderFlags) }
{}
Viewport& Viewport::operator=(Viewport&& other) noexcept {
    m_Scene               = std::move(other.m_Scene);
    m_Camera              = std::move(other.m_Camera);
    m_Viewport_Dimensions = std::move(other.m_Viewport_Dimensions);
    m_BackgroundColor     = std::move(other.m_BackgroundColor);
    m_StateFlags          = std::move(other.m_StateFlags);
    m_DepthMaskValue      = std::move(other.m_DepthMaskValue);
    m_ID                  = std::move(other.m_ID);
    m_RenderFlags         = std::move(other.m_RenderFlags);
    return *this;
}

void Viewport::activateDepthMask(bool active) {
    (active) ? m_StateFlags.add(StateFlags::DepthMaskActive) : m_StateFlags.remove(StateFlags::DepthMaskActive);
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
void Viewport::setViewportDimensions(float x, float y, float width, float height) {
    m_Viewport_Dimensions.x = x;
    m_Viewport_Dimensions.y = y;
    m_Viewport_Dimensions.z = width;
    m_Viewport_Dimensions.w = height;
}