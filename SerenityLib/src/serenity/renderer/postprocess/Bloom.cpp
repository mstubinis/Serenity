
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

void Engine::priv::Bloom::internal_init_fragment_code() {
    STATIC_BLOOM.m_GLSL_frag_code = R"(
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
}
bool Engine::priv::Bloom::init() {
    if (!STATIC_BLOOM.m_GLSL_frag_code.empty())
        return false;

    STATIC_BLOOM.internal_init_fragment_code();

    auto lambda_part_a = []() {
        STATIC_BLOOM.m_Vertex_Shader   = Engine::Resources::addResource<Shader>(Engine::priv::EShaders::fullscreen_quad_vertex, ShaderType::Vertex, false);
        STATIC_BLOOM.m_Fragment_Shader = Engine::Resources::addResource<Shader>(STATIC_BLOOM.m_GLSL_frag_code, ShaderType::Fragment, false);
    };
    auto lambda_part_b = []() {
        STATIC_BLOOM.m_Shader_Program = Engine::Resources::addResource<ShaderProgram>("Bloom", STATIC_BLOOM.m_Vertex_Shader, STATIC_BLOOM.m_Fragment_Shader);
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