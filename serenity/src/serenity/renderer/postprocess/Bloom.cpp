
#include <serenity/renderer/postprocess/Bloom.h>
#include <serenity/renderer/Renderer.h>
#include <serenity/renderer/GBuffer.h>
#include <serenity/resources/shader/ShaderProgram.h>
#include <serenity/resources/shader/Shader.h>
#include <serenity/resources/Engine_BuiltInShaders.h>
#include <serenity/threading/ThreadingModule.h>
#include <serenity/scene/Viewport.h>
#include <serenity/resources/Engine_Resources.h>
#include <serenity/renderer/opengl/BindTextureRAII.h>

Engine::priv::Bloom Engine::priv::Bloom::STATIC_BLOOM;

namespace {
    //uniform vec4 Data; //x = scale y = threshold z = exposure w = strength
    std::string internal_get_fragment_code() {
        return R"(
uniform SAMPLER_TYPE_2D uSceneTextureForBlooming;

uniform vec4 Data;
varying vec2 texcoords;

void main() {
    vec2 uvs = ViewportUVCalculation(texcoords);
    vec3 sceneColor = texture2D(USE_SAMPLER_2D(uSceneTextureForBlooming), uvs).rgb;
    sceneColor = vec3(1.0) - exp(-sceneColor * Data.z);
    vec3 finalColor = sceneColor - vec3(Data.y);
    finalColor = finalColor + (finalColor * Data.w);
    gl_FragColor.rgb = max(vec3(0.0), finalColor) * Data.x;
}
)";
    }
}

void Engine::priv::Bloom::init() {
    auto lambda_part_a = []() {
        STATIC_BLOOM.m_Vertex_Shader   = Engine::Resources::addResource<Shader>(Engine::priv::EShaders::fullscreen_quad_vertex, ShaderType::Vertex);
        STATIC_BLOOM.m_Fragment_Shader = Engine::Resources::addResource<Shader>(internal_get_fragment_code(), ShaderType::Fragment);
    };
    auto lambda_part_b = []() {
        STATIC_BLOOM.m_Shader_Program  = Engine::Resources::addResource<ShaderProgram>("Bloom", STATIC_BLOOM.m_Vertex_Shader, STATIC_BLOOM.m_Fragment_Shader);
    };
    Engine::priv::threading::addJobWithPostCallback(lambda_part_a, lambda_part_b);
}
void Engine::priv::Bloom::pass(const Engine::priv::GBuffer& gbuffer, const Viewport& viewport, uint32_t sceneTexture, const Engine::priv::RenderModule& renderer) {
    const auto& viewportDimensions = viewport.getViewportDimensions();
    renderer.bind(m_Shader_Program.get<ShaderProgram>());

    Engine::Renderer::sendUniform4("Data", m_Scale, m_Threshold, m_Exposure, m_Bloom_Strength);

    Engine::priv::OpenGLBindTextureRAII uSceneTextureForBlooming{ "uSceneTextureForBlooming", gbuffer.getTexture(sceneTexture), 0, false };

    Engine::Renderer::renderFullscreenQuadCentered(viewportDimensions.z, viewportDimensions.w);
}