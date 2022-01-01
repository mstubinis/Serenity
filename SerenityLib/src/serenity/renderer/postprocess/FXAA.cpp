
#include <serenity/renderer/postprocess/FXAA.h>
#include <serenity/renderer/Renderer.h>
#include <serenity/threading/ThreadingModule.h>
#include <serenity/renderer/GBuffer.h>
#include <serenity/resources/shader/ShaderProgram.h>
#include <serenity/resources/shader/Shader.h>
#include <serenity/scene/Viewport.h>
#include <serenity/resources/Engine_BuiltInShaders.h>
#include <serenity/resources/Engine_Resources.h>
#include <serenity/renderer/opengl/BindTextureRAII.h>

Engine::priv::FXAA Engine::priv::FXAA::STATIC_FXAA;

void Engine::priv::FXAA::internal_init_fragment_code() {
    STATIC_FXAA.m_GLSL_frag_code =
        "uniform float FXAA_REDUCE_MIN;\n"
        "uniform float FXAA_REDUCE_MUL;\n"
        "uniform float FXAA_SPAN_MAX;\n"
        "\n"
        "uniform SAMPLER_TYPE_2D inTexture;\n"
        //"uniform SAMPLER_TYPE_2D edgeTexture;\n"
        "uniform SAMPLER_TYPE_2D depthTexture;\n"
        "\n"
        "varying vec2 texcoords;\n"
        "\n"
        "void main() {\n"
        "   vec2 uvs = texcoords * (ScreenInfo.zw / ScreenInfo.xy);\n"
        "   vec2 invRes = vec2(0.0004) / ScreenInfo.zw;\n"
        "   float depth = texture2D(USE_SAMPLER_2D(depthTexture), uvs).r;\n"
        //"   float edge = texture2D(USE_SAMPLER_2D(edgeTexture), uvs).r;\n"
        "   if(depth >= 0.999){\n"
        "       gl_FragColor = texture2D(USE_SAMPLER_2D(inTexture), uvs);\n"
        "       return;\n"
        "   }\n"
        //  local constrast check
        "   vec3 rgbNW = texture2D(USE_SAMPLER_2D(inTexture), uvs + (vec2( -1.0, -1.0)) * invRes).xyz;\n"
        "   vec3 rgbNE = texture2D(USE_SAMPLER_2D(inTexture), uvs + (vec2(  1.0, -1.0)) * invRes).xyz;\n"
        "   vec3 rgbSW = texture2D(USE_SAMPLER_2D(inTexture), uvs + (vec2( -1.0,  1.0)) * invRes).xyz;\n"
        "   vec3 rgbSE = texture2D(USE_SAMPLER_2D(inTexture), uvs + (vec2(  1.0,  1.0)) * invRes).xyz;\n"
        "   vec3 rgbM  = texture2D(USE_SAMPLER_2D(inTexture), uvs).xyz;\n"
        "   vec3 luma = vec3(0.299, 0.587, 0.114);\n"
        "   float lumaNW = dot(rgbNW, luma);\n"
        "   float lumaNE = dot(rgbNE, luma);\n"
        "   float lumaSW = dot(rgbSW, luma);\n"
        "   float lumaSE = dot(rgbSE, luma);\n"
        "   float lumaM  = dot(rgbM,  luma);\n"
        "   float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));\n"
        "   float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE))); \n"
        "   vec2 dir;\n"
        "   dir.x = -(lumaNW + lumaNE) - (lumaSW + lumaSE);\n"
        "   dir.y =  (lumaNW + lumaSW) - (lumaNE + lumaSE);\n"
        "   float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);\n"
        "   float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);\n"
        "   dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX), max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), dir * rcpDirMin)) * invRes;\n"
        "   vec3 rgbA = 0.5 * (texture2D(USE_SAMPLER_2D(inTexture), uvs   + dir * (1.0/3.0 - 0.5)).xyz + texture2D(USE_SAMPLER_2D(inTexture), uvs + dir * (2.0/3.0 - 0.5)).xyz);\n"
        "   vec3 rgbB = rgbA * 0.5 + 0.25 * (texture2D(USE_SAMPLER_2D(inTexture), uvs + dir * - 0.5).xyz + texture2D(USE_SAMPLER_2D(inTexture), uvs + dir * 0.5).xyz);\n"
        "   float lumaB = dot(rgbB,luma);\n"
        "   if ((lumaB < lumaMin) || (lumaB > lumaMax)) {\n"
        "       gl_FragColor = vec4(rgbA, 1.0);\n"
        "   } else {\n"
        "       gl_FragColor = vec4(rgbB, 1.0);\n"
        "   }\n"
        "}";
}
bool Engine::priv::FXAA::init() {
    if (!STATIC_FXAA.m_GLSL_frag_code.empty())
        return false;

    STATIC_FXAA.internal_init_fragment_code();

    auto lambda_part_a = []() {
        STATIC_FXAA.m_Vertex_shader   = Engine::Resources::addResource<Shader>(Engine::priv::EShaders::fullscreen_quad_vertex, ShaderType::Vertex);
        STATIC_FXAA.m_Fragment_shader = Engine::Resources::addResource<Shader>(STATIC_FXAA.m_GLSL_frag_code, ShaderType::Fragment);
    };
    auto lambda_part_b = []() {
        STATIC_FXAA.m_Shader_program  = Engine::Resources::addResource<ShaderProgram>("FXAA", STATIC_FXAA.m_Vertex_shader, STATIC_FXAA.m_Fragment_shader);
    };

    Engine::priv::threading::addJobWithPostCallback(lambda_part_a, lambda_part_b);

    return true;
}
void Engine::priv::FXAA::pass(GBuffer& gbuffer, const Viewport& viewport, uint32_t sceneTexture, const Engine::priv::RenderModule& renderer) {
    const auto& viewportDimensions = viewport.getViewportDimensions();
    renderer.bind(m_Shader_program.get<ShaderProgram>());

    Engine::Renderer::sendUniform1("FXAA_REDUCE_MIN", reduce_min);
    Engine::Renderer::sendUniform1("FXAA_REDUCE_MUL", reduce_mul);
    Engine::Renderer::sendUniform1("FXAA_SPAN_MAX", span_max);

    Engine::priv::OpenGLBindTextureRAII inTexture{ "inTexture", gbuffer.getTexture(sceneTexture), 0, false };
    Engine::priv::OpenGLBindTextureRAII edgeTexture{ "edgeTexture", gbuffer.getTexture(GBufferType::Misc), 1, true };
    Engine::priv::OpenGLBindTextureRAII depthTexture{ "depthTexture", gbuffer.getTexture(GBufferType::Depth), 2, false };

    Engine::Renderer::renderFullscreenQuad(viewportDimensions.z, viewportDimensions.w);
}
