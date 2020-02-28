#include <core/engine/renderer/postprocess/Bloom.h>
#include <core/engine/renderer/Renderer.h>
#include <core/engine/renderer/GBuffer.h>
#include <core/engine/renderer/FramebufferObject.h>
#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/shaders/Shader.h>
#include <core/engine/resources/Engine_BuiltInShaders.h>
#include <core/engine/threading/Engine_ThreadManager.h>
#include <core/engine/scene/Viewport.h>

#include <glm/common.hpp>

using namespace std;

Engine::priv::Bloom Engine::priv::Bloom::bloom;

Engine::priv::Bloom::~Bloom() {
    SAFE_DELETE(m_Vertex_Shader);
    SAFE_DELETE(m_Fragment_Shader);
    SAFE_DELETE(m_Shader_Program);
}
const bool Engine::priv::Bloom::init_shaders() {
    if (!m_GLSL_frag_code.empty())
        return false;

#pragma region Bloom
    m_GLSL_frag_code =
        "const vec3 ConstantZeroVec3 = vec3(0.0,0.0,0.0);\n"
        "uniform SAMPLER_TYPE_2D SceneTexture;\n"
        "\n"
        "uniform vec4 Data;\n" //x = scale y = threshold z = exposure w = UNUSED
        "varying vec2 texcoords;\n"
        "\n"
        "void main(){\n"
        "    vec3 sceneColor = texture2D(SceneTexture,texcoords).rgb;\n"
        //                                               exposure
        "    sceneColor = vec3(1.0) - exp(-sceneColor * Data.z);\n"
        //                                                         threshold       scale
        "    gl_FragColor.rgb = max(ConstantZeroVec3,sceneColor - vec3(Data.y)) * Data.x;\n"
        "}";
#pragma endregion

    auto lambda_part_a = [&]() {
        m_Vertex_Shader = NEW Shader(Engine::priv::EShaders::fullscreen_quad_vertex, ShaderType::Vertex, false);
        m_Fragment_Shader = NEW Shader(m_GLSL_frag_code, ShaderType::Fragment, false);
    };
    auto lambda_part_b = [&]() {
        m_Shader_Program = NEW ShaderProgram("Bloom", *m_Vertex_Shader, *m_Fragment_Shader);
    };
    Engine::priv::threading::addJobWithPostCallback(lambda_part_a, lambda_part_b);

    return true;
}
void Engine::priv::Bloom::pass(Engine::priv::GBuffer& gbuffer, const Viewport& viewport, const unsigned int sceneTexture, const Engine::priv::Renderer& renderer) {
    renderer._bindShaderProgram(m_Shader_Program);

    Engine::Renderer::sendUniform4("Data", scale, threshold, exposure, 0.0f);
    Engine::Renderer::sendTexture("SceneTexture", gbuffer.getTexture(sceneTexture), 0);

    Engine::Renderer::renderFullscreenQuad();
}


const float Engine::Renderer::bloom::getThreshold() {
    return Engine::priv::Bloom::bloom.threshold;
}
void Engine::Renderer::bloom::setThreshold(const float t) {
    Engine::priv::Bloom::bloom.threshold = t;
}
const float Engine::Renderer::bloom::getExposure() {
    return Engine::priv::Bloom::bloom.exposure;
}
void Engine::Renderer::bloom::setExposure(const float e) {
    Engine::priv::Bloom::bloom.exposure = e;
}
const bool Engine::Renderer::bloom::enabled() {
    return Engine::priv::Bloom::bloom.bloom_active;
}
const unsigned int Engine::Renderer::bloom::getNumPasses() {
    return Engine::priv::Bloom::bloom.num_passes;
}
void Engine::Renderer::bloom::setNumPasses(const unsigned int p) {
    Engine::priv::Bloom::bloom.num_passes = p;
}
void Engine::Renderer::bloom::enable(const bool b) {
    Engine::priv::Bloom::bloom.bloom_active = b;
}
void Engine::Renderer::bloom::disable() {
    Engine::priv::Bloom::bloom.bloom_active = false;
}
const float Engine::Renderer::bloom::getBlurRadius() {
    return Engine::priv::Bloom::bloom.blur_radius;
}
const float Engine::Renderer::bloom::getBlurStrength() {
    return Engine::priv::Bloom::bloom.blur_strength;
}
void Engine::Renderer::bloom::setBlurRadius(const float r) {
    Engine::priv::Bloom::bloom.blur_radius = glm::max(0.0f, r);
}
void Engine::Renderer::bloom::setBlurStrength(const float r) {
    Engine::priv::Bloom::bloom.blur_strength = glm::max(0.0f, r);
}
const float Engine::Renderer::bloom::getScale() {
    return Engine::priv::Bloom::bloom.scale;
}
void Engine::Renderer::bloom::setScale(const float s) {
    Engine::priv::Bloom::bloom.scale = glm::max(0.0f, s);
}