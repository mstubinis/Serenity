
#include <serenity/renderer/postprocess/FOG.h>
#include <serenity/renderer/Renderer.h>
#include <serenity/resources/shader/ShaderProgram.h>
#include <serenity/math/Engine_Math.h>

Engine::priv::Fog Engine::priv::Fog::STATIC_FOG;

bool Engine::Renderer::fog::enabled() {
    return Engine::priv::Fog::STATIC_FOG.fog_active;
}
void Engine::Renderer::fog::enable(bool enabled) {
    Engine::priv::Fog::STATIC_FOG.fog_active = enabled;
}
void Engine::Renderer::fog::disable() {
    Engine::priv::Fog::STATIC_FOG.fog_active = false;
}
void Engine::Renderer::fog::setColor(const glm::vec4& color) {
    Engine::Renderer::fog::setColor(color.r, color.g, color.b, color.a);
}
void Engine::Renderer::fog::setColor(float r, float g, float b, float a) {
    Engine::Math::setColor(Engine::priv::Fog::STATIC_FOG.color, r, g, b, a);
}
void Engine::Renderer::fog::setNullDistance(float nullDistance) {
    Engine::priv::Fog::STATIC_FOG.distNull = nullDistance;
}
void Engine::Renderer::fog::setBlendDistance(float blendDistance) {
    Engine::priv::Fog::STATIC_FOG.distBlend = blendDistance;
}
float Engine::Renderer::fog::getNullDistance() {
    return Engine::priv::Fog::STATIC_FOG.distNull;
}
float Engine::Renderer::fog::getBlendDistance() {
    return Engine::priv::Fog::STATIC_FOG.distBlend;
}