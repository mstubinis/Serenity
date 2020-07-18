#include "core/engine/utils/PrecompiledHeader.h"
#include <core/engine/renderer/postprocess/FOG.h>
#include <core/engine/renderer/Renderer.h>

#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/math/Engine_Math.h>

using namespace std;

Engine::priv::Fog Engine::priv::Fog::fog;

Engine::priv::Fog::~Fog() {
}
const bool Engine::Renderer::fog::enabled() {
    return Engine::priv::Fog::fog.fog_active;
}
void Engine::Renderer::fog::enable(const bool b) {
    Engine::priv::Fog::fog.fog_active = b;
}
void Engine::Renderer::fog::disable() {
    Engine::priv::Fog::fog.fog_active = false;
}
void Engine::Renderer::fog::setColor(const glm::vec4& color) {
    Engine::Renderer::fog::setColor(color.r, color.g, color.b, color.a);
}
void Engine::Renderer::fog::setColor(const float r, const float g, const float b, const float a) {
    Engine::Math::setColor(Engine::priv::Fog::fog.color, r, g, b, a);
}
void Engine::Renderer::fog::setNullDistance(const float d) {
    Engine::priv::Fog::fog.distNull = d;
}
void Engine::Renderer::fog::setBlendDistance(const float d) {
    Engine::priv::Fog::fog.distBlend = d;
}
const float Engine::Renderer::fog::getNullDistance() {
    return Engine::priv::Fog::fog.distNull;
}
const float Engine::Renderer::fog::getBlendDistance() {
    return Engine::priv::Fog::fog.distBlend;
}