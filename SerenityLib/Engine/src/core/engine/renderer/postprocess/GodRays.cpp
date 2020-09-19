#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/renderer/postprocess/GodRays.h>
#include <core/engine/renderer/Renderer.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/renderer/GBuffer.h>
#include <core/engine/renderer/FramebufferObject.h>
#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/shaders/Shader.h>
#include <core/engine/resources/Engine_BuiltInShaders.h>
#include <core/engine/threading/ThreadingModule.h>

Engine::priv::GodRays Engine::priv::GodRays::STATIC_GOD_RAYS;

bool Engine::priv::GodRays::init_shaders() {
    if (!m_GLSL_frag_code.empty())
        return false;

#pragma region GodRays
    m_GLSL_frag_code =
        "uniform vec4 RaysInfo;\n"//exposure | decay | density | weight
        "\n"
        "uniform vec2 lightPositionOnScreen;\n"
        "uniform SAMPLER_TYPE_2D firstPass;\n"
        "uniform int samples;\n"
        "\n"
        "uniform float alpha;\n"
        "varying vec2 texcoords;\n"
        "void main(){\n"
        "    vec2 uv = texcoords;\n"
        "    vec2 deltaUV = vec2(uv - lightPositionOnScreen);\n"
        "    deltaUV *= 1.0 /  float(samples) * RaysInfo.z;\n"
        "    float illuminationDecay = 1.0;\n"
        "    vec3 totalColor = vec3(0.0);\n"
        "    for(int i = 0; i < samples; ++i){\n"
        "        uv -= deltaUV / 2.0;\n"
        "        vec2 sampleData = texture2D(firstPass,uv).ba;\n"
        "        vec2 unpackedRG = Unpack2NibblesFrom8BitChannel(sampleData.r);\n"
        "        vec3 realSample = vec3(unpackedRG.r,unpackedRG.g,sampleData.g);\n"
        "        realSample *= illuminationDecay * RaysInfo.w;\n"
        "        totalColor += realSample;\n"
        "        illuminationDecay *= RaysInfo.y;\n"
        "    }\n"
        "    gl_FragColor.rgb = (totalColor * alpha) * RaysInfo.x;\n"
        "    gl_FragColor.a = 1.0;\n"
        "}";
#pragma endregion

    auto lambda_part_a = [&]() {
        m_Vertex_Shader   = std::make_unique<Shader>(Engine::priv::EShaders::fullscreen_quad_vertex, ShaderType::Vertex, false);
        m_Fragment_Shader = std::make_unique<Shader>(m_GLSL_frag_code, ShaderType::Fragment, false);
    };
    auto lambda_part_b = [&]() {
        m_Shader_Program  = std::make_unique<ShaderProgram>("GodRays", *m_Vertex_Shader, *m_Fragment_Shader);
    };
    Engine::priv::threading::addJobWithPostCallback(lambda_part_a, lambda_part_b);

    return true;
}
void Engine::priv::GodRays::pass(GBuffer& gbuffer, const Viewport& viewport, const glm::vec2& lightScrnPos, float alpha, const Engine::priv::Renderer& renderer) {
    const auto& dimensions = viewport.getViewportDimensions();
    renderer.bind(m_Shader_Program.get());
    Engine::Renderer::sendUniform4("RaysInfo", exposure, decay, density, weight);
    Engine::Renderer::sendUniform2("lightPositionOnScreen", lightScrnPos.x / dimensions.z, lightScrnPos.y / dimensions.w);
    Engine::Renderer::sendUniform1("samples", samples);
    Engine::Renderer::sendUniform1("alpha", alpha);
    Engine::Renderer::sendTexture("firstPass", gbuffer.getTexture(GBufferType::Misc), 0);

    Engine::Renderer::renderFullscreenQuad();
}
bool Engine::Renderer::godRays::enabled() {
    return Engine::priv::GodRays::STATIC_GOD_RAYS.godRays_active;
}
void Engine::Renderer::godRays::enable(bool enalbed) {
    Engine::priv::GodRays::STATIC_GOD_RAYS.godRays_active = enalbed;
}
void Engine::Renderer::godRays::disable() {
    Engine::priv::GodRays::STATIC_GOD_RAYS.godRays_active = false;
}
float Engine::Renderer::godRays::getExposure() {
    return Engine::priv::GodRays::STATIC_GOD_RAYS.exposure;
}
float Engine::Renderer::godRays::getFactor() {
    return Engine::priv::GodRays::STATIC_GOD_RAYS.factor;
}
float Engine::Renderer::godRays::getDecay() {
    return Engine::priv::GodRays::STATIC_GOD_RAYS.decay;
}
float Engine::Renderer::godRays::getDensity() {
    return Engine::priv::GodRays::STATIC_GOD_RAYS.density;
}
float Engine::Renderer::godRays::getWeight() {
    return Engine::priv::GodRays::STATIC_GOD_RAYS.weight;
}
unsigned int Engine::Renderer::godRays::getSamples() {
    return Engine::priv::GodRays::STATIC_GOD_RAYS.samples;
}
float Engine::Renderer::godRays::getFOVDegrees() {
    return Engine::priv::GodRays::STATIC_GOD_RAYS.fovDegrees;
}
float Engine::Renderer::godRays::getAlphaFalloff() {
    return Engine::priv::GodRays::STATIC_GOD_RAYS.alphaFalloff;
}
void Engine::Renderer::godRays::setExposure(float exposure) {
    Engine::priv::GodRays::STATIC_GOD_RAYS.exposure = exposure;
}
void Engine::Renderer::godRays::setFactor(float factor) {
    Engine::priv::GodRays::STATIC_GOD_RAYS.factor = factor;
}
void Engine::Renderer::godRays::setDecay(float decay) {
    Engine::priv::GodRays::STATIC_GOD_RAYS.decay = decay;
}
void Engine::Renderer::godRays::setDensity(float density) {
    Engine::priv::GodRays::STATIC_GOD_RAYS.density = density;
}
void Engine::Renderer::godRays::setWeight(float weight) {
    Engine::priv::GodRays::STATIC_GOD_RAYS.weight = weight;
}
void Engine::Renderer::godRays::setSamples(unsigned int numSamples) {
    Engine::priv::GodRays::STATIC_GOD_RAYS.samples = glm::max(0U, numSamples);
}
void Engine::Renderer::godRays::setFOVDegrees(float fovInDegrees) {
    Engine::priv::GodRays::STATIC_GOD_RAYS.fovDegrees = fovInDegrees;
}
void Engine::Renderer::godRays::setAlphaFalloff(float alphaFalloff) {
    Engine::priv::GodRays::STATIC_GOD_RAYS.alphaFalloff = alphaFalloff;
}
void Engine::Renderer::godRays::setSun(Entity* sunEntity) {
    Resources::getCurrentScene()->setGodRaysSun(sunEntity);
}
Entity* Engine::Renderer::godRays::getSun() {
    return Engine::Resources::getCurrentScene()->getGodRaysSun();
}