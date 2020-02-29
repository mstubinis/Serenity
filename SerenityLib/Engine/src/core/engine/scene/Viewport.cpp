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
    const auto& winSize = Resources::getWindowSize();
    m_Scene             = &const_cast<Scene&>(scene);

    setCamera(camera);
    setViewportDimensions(0.0f, 0.0f, static_cast<float>(winSize.x), static_cast<float>(winSize.y));
}
Viewport::~Viewport() {
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


const unsigned int Viewport::id() const {
    return m_ID;
}
void Viewport::setID(const unsigned int id) {
    m_ID = id;
}


const float Viewport::getDepthMaskValue() const {
    return m_DepthMaskValue;
}
void Viewport::setDepthMaskValue(const float depth) {
    m_DepthMaskValue = depth;
}

void Viewport::activateDepthMask(const bool active) {
    if (active) {
        m_StateFlags.add(StateFlags::DepthMaskActive);
    }else{
        m_StateFlags.remove(StateFlags::DepthMaskActive);
    }
}
const bool Viewport::isDepthMaskActive() const {
    return m_StateFlags.has(StateFlags::DepthMaskActive);
}
const glm::vec4& Viewport::getBackgroundColor() const {
    return m_BackgroundColor;
}
void Viewport::setBackgroundColor(const float r, const float g, const float b, const float a) {
    m_BackgroundColor.r = r;
    m_BackgroundColor.g = g;
    m_BackgroundColor.b = b;
    m_BackgroundColor.a = a;
}

void Viewport::setAspectRatioSynced(const bool synced) {
    if (synced) {
        m_StateFlags.add(StateFlags::AspectRatioSynced);
    }else{
        m_StateFlags.remove(StateFlags::AspectRatioSynced);
    }
}
const bool Viewport::isAspectRatioSynced() const {
    return m_StateFlags.has(StateFlags::AspectRatioSynced);
}

const Scene& Viewport::getScene() const {
    return *m_Scene;
}
const Camera& Viewport::getCamera() const {
    return *m_Camera;
}
void Viewport::activate(const bool active) {
    if (active) {
        m_StateFlags.add(StateFlags::Active);
    }else{
        m_StateFlags.remove(StateFlags::Active);
    }
}
const bool Viewport::isActive() const {
    return m_StateFlags.has(StateFlags::Active);
}
void Viewport::setCamera(const Camera& camera) {
    m_Camera = &const_cast<Camera&>(camera);
}
void Viewport::setViewportDimensions(const float x, const float y, const float width, const float height) {
    m_Viewport_Dimensions.x = x;
    m_Viewport_Dimensions.y = y;
    m_Viewport_Dimensions.z = width;
    m_Viewport_Dimensions.w = height;
}
const glm::vec4& Viewport::getViewportDimensions() const {
    return m_Viewport_Dimensions;
}
const unsigned short Viewport::getRenderFlags() const {
    return m_RenderFlags.get();
}
void Viewport::setRenderFlag(const ViewportRenderingFlag::Flag flag) {
    m_RenderFlags = flag;
}
void Viewport::addRenderFlag(const ViewportRenderingFlag::Flag flag) {
    m_RenderFlags.add(flag);
}
void Viewport::removeRenderFlag(const ViewportRenderingFlag::Flag flag) {
    m_RenderFlags.remove(flag);
}