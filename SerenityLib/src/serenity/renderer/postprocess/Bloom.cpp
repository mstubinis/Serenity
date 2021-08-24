
#include <serenity/renderer/postprocess/Bloom.h>
#include <serenity/renderer/Renderer.h>
#include <serenity/renderer/GBuffer.h>
#include <serenity/renderer/FramebufferObject.h>
#include <serenity/resources/shader/ShaderProgram.h>
#include <serenity/resources/shader/Shader.h>
#include <serenity/resources/Engine_BuiltInShaders.h>
#include <serenity/threading/ThreadingModule.h>
#include <serenity/scene/Viewport.h>
#include <serenity/resources/Engine_Resources.h>

Engine::priv::Bloom Engine::priv::Bloom::STATIC_BLOOM;

namespace {
    //uniform vec4 Data; //x = scale y = threshold z = exposure w = strength
    std::string internal_get_fragment_code() {
        return R"(
uniform SAMPLER_TYPE_2D SceneTexture;

uniform vec4 Data;
varying vec2 texcoords;

void main() {
    vec3 sceneColor = texture2D(SceneTexture, texcoords).rgb;
    sceneColor = vec3(1.0) - exp(-sceneColor * Data.z);
    vec3 finalColor = sceneColor - vec3(Data.y);
    finalColor = finalColor + (finalColor * Data.w);
    gl_FragColor.rgb = max(vec3(0.0), finalColor) * Data.x;
}
)";
    }
}

void Engine::priv::Bloom::init() {
    auto lambda_part_a = [this]() {
        m_Vertex_Shader   = Engine::Resources::addResource<Shader>(Engine::priv::EShaders::fullscreen_quad_vertex, ShaderType::Vertex, false);
        m_Fragment_Shader = Engine::Resources::addResource<Shader>(internal_get_fragment_code(), ShaderType::Fragment, false);
    };
    auto lambda_part_b = [this]() {
        m_Shader_Program  = Engine::Resources::addResource<ShaderProgram>("Bloom", m_Vertex_Shader, m_Fragment_Shader);
    };
    Engine::priv::threading::addJobWithPostCallback(lambda_part_a, lambda_part_b);
}
void Engine::priv::Bloom::pass(const Engine::priv::GBuffer& gbuffer, const Viewport& viewport, uint32_t sceneTexture, const Engine::priv::RenderModule& renderer) {
    renderer.bind(m_Shader_Program.get<ShaderProgram>());

    Engine::Renderer::sendUniform4("Data", m_Scale, m_Threshold, m_Exposure, m_Bloom_Strength);
    Engine::Renderer::sendTexture("SceneTexture", gbuffer.getTexture(sceneTexture), 0);

    Engine::Renderer::renderFullscreenQuad();

    Engine::Renderer::clearTexture(0, GL_TEXTURE_2D);
}