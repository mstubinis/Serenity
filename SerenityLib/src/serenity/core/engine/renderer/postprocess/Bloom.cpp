
#include <serenity/core/engine/renderer/postprocess/Bloom.h>
#include <serenity/core/engine/renderer/Renderer.h>
#include <serenity/core/engine/renderer/GBuffer.h>
#include <serenity/core/engine/renderer/FramebufferObject.h>
#include <serenity/core/engine/shaders/ShaderProgram.h>
#include <serenity/core/engine/shaders/Shader.h>
#include <serenity/core/engine/resources/Engine_BuiltInShaders.h>
#include <serenity/core/engine/threading/ThreadingModule.h>
#include <serenity/core/engine/scene/Viewport.h>
#include <serenity/core/engine/resources/Engine_Resources.h>

Engine::priv::Bloom Engine::priv::Bloom::bloom;

bool Engine::priv::Bloom::init_shaders() {
    if (!m_GLSL_frag_code.empty())
        return false;

#pragma region Bloom
    m_GLSL_frag_code = R"(
        const vec3 ConstantZeroVec3 = vec3(0.0, 0.0, 0.0);
        uniform SAMPLER_TYPE_2D SceneTexture;

        uniform vec4 Data; //x = scale y = threshold z = exposure w = UNUSED
        varying vec2 texcoords;

        void main(){
            vec3 sceneColor = texture2D(SceneTexture,texcoords).rgb;
            sceneColor = vec3(1.0) - exp(-sceneColor * Data.z); //exposure
            gl_FragColor.rgb = max(ConstantZeroVec3, sceneColor - vec3(Data.y)) * Data.x; //threshold   scale
        }
    )";
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
void Engine::priv::Bloom::pass(Engine::priv::GBuffer& gbuffer, const Viewport& viewport, uint32_t sceneTexture, const Engine::priv::RenderModule& renderer) {
    renderer.bind(m_Shader_Program.get<ShaderProgram>());

    Engine::Renderer::sendUniform4("Data", m_Scale, m_Threshold, m_Exposure, 0.0f);
    Engine::Renderer::sendTexture("SceneTexture", gbuffer.getTexture(sceneTexture), 0);

    Engine::Renderer::renderFullscreenQuad();

    Engine::Renderer::clearTexture(0, GL_TEXTURE_2D);
}