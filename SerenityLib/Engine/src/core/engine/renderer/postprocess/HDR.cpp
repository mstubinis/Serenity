#include <core/engine/renderer/postprocess/HDR.h>
#include <core/engine/renderer/GBuffer.h>
#include <core/engine/renderer/Engine_Renderer.h>

#include <core/engine/shaders/ShaderProgram.h>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

HDR HDR::hdr;

HDR::HDR() {
    hdr_active = true;
    exposure   = 3.0f;
    algorithm  = HDRAlgorithm::Uncharted;
}
HDR::~HDR() {
}
void HDR::pass(ShaderProgram& program,GBuffer& gbuffer,const unsigned int& fboWidth,const unsigned int& fboHeight,const bool& godRays,const bool& lighting,const float& godRaysFactor) {
    program.bind();

    Renderer::sendUniform4Safe("HDRInfo", exposure, static_cast<float>(hdr_active), godRaysFactor, static_cast<float>(algorithm));
    Renderer::sendUniform2Safe("Has", static_cast<int>(godRays), static_cast<int>(lighting));

    Renderer::sendTextureSafe("lightingBuffer", gbuffer.getTexture(GBufferType::Lighting), 0);
    Renderer::sendTextureSafe("gDiffuseMap", gbuffer.getTexture(GBufferType::Diffuse), 1);
    Renderer::sendTextureSafe("gNormalMap", gbuffer.getTexture(GBufferType::Normal), 2);
    Renderer::sendTextureSafe("gGodsRaysMap", gbuffer.getTexture(GBufferType::GodRays), 3);

    Renderer::renderFullscreenTriangle(fboWidth, fboHeight);
}
const bool Renderer::hdr::enabled() {
    return HDR::hdr.hdr_active;
}
void Renderer::hdr::enable(const bool b) {
    HDR::hdr.hdr_active = b;
}
void Renderer::hdr::disable() {
    HDR::hdr.hdr_active = false;
}
const float Renderer::hdr::getExposure() {
    return HDR::hdr.exposure;
}
void Renderer::hdr::setExposure(const float e) {
    HDR::hdr.exposure = e;
}
void Renderer::hdr::setAlgorithm(const HDRAlgorithm::Algorithm a) {
    HDR::hdr.algorithm = a;
}