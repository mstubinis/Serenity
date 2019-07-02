#include <core/engine/renderer/postprocess/HDR.h>
#include <core/engine/renderer/GBuffer.h>
#include <core/engine/renderer/Engine_Renderer.h>

#include <core/ShaderProgram.h>

using namespace Engine;
using namespace std;

epriv::Postprocess_HDR epriv::Postprocess_HDR::HDR;


epriv::Postprocess_HDR::Postprocess_HDR() {
    hdr       = true;
    exposure  = 3.0f;
    algorithm = HDRAlgorithm::Uncharted;
}
epriv::Postprocess_HDR::~Postprocess_HDR() {
}
void epriv::Postprocess_HDR::pass(ShaderP& program,GBuffer& gbuffer,const unsigned int&fboWidth,const unsigned int& fboHeight,const bool& godRays,const bool& lighting,const float& godRaysFactor) {
    program.bind();

    Renderer::sendUniform4Safe("HDRInfo", exposure, static_cast<float>(hdr), godRaysFactor, static_cast<float>(algorithm));
    Renderer::sendUniform2Safe("Has", static_cast<int>(godRays), static_cast<int>(lighting));

    Renderer::sendTextureSafe("lightingBuffer", gbuffer.getTexture(GBufferType::Lighting), 0);
    Renderer::sendTextureSafe("gDiffuseMap", gbuffer.getTexture(GBufferType::Diffuse), 1);
    Renderer::sendTextureSafe("gNormalMap", gbuffer.getTexture(GBufferType::Normal), 2);
    Renderer::sendTextureSafe("gGodsRaysMap", gbuffer.getTexture(GBufferType::GodRays), 3);
    Renderer::renderFullscreenTriangle(fboWidth, fboHeight);
}


bool Engine::Renderer::hdr::enabled() {
    return epriv::Postprocess_HDR::HDR.hdr;
}
void Engine::Renderer::hdr::enable(const bool b) {
    epriv::Postprocess_HDR::HDR.hdr = b;
}
void Engine::Renderer::hdr::disable() {
    epriv::Postprocess_HDR::HDR.hdr = false;
}
float Engine::Renderer::hdr::getExposure() {
    return epriv::Postprocess_HDR::HDR.exposure;
}
void Engine::Renderer::hdr::setExposure(const float e) {
    epriv::Postprocess_HDR::HDR.exposure = e;
}
void Engine::Renderer::hdr::setAlgorithm(const HDRAlgorithm::Algorithm a) {
    epriv::Postprocess_HDR::HDR.algorithm = a;
}