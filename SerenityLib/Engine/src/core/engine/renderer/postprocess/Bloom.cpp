#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/renderer/postprocess/Bloom.h>
#include <core/engine/renderer/Renderer.h>
#include <core/engine/renderer/GBuffer.h>
#include <core/engine/renderer/FramebufferObject.h>
#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/shaders/Shader.h>
#include <core/engine/resources/Engine_BuiltInShaders.h>
#include <core/engine/threading/ThreadingModule.h>
#include <core/engine/scene/Viewport.h>
#include <core/engine/resources/Engine_Resources.h>

Engine::priv::Bloom Engine::priv::Bloom::bloom;

bool Engine::priv::Bloom::init_shaders() {
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
        m_Vertex_Shader   = Engine::Resources::addResource<Shader>(Engine::priv::EShaders::fullscreen_quad_vertex, ShaderType::Vertex, false);
        m_Fragment_Shader = Engine::Resources::addResource<Shader>(m_GLSL_frag_code, ShaderType::Fragment, false);
    };
    auto lambda_part_b = [&]() {
        m_Shader_Program = Engine::Resources::addResource<ShaderProgram>("Bloom", m_Vertex_Shader, m_Fragment_Shader);
    };
    Engine::priv::threading::addJobWithPostCallback(lambda_part_a, lambda_part_b);

    return true;
}
void Engine::priv::Bloom::pass(Engine::priv::GBuffer& gbuffer, const Viewport& viewport, unsigned int sceneTexture, const Engine::priv::Renderer& renderer) {
    renderer.bind(m_Shader_Program.get<ShaderProgram>());

    Engine::Renderer::sendUniform4("Data", scale, threshold, exposure, 0.0f);
    Engine::Renderer::sendTexture("SceneTexture", gbuffer.getTexture(sceneTexture), 0);

    Engine::Renderer::renderFullscreenQuad();
}


float Engine::Renderer::bloom::getThreshold() {
    return Engine::priv::Bloom::bloom.threshold;
}
void Engine::Renderer::bloom::setThreshold(float t) {
    Engine::priv::Bloom::bloom.threshold = t;
}
float Engine::Renderer::bloom::getExposure() {
    return Engine::priv::Bloom::bloom.exposure;
}
void Engine::Renderer::bloom::setExposure(float e) {
    Engine::priv::Bloom::bloom.exposure = e;
}
bool Engine::Renderer::bloom::enabled() {
    return Engine::priv::Bloom::bloom.bloom_active;
}
unsigned int Engine::Renderer::bloom::getNumPasses() {
    return Engine::priv::Bloom::bloom.num_passes;
}
void Engine::Renderer::bloom::setNumPasses(unsigned int p) {
    Engine::priv::Bloom::bloom.num_passes = p;
}
void Engine::Renderer::bloom::enable(bool b) {
    Engine::priv::Bloom::bloom.bloom_active = b;
}
void Engine::Renderer::bloom::disable() {
    Engine::priv::Bloom::bloom.bloom_active = false;
}
float Engine::Renderer::bloom::getBlurRadius() {
    return Engine::priv::Bloom::bloom.blur_radius;
}
float Engine::Renderer::bloom::getBlurStrength() {
    return Engine::priv::Bloom::bloom.blur_strength;
}
void Engine::Renderer::bloom::setBlurRadius(float r) {
    Engine::priv::Bloom::bloom.blur_radius = glm::max(0.0f, r);
}
void Engine::Renderer::bloom::setBlurStrength(float r) {
    Engine::priv::Bloom::bloom.blur_strength = glm::max(0.0f, r);
}
float Engine::Renderer::bloom::getScale() {
    return Engine::priv::Bloom::bloom.scale;
}
void Engine::Renderer::bloom::setScale(float s) {
    Engine::priv::Bloom::bloom.scale = glm::max(0.0f, s);
}