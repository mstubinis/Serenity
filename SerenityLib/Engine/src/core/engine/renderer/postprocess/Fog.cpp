#include <core/engine/renderer/postprocess/FOG.h>
#include <core/engine/renderer/Engine_Renderer.h>

#include <core/engine/renderer/GBuffer.h>
#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/math/Engine_Math.h>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

Postprocess_Fog Postprocess_Fog::Fog;

Postprocess_Fog::Postprocess_Fog() {
    fog       = false;
    distNull  = 5.0f;
    distBlend = 50.0f;
    color     = glm::vec4(1.0f, 1.0f, 1.0f, 0.97f);
}
Postprocess_Fog::~Postprocess_Fog() {

}

const bool Renderer::fog::enabled() {
    return Postprocess_Fog::Fog.fog;
}
void Renderer::fog::enable(const bool b) {
    Postprocess_Fog::Fog.fog = b;
}
void Renderer::fog::disable() {
    Postprocess_Fog::Fog.fog = false;
}
void Renderer::fog::setColor(const glm::vec4& color) {
    Renderer::fog::setColor(color.r, color.g, color.b, color.a);
}
void Renderer::fog::setColor(const float r, const float g, const float b, const float a) {
    Math::setColor(Postprocess_Fog::Fog.color, r, g, b, a);
}
void Renderer::fog::setNullDistance(const float d) {
    Postprocess_Fog::Fog.distNull = d;
}
void Renderer::fog::setBlendDistance(const float d) {
    Postprocess_Fog::Fog.distBlend = d;
}
const float Renderer::fog::getNullDistance() {
    return Postprocess_Fog::Fog.distNull;
}
const float Renderer::fog::getBlendDistance() {
    return Postprocess_Fog::Fog.distBlend;
}