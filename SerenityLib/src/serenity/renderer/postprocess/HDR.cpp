
#include <serenity/renderer/postprocess/HDR.h>
#include <serenity/renderer/GBuffer.h>
#include <serenity/renderer/Renderer.h>

#include <serenity/resources/shader/ShaderProgram.h>
#include <serenity/resources/shader/Shader.h>
#include <serenity/resources/Engine_BuiltInShaders.h>
#include <serenity/threading/ThreadingModule.h>
#include <serenity/scene/Viewport.h>
#include <serenity/resources/Engine_Resources.h>
#include <serenity/renderer/opengl/BindTextureRAII.h>

Engine::priv::HDR Engine::priv::HDR::STATIC_HDR;

void Engine::priv::HDR::internal_init_fragment_code() {
//  uniform vec4 HDRInfo; // exposure | UNUSED | godRays_Factor | HDRAlgorithm
//  HDRInfo.w == 1.0 // Reinhard tone mapping
//  HDRInfo.w == 2.0 // Filmic tone mapping
//  HDRInfo.w == 3.0 // Exposure tone mapping
//  HDRInfo.w == 4.0 // Uncharted tone mapping
    STATIC_HDR.m_GLSL_frag_code = R"(
uniform SAMPLER_TYPE_2D gLightingMap;
uniform SAMPLER_TYPE_2D gGodsRaysMap;

varying vec2 texcoords;

uniform vec4 HDRInfo;
uniform int HasGodRays;

vec3 uncharted(vec3 x, float a, float b, float c, float d, float e, float f){ 
    return vec3(((x * (a * x + c * b) + d * e) / (x * (a * x + b) + d * f)) - e / f); 
}
void main() {
    vec2 uvs = ViewportUVCalculation(texcoords);
    vec3 lighting = texture2D(USE_SAMPLER_2D(gLightingMap), uvs).rgb;
    if(HDRInfo.w == 1.0){
        lighting  = lighting / (lighting + ConstantOneVec3);
        lighting *= HDRInfo.x;
    }else if(HDRInfo.w == 2.0){
        vec3 x    = max(vec3(0), lighting - vec3(0.004));
        lighting  = (x * (vec3(6.20) * x + vec3(0.5))) / (x * (vec3(6.2) * x + vec3(1.7)) + vec3(0.06));
        lighting *= (HDRInfo.x * 0.2);
    }else if(HDRInfo.w == 3.0){
        lighting  = ConstantOneVec3 - exp(-lighting * (HDRInfo.x * 0.5));
    }else if(HDRInfo.w == 4.0){
        const float A = 0.15; 
        const float B = 0.5; 
        const float C = 0.1; 
        const float D = 0.2; 
        const float E = 0.02; 
        const float F = 0.3; 
        const float W = 11.2;
        lighting      = HDRInfo.x * uncharted(lighting, A, B, C, D, E, F);
        vec3 white    = 1.0 / uncharted( vec3(W), A, B, C, D, E, F );
        lighting     *= white;
    }
    if(HasGodRays == 1){
        vec3 rays = texture2D(USE_SAMPLER_2D(gGodsRaysMap), uvs).rgb;
        lighting += rays * HDRInfo.z;
    }
    gl_FragData[0] = vec4(lighting, 1.0);
    gl_FragData[1] = vec4(lighting, 1.0);
}
    )";
}
bool Engine::priv::HDR::init() {
    if (!STATIC_HDR.m_GLSL_frag_code.empty())
        return false;

    STATIC_HDR.internal_init_fragment_code();

    auto lambda_part_a = []() {
        STATIC_HDR.m_Vertex_Shader   = Engine::Resources::addResource<Shader>(Engine::priv::EShaders::fullscreen_quad_vertex, ShaderType::Vertex);
        STATIC_HDR.m_Fragment_Shader = Engine::Resources::addResource<Shader>(STATIC_HDR.m_GLSL_frag_code, ShaderType::Fragment);
    };
    auto lambda_part_b = []() {
        STATIC_HDR.m_Shader_Program  = Engine::Resources::addResource<ShaderProgram>("HDR", STATIC_HDR.m_Vertex_Shader, STATIC_HDR.m_Fragment_Shader);
    };
    Engine::priv::threading::addJobWithPostCallback(lambda_part_a, lambda_part_b);

    return true;
}
void Engine::priv::HDR::pass(GBuffer& gbuffer, const Viewport& viewport, uint32_t outTexture, uint32_t outTexture2, bool godRays, float godRaysFactor, const RenderModule& renderer) {
    const auto& viewportDimensions = viewport.getViewportDimensions();
    renderer.bind(m_Shader_Program.get<ShaderProgram>());

    gbuffer.bindFramebuffers({ outTexture, outTexture2 }, "RGBA");

    Engine::Renderer::sendUniform4Safe("HDRInfo", m_Exposure, 0.0f, godRaysFactor, float(m_Algorithm));
    Engine::Renderer::sendUniform1Safe("HasGodRays", int(godRays));

    Engine::priv::OpenGLBindTextureRAII gLightingMap{ "gLightingMap", gbuffer.getTexture(GBufferType::Lighting), 0, false };
    Engine::priv::OpenGLBindTextureRAII gGodsRaysMap{ "gGodsRaysMap", gbuffer.getTexture(GBufferType::GodRays), 1, false };

    Engine::Renderer::renderFullscreenQuadCentered(viewportDimensions.z, viewportDimensions.w);

}