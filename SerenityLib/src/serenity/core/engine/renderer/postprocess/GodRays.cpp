
#include <serenity/core/engine/renderer/postprocess/GodRays.h>
#include <serenity/core/engine/renderer/Renderer.h>
#include <serenity/core/engine/resources/Engine_Resources.h>
#include <serenity/core/engine/scene/Scene.h>
#include <serenity/core/engine/renderer/GBuffer.h>
#include <serenity/core/engine/renderer/FramebufferObject.h>
#include <serenity/core/engine/shaders/ShaderProgram.h>
#include <serenity/core/engine/shaders/Shader.h>
#include <serenity/core/engine/resources/Engine_BuiltInShaders.h>
#include <serenity/core/engine/threading/ThreadingModule.h>

Engine::priv::GodRays Engine::priv::GodRays::STATIC_GOD_RAYS;

bool Engine::priv::GodRays::init_shaders() {
    if (!m_GLSL_frag_code.empty())
        return false;

#pragma region GodRays
    m_GLSL_frag_code = R"(
        uniform vec4 RaysInfo; //exposure | decay | density | weight

        uniform vec2 lightPositionOnScreen;
        uniform SAMPLER_TYPE_2D firstPass;
        uniform int samples;

        uniform float alpha;
        varying vec2 texcoords;
        void main(){
            vec2 uv = texcoords;
            vec2 deltaUV = vec2(uv - lightPositionOnScreen);
            deltaUV *= 1.0 /  float(samples) * RaysInfo.z;
            float illuminationDecay = 1.0;
            vec3 totalColor = vec3(0.0);
            for(int i = 0; i < samples; ++i){
                uv -= deltaUV / 2.0;
                vec2 sampleData = texture2D(firstPass,uv).ba;
                vec2 unpackedRG = Unpack2NibblesFrom8BitChannel(sampleData.r);
                vec3 realSample = vec3(unpackedRG.r,unpackedRG.g,sampleData.g);
                realSample *= illuminationDecay * RaysInfo.w;
                totalColor += realSample;
                illuminationDecay *= RaysInfo.y;
            }
            gl_FragColor.rgb = (totalColor * alpha) * RaysInfo.x;
            gl_FragColor.a = 1.0;
        }
    )";
#pragma endregion

    auto lambda_part_a = [&]() {
        m_Vertex_Shader   = Engine::Resources::addResource<Shader>(Engine::priv::EShaders::fullscreen_quad_vertex, ShaderType::Vertex, false);
        m_Fragment_Shader = Engine::Resources::addResource<Shader>(m_GLSL_frag_code, ShaderType::Fragment, false);
    };
    auto lambda_part_b = [&]() {
        m_Shader_Program  = Engine::Resources::addResource<ShaderProgram>("GodRays", m_Vertex_Shader, m_Fragment_Shader);
    };
    Engine::priv::threading::addJobWithPostCallback(lambda_part_a, lambda_part_b);

    return true;
}
void Engine::priv::GodRays::pass(GBuffer& gbuffer, const Viewport& viewport, const glm::vec2& lightScrnPos, float alpha, const RenderModule& renderer) {
    const auto& dimensions = viewport.getViewportDimensions();
    renderer.bind(m_Shader_Program.get<ShaderProgram>());
    Engine::Renderer::sendUniform4("RaysInfo", exposure, decay, density, weight);
    Engine::Renderer::sendUniform2("lightPositionOnScreen", lightScrnPos.x / dimensions.z, lightScrnPos.y / dimensions.w);
    Engine::Renderer::sendUniform1("samples", samples);
    Engine::Renderer::sendUniform1("alpha", alpha);
    Engine::Renderer::sendTexture("firstPass", gbuffer.getTexture(GBufferType::Misc), 0);

    Engine::Renderer::renderFullscreenQuad();

    Engine::Renderer::clearTexture(0, GL_TEXTURE_2D);
}

void Engine::Renderer::godRays::setSun(Entity sunEntity) noexcept {
    Resources::getCurrentScene()->setGodRaysSun(sunEntity);
}
Entity Engine::Renderer::godRays::getSun() noexcept {
    return Engine::Resources::getCurrentScene()->getGodRaysSun();
}