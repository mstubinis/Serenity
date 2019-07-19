#include <core/engine/renderer/postprocess/FOG.h>
#include <core/engine/renderer/Engine_Renderer.h>

#include <core/engine/renderer/GBuffer.h>
#include <core/ShaderProgram.h>
#include <core/engine/math/Engine_Math.h>

using namespace Engine;

epriv::Postprocess_Fog epriv::Postprocess_Fog::Fog;

epriv::Postprocess_Fog::Postprocess_Fog() {
    fog       = false;
    distNull  = 5.0f;
    distBlend = 65.0f;
    color     = glm::vec4(1.0f, 1.0f, 1.0f, 0.95f);
}
epriv::Postprocess_Fog::~Postprocess_Fog() {

}



bool Renderer::fog::enabled() { 
    return epriv::Postprocess_Fog::Fog.fog;
}
void Renderer::fog::enable(const bool b) {
    epriv::Postprocess_Fog::Fog.fog = b;
}
void Renderer::fog::disable() {
    epriv::Postprocess_Fog::Fog.fog = false;
}
void Renderer::fog::setColor(const glm::vec4& color) {
    Renderer::fog::setColor(color.r, color.g, color.b, color.a);
}
void Renderer::fog::setColor(const float r, const float g, const float b, const float a) {
    Math::setColor(epriv::Postprocess_Fog::Fog.color, r, g, b, a);
}
void Renderer::fog::setNullDistance(const float d) {
    epriv::Postprocess_Fog::Fog.distNull = d;
}
void Renderer::fog::setBlendDistance(const float d) {
    epriv::Postprocess_Fog::Fog.distBlend = d;
}
float Renderer::fog::getNullDistance() {
    return epriv::Postprocess_Fog::Fog.distNull;
}
float Renderer::fog::getBlendDistance() {
    return epriv::Postprocess_Fog::Fog.distBlend;
}