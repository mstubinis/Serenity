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
        "const vec3 ConstantZeroVec3 = vec3(0.0, 0.0, 0.0);\n"
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
        "    gl_FragColor.rgb = max(ConstantZeroVec3, sceneColor - vec3(Data.y)) * Data.x;\n"
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
void Engine::priv::Bloom::pass(Engine::priv::GBuffer& gbuffer, const Viewport& viewport, uint32_t sceneTexture, const Engine::priv::RenderModule& renderer) {
    renderer.bind(m_Shader_Program.get<ShaderProgram>());

    Engine::Renderer::sendUniform4("Data", m_Scale, m_Threshold, m_Exposure, 0.0f);
    Engine::Renderer::sendTexture("SceneTexture", gbuffer.getTexture(sceneTexture), 0);

    Engine::Renderer::renderFullscreenQuad();
}