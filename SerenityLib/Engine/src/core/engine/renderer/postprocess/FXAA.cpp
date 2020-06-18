#include <core/engine/renderer/postprocess/FXAA.h>
#include <core/engine/renderer/Renderer.h>

#include <core/engine/threading/ThreadingModule.h>

#include <core/engine/renderer/GBuffer.h>
#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/shaders/Shader.h>
#include <core/engine/scene/Viewport.h>

#include <core/engine/resources/Engine_BuiltInShaders.h>

using namespace std;

Engine::priv::FXAA Engine::priv::FXAA::fxaa;

const bool Engine::priv::FXAA::init_shaders() {
    if (!m_GLSL_frag_code.empty())
        return false;

#pragma region glsl_frag

    m_GLSL_frag_code =
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
        "void main(){\n"
        "   vec2 invRes = vec2(0.0004) / vec2(ScreenInfo.z / ScreenInfo.w);\n"
        "   float depth = texture2D(depthTexture, texcoords).r;\n"
        //"   float edge = texture2D(edgeTexture, texcoords).r;\n"
        "   if(depth >= 0.999){\n"
        "       gl_FragColor = texture2D(inTexture, texcoords);\n"
        "       return;\n"
        "   }\n"
        //  local constrast check
        "   vec3 rgbNW = texture2D(inTexture, texcoords + (vec2( -1.0, -1.0)) * invRes).xyz;\n"
        "   vec3 rgbNE = texture2D(inTexture, texcoords + (vec2(  1.0, -1.0)) * invRes).xyz;\n"
        "   vec3 rgbSW = texture2D(inTexture, texcoords + (vec2( -1.0,  1.0)) * invRes).xyz;\n"
        "   vec3 rgbSE = texture2D(inTexture, texcoords + (vec2(  1.0,  1.0)) * invRes).xyz;\n"
        "   vec3 rgbM  = texture2D(inTexture, texcoords).xyz;\n"
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
        "   float rcpDirMin = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);\n"
        "   dir = min(vec2(FXAA_SPAN_MAX,FXAA_SPAN_MAX),max(vec2(-FXAA_SPAN_MAX,-FXAA_SPAN_MAX),dir * rcpDirMin)) * invRes;\n"
        "   vec3 rgbA = 0.5 * (texture2D(inTexture, texcoords   + dir * (1.0/3.0 - 0.5)).xyz + texture2D(inTexture, texcoords + dir * (2.0/3.0 - 0.5)).xyz);\n"
        "   vec3 rgbB = rgbA * 0.5 + 0.25 * (texture2D(inTexture,texcoords + dir * - 0.5).xyz + texture2D(inTexture, texcoords + dir * 0.5).xyz);\n"
        "   float lumaB = dot(rgbB,luma);\n"
        "   if((lumaB < lumaMin) || (lumaB > lumaMax)){\n"
        "       gl_FragColor = vec4(rgbA, 1.0);\n"
        "   }else{\n"
        "       gl_FragColor = vec4(rgbB, 1.0);\n"
        "   }\n"
        "}";
#pragma endregion


    auto lambda_part_a = [&]() {
        m_Vertex_shader   = NEW Shader(Engine::priv::EShaders::fullscreen_quad_vertex, ShaderType::Vertex, false);
        m_Fragment_shader = NEW Shader(m_GLSL_frag_code, ShaderType::Fragment, false);
    };
    auto lambda_part_b = [&]() {
        m_Shader_program = NEW ShaderProgram("FXAA", *m_Vertex_shader, *m_Fragment_shader);
    };

    Engine::priv::threading::addJobWithPostCallback(lambda_part_a, lambda_part_b);

    return true;
}
Engine::priv::FXAA::~FXAA() {
    SAFE_DELETE(m_Shader_program);
    SAFE_DELETE(m_Fragment_shader);
    SAFE_DELETE(m_Vertex_shader);
}
void Engine::priv::FXAA::pass(GBuffer& gbuffer, const Viewport& viewport, const unsigned int sceneTexture, const Engine::priv::Renderer& renderer) {
    const auto& dimensions = viewport.getViewportDimensions();
    renderer.bind(m_Shader_program);

    Engine::Renderer::sendUniform1("FXAA_REDUCE_MIN", reduce_min);
    Engine::Renderer::sendUniform1("FXAA_REDUCE_MUL", reduce_mul);
    Engine::Renderer::sendUniform1("FXAA_SPAN_MAX", span_max);

    Engine::Renderer::sendTexture("inTexture", gbuffer.getTexture(sceneTexture), 0);
    Engine::Renderer::sendTextureSafe("edgeTexture", gbuffer.getTexture(GBufferType::Misc), 1);
    Engine::Renderer::sendTexture("depthTexture", gbuffer.getTexture(GBufferType::Depth), 2);

    Engine::Renderer::renderFullscreenQuad();
}
void Engine::Renderer::fxaa::setReduceMin(const float r) {
    Engine::priv::FXAA::fxaa.reduce_min = glm::max(0.0f, r);
}
void Engine::Renderer::fxaa::setReduceMul(const float r) {
    Engine::priv::FXAA::fxaa.reduce_mul = glm::max(0.0f, r);
}
void Engine::Renderer::fxaa::setSpanMax(const float r) {
    Engine::priv::FXAA::fxaa.span_max = glm::max(0.0f, r);
}
const float Engine::Renderer::fxaa::getReduceMin() {
    return Engine::priv::FXAA::fxaa.reduce_min;
}
const float Engine::Renderer::fxaa::getReduceMul() {
    return Engine::priv::FXAA::fxaa.reduce_mul;
}
const float Engine::Renderer::fxaa::getSpanMax() {
    return Engine::priv::FXAA::fxaa.span_max;
}