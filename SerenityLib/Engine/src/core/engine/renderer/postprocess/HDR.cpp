#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/renderer/postprocess/HDR.h>
#include <core/engine/renderer/GBuffer.h>
#include <core/engine/renderer/Renderer.h>

#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/shaders/Shader.h>
#include <core/engine/resources/Engine_BuiltInShaders.h>
#include <core/engine/threading/ThreadingModule.h>
#include <core/engine/scene/Viewport.h>
#include <core/engine/resources/Engine_Resources.h>

Engine::priv::HDR Engine::priv::HDR::STATIC_HDR;

bool Engine::priv::HDR::init_shaders() {
    if (!m_GLSL_frag_code.empty())
        return false;

#pragma region HDR
    m_GLSL_frag_code = R"(
        uniform SAMPLER_TYPE_2D lightingBuffer;
        uniform SAMPLER_TYPE_2D gGodsRaysMap;

        varying vec2 texcoords;

        uniform vec4 HDRInfo; // exposure | UNUSED | godRays_Factor | HDRAlgorithm
        uniform int HasGodRays;

        vec3 uncharted(vec3 x,float a,float b,float c,float d,float e,float f){ return vec3(((x*(a*x+c*b)+d*e)/(x*(a*x+b)+d*f))-e/f); }
        void main(){
            vec3 lighting = texture2D(USE_SAMPLER_2D(lightingBuffer), texcoords).rgb;
            if(HDRInfo.w > 0.0){ //has hdr
                if(HDRInfo.w == 1.0){// Reinhard tone mapping
                    lighting = lighting / (lighting + ConstantOneVec3);
                }else if(HDRInfo.w == 2.0){ // Filmic tone mapping
                    vec3 x = max(vec3(0), lighting - vec3(0.004));
                    lighting = (x * (vec3(6.20) * x + vec3(0.5))) / (x * (vec3(6.2) * x + vec3(1.7)) + vec3(0.06));
                }else if(HDRInfo.w == 3.0){ // Exposure tone mapping
                    lighting = ConstantOneVec3 - exp(-lighting * HDRInfo.x);
                }else if(HDRInfo.w == 4.0){ // Uncharted tone mapping
                    float A = 0.15; float B = 0.5; float C = 0.1; float D = 0.2; float E = 0.02; float F = 0.3; float W = 11.2;
                    lighting = HDRInfo.x * uncharted(lighting,A,B,C,D,E,F);
                    vec3 white = 1.0 / uncharted( vec3(W),A,B,C,D,E,F );
                    lighting *= white;
                }
            }
            if(HasGodRays == 1){ //has god rays?
                vec3 rays = texture2D(USE_SAMPLER_2D(gGodsRaysMap), texcoords).rgb;
                lighting += (rays * HDRInfo.z);
            }
            gl_FragData[0] = vec4(lighting, 1.0);
            gl_FragData[1] = vec4(lighting, 1.0);
        }
    )";
#pragma endregion

    auto lambda_part_a = [&]() {
        m_Vertex_Shader   = Engine::Resources::addResource<Shader>(Engine::priv::EShaders::fullscreen_quad_vertex, ShaderType::Vertex, false);
        m_Fragment_Shader = Engine::Resources::addResource<Shader>(m_GLSL_frag_code, ShaderType::Fragment, false);
    };
    auto lambda_part_b = [&]() {
        m_Shader_Program  = Engine::Resources::addResource<ShaderProgram>("HDR", m_Vertex_Shader, m_Fragment_Shader);
    };
    Engine::priv::threading::addJobWithPostCallback(lambda_part_a, lambda_part_b);

    return true;
}
void Engine::priv::HDR::pass(GBuffer& gbuffer, const Viewport& viewport, uint32_t outTexture, uint32_t outTexture2, bool godRays, float godRaysFactor, const RenderModule& renderer) {
    renderer.bind(m_Shader_Program.get<ShaderProgram>());

    gbuffer.bindFramebuffers(outTexture, outTexture2, "RGBA");

    Engine::Renderer::sendUniform4Safe("HDRInfo", m_Exposure, 0.0f, godRaysFactor, (float)m_Algorithm);
    Engine::Renderer::sendUniform1Safe("HasGodRays", (int)godRays);

    Engine::Renderer::sendTextureSafe("lightingBuffer", gbuffer.getTexture(GBufferType::Lighting), 0);
    Engine::Renderer::sendTextureSafe("gGodsRaysMap", gbuffer.getTexture(GBufferType::GodRays), 1);

    Engine::Renderer::renderFullscreenQuad();

    Engine::Renderer::clearTexture(0, GL_TEXTURE_2D);
    Engine::Renderer::clearTexture(1, GL_TEXTURE_2D);
}