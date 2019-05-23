#include <core/engine/renderer/postprocess/SSAO.h>
#include <core/engine/renderer/GBuffer.h>
#include <core/engine/renderer/FramebufferObject.h>

#include <core/ShaderProgram.h>
#include <core/Camera.h>

#include <glm/vec4.hpp>
#include <random>

using namespace Engine;
using namespace std;


epriv::Postprocess_SSAO epriv::Postprocess_SSAO::SSAO;

epriv::Postprocess_SSAO::Postprocess_SSAO() {
    m_ssao = true;
    m_ssao_do_blur = true;
    m_ssao_samples = 8;
    m_ssao_blur_num_passes = 2;
    m_ssao_blur_radius = 0.66f;
    m_ssao_blur_strength = 0.48f;
    m_ssao_scale = 1.0f;
    m_ssao_intensity = 1.8f;
    m_ssao_bias = 0.048f;
    m_ssao_radius = 0.175f;

    uniform_real_distribution<float> rand(0.0f, 1.0f);
    default_random_engine gen;
    for (uint i = 0; i < SSAO_KERNEL_COUNT; ++i) {
        glm::vec3 sample(rand(gen)*2.0f - 1.0f, rand(gen)*2.0f - 1.0f, rand(gen));
        sample = glm::normalize(sample);
        sample *= rand(gen);
        float scale = float(i) / float(SSAO_KERNEL_COUNT);
        float a = 0.1f;
        float b = 1.0f;
        float f = scale * scale;
        scale = a + f * (b - a);
        sample *= scale;
        m_ssao_Kernels[i] = sample;
    }
    vector<glm::vec3> ssaoNoise;
    for (uint i = 0; i < SSAO_NORMALMAP_SIZE * SSAO_NORMALMAP_SIZE; ++i) {
        ssaoNoise.emplace_back(rand(gen)*2.0 - 1.0, rand(gen)*2.0 - 1.0, 0.0f);
    }
    Renderer::genAndBindTexture(GL_TEXTURE_2D, m_ssao_noise_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SSAO_NORMALMAP_SIZE, SSAO_NORMALMAP_SIZE, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}
epriv::Postprocess_SSAO::~Postprocess_SSAO() {

}



void epriv::Postprocess_SSAO::pass(ShaderP& program, GBuffer& gbuffer, const uint& fboWidth, const uint& fboHeight, Camera& c) {
    program.bind();
    const float& divisor = gbuffer.getSmallFBO()->divisor();
    if (RenderManager::GLSL_VERSION < 140) {
        Renderer::sendUniformMatrix4Safe("CameraInvViewProj", c.getViewProjectionInverse());
        Renderer::sendUniformMatrix4Safe("CameraInvProj", c.getProjectionInverse());
        Renderer::sendUniform4Safe("CameraInfo1", glm::vec4(c.getPosition(), c.getNear()));
        Renderer::sendUniform4Safe("CameraInfo2", glm::vec4(c.getViewVector(), c.getFar()));
    }
    uint x = uint(float(fboWidth) * divisor);
    uint y = uint(float(fboHeight) * divisor);
    Renderer::sendUniform2("ScreenSize", (float)x, (float)y);
    Renderer::sendUniform4("SSAOInfo", m_ssao_radius, m_ssao_intensity, m_ssao_bias, m_ssao_scale);
    Renderer::sendUniform4("SSAOInfoA", 0, 0, m_ssao_samples, SSAO_NORMALMAP_SIZE);//change to 4f eventually?

    Renderer::sendTexture("gNormalMap", gbuffer.getTexture(GBufferType::Normal), 0);
    Renderer::sendTexture("gRandomMap", m_ssao_noise_texture, 1, GL_TEXTURE_2D);
    Renderer::sendTexture("gDepthMap", gbuffer.getTexture(GBufferType::Depth), 2);

    Renderer::renderFullscreenTriangle(x, y);
}


void epriv::Postprocess_SSAO::passBlur(ShaderP& program, GBuffer& gbuffer, const uint& fboWidth, const uint& fboHeight, string type, const GLuint& texture) {
    program.bind();
    const float& _divisor = gbuffer.getSmallFBO()->divisor();
    glm::vec2 hv(0.0f);
    if (type == "H") { hv = glm::vec2(1.0f, 0.0f); }
    else { hv = glm::vec2(0.0f, 1.0f); }
    glm::ivec2 Res(fboWidth, fboHeight);
    Renderer::sendUniform1("strengthModifier", m_ssao_blur_strength);
    Renderer::sendUniform2("Resolution", Res);
    Renderer::sendUniform4("Data", m_ssao_blur_radius, 0.0f, hv.x, hv.y);
    Renderer::sendTexture("image", gbuffer.getTexture(texture), 0);

    uint x = uint(float(fboWidth) * _divisor);
    uint y = uint(float(fboHeight) * _divisor);
    Renderer::renderFullscreenTriangle(x, y);
}




bool renderer::ssao::enabled() {
    return epriv::Postprocess_SSAO::SSAO.m_ssao;
}
void renderer::ssao::enable(bool b) {
    epriv::Postprocess_SSAO::SSAO.m_ssao = b;
}
void renderer::ssao::disable() {
    epriv::Postprocess_SSAO::SSAO.m_ssao = false;
}
void renderer::ssao::enableBlur(bool b) {
    epriv::Postprocess_SSAO::SSAO.m_ssao_do_blur = b;
}
void renderer::ssao::disableBlur() {
    epriv::Postprocess_SSAO::SSAO.m_ssao_do_blur = false;
}
float renderer::ssao::getBlurRadius() {
    return epriv::Postprocess_SSAO::SSAO.m_ssao_blur_radius;
}
void renderer::ssao::setBlurRadius(float r) {
    epriv::Postprocess_SSAO::SSAO.m_ssao_blur_radius = r;
}
float renderer::ssao::getBlurStrength() {
    return epriv::Postprocess_SSAO::SSAO.m_ssao_blur_strength;
}
void renderer::ssao::setBlurStrength(float s) {
    epriv::Postprocess_SSAO::SSAO.m_ssao_blur_strength = s;
}
float renderer::ssao::getIntensity() {
    return epriv::Postprocess_SSAO::SSAO.m_ssao_intensity;
}
void renderer::ssao::setIntensity(float i) {
    epriv::Postprocess_SSAO::SSAO.m_ssao_intensity = i;
}
float renderer::ssao::getRadius() {
    return epriv::Postprocess_SSAO::SSAO.m_ssao_radius;
}
void renderer::ssao::setRadius(float r) {
    epriv::Postprocess_SSAO::SSAO.m_ssao_radius = r;
}
float renderer::ssao::getScale() {
    return epriv::Postprocess_SSAO::SSAO.m_ssao_scale;
}
void renderer::ssao::setScale(float s) {
    epriv::Postprocess_SSAO::SSAO.m_ssao_scale = s;
}
float renderer::ssao::getBias() {
    return epriv::Postprocess_SSAO::SSAO.m_ssao_bias;
}
void renderer::ssao::setBias(float b) {
    epriv::Postprocess_SSAO::SSAO.m_ssao_bias = b;
}
unsigned int renderer::ssao::getSamples() {
    return epriv::Postprocess_SSAO::SSAO.m_ssao_samples;
}
void renderer::ssao::setSamples(unsigned int s) {
    epriv::Postprocess_SSAO::SSAO.m_ssao_samples = s;
}