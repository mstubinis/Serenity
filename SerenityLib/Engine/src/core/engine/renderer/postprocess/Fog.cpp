#include <core/engine/renderer/postprocess/FOG.h>
#include <core/engine/renderer/Engine_Renderer.h>

#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/math/Engine_Math.h>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

Fog Fog::fog;

Fog::Fog() {
    fog_active = false;
    distNull   = 5.0f;
    distBlend  = 50.0f;
    color      = glm::vec4(1.0f, 1.0f, 1.0f, 0.97f);
}
Fog::~Fog() {
}
const bool Renderer::fog::enabled() {
    return Fog::fog.fog_active;
}
void Renderer::fog::enable(const bool b) {
    Fog::fog.fog_active = b;
}
void Renderer::fog::disable() {
    Fog::fog.fog_active = false;
}
void Renderer::fog::setColor(const glm::vec4& color) {
    Renderer::fog::setColor(color.r, color.g, color.b, color.a);
}
void Renderer::fog::setColor(const float r, const float g, const float b, const float a) {
    Math::setColor(Fog::fog.color, r, g, b, a);
}
void Renderer::fog::setNullDistance(const float d) {
    Fog::fog.distNull = d;
}
void Renderer::fog::setBlendDistance(const float d) {
    Fog::fog.distBlend = d;
}
const float Renderer::fog::getNullDistance() {
    return Fog::fog.distNull;
}
const float Renderer::fog::getBlendDistance() {
    return Fog::fog.distBlend;
}