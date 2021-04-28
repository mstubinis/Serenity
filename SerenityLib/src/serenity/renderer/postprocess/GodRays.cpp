
#include <serenity/renderer/postprocess/GodRays.h>
#include <serenity/renderer/Renderer.h>
#include <serenity/resources/Engine_Resources.h>
#include <serenity/scene/Scene.h>
#include <serenity/renderer/GBuffer.h>
#include <serenity/renderer/FramebufferObject.h>
#include <serenity/resources/shader/ShaderProgram.h>
#include <serenity/resources/shader/Shader.h>
#include <serenity/resources/Engine_BuiltInShaders.h>
#include <serenity/threading/ThreadingModule.h>

Engine::priv::GodRays Engine::priv::GodRays::STATIC_GOD_RAYS;

void Engine::priv::GodRays::internal_init_fragment_code() {
    STATIC_GOD_RAYS.m_GLSL_frag_code = R"(
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
        vec2 sampleData = texture2D(firstPass, uv).ba;
        vec2 unpackedRG = Unpack2NibblesFrom8BitChannel(sampleData.r);
        vec3 realSample = vec3(unpackedRG.r, unpackedRG.g, sampleData.g);
        realSample *= illuminationDecay * RaysInfo.w;
        totalColor += realSample;
        illuminationDecay *= RaysInfo.y;
    }
    gl_FragColor.rgb = (totalColor * alpha) * RaysInfo.x;
    gl_FragColor.a = 1.0;
}
    )";
}
bool Engine::priv::GodRays::init() {
    if (!STATIC_GOD_RAYS.m_GLSL_frag_code.empty())
        return false;

    STATIC_GOD_RAYS.internal_init_fragment_code();

    auto lambda_part_a = []() {
        STATIC_GOD_RAYS.m_Vertex_Shader   = Engine::Resources::addResource<Shader>(Engine::priv::EShaders::fullscreen_quad_vertex, ShaderType::Vertex, false);
        STATIC_GOD_RAYS.m_Fragment_Shader = Engine::Resources::addResource<Shader>(STATIC_GOD_RAYS.m_GLSL_frag_code, ShaderType::Fragment, false);
    };
    auto lambda_part_b = []() {
        STATIC_GOD_RAYS.m_Shader_Program  = Engine::Resources::addResource<ShaderProgram>("GodRays", STATIC_GOD_RAYS.m_Vertex_Shader, STATIC_GOD_RAYS.m_Fragment_Shader);
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