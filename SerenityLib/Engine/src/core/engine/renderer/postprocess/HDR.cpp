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
    m_GLSL_frag_code =
        "\n"
        "uniform SAMPLER_TYPE_2D lightingBuffer;\n"
        "uniform SAMPLER_TYPE_2D gDiffuseMap;\n"
        "uniform SAMPLER_TYPE_2D gGodsRaysMap;\n"
        "\n"
        "varying vec2 texcoords;\n"
        "\n"
        "uniform vec4 HDRInfo;\n"// exposure | UNUSED | godRays_Factor | HDRAlgorithm
        "uniform int HasGodRays;\n"
        "\n"
        "vec3 uncharted(vec3 x,float a,float b,float c,float d,float e,float f){ return vec3(((x*(a*x+c*b)+d*e)/(x*(a*x+b)+d*f))-e/f); }\n"
        "void main(){\n"
        "    vec3 diffuse = texture2D(USE_SAMPLER_2D(gDiffuseMap), texcoords).rgb;\n"
        "    vec3 lighting = texture2D(USE_SAMPLER_2D(lightingBuffer), texcoords).rgb;\n"
        "    if(HDRInfo.w > 0.0){\n"//has hdr?
        "        if(HDRInfo.w == 1.0){\n"// Reinhard tone mapping
        "            lighting = lighting / (lighting + ConstantOneVec3);\n"
        "        }else if(HDRInfo.w == 2.0){\n"// Filmic tone mapping
        "            vec3 x = max(vec3(0), lighting - vec3(0.004));\n"
        "            lighting = (x * (vec3(6.20) * x + vec3(0.5))) / (x * (vec3(6.2) * x + vec3(1.7)) + vec3(0.06));\n"
        "        }else if(HDRInfo.w == 3.0){\n"// Exposure tone mapping
        "            lighting = ConstantOneVec3 - exp(-lighting * HDRInfo.x);\n"
        "        }else if(HDRInfo.w == 4.0){\n"// Uncharted tone mapping
        "            float A = 0.15; float B = 0.5; float C = 0.1; float D = 0.2; float E = 0.02; float F = 0.3; float W = 11.2;\n"
        "            lighting = HDRInfo.x * uncharted(lighting,A,B,C,D,E,F);\n"
        "            vec3 white = 1.0 / uncharted( vec3(W),A,B,C,D,E,F );\n"
        "            lighting *= white;\n"
        "        }\n"
        "    }\n"
        "    if(HasGodRays == 1){\n" //has god rays?
        "        vec3 rays = texture2D(USE_SAMPLER_2D(gGodsRaysMap), texcoords).rgb;\n"
        "        lighting += (rays * HDRInfo.z);\n"
        "    }\n"
        "    gl_FragData[0] = vec4(lighting, 1.0);\n"
        "    gl_FragData[1] = vec4(lighting, 1.0);\n"
        "}";
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
    Engine::Renderer::sendTextureSafe("gDiffuseMap", gbuffer.getTexture(GBufferType::Diffuse), 1);
    Engine::Renderer::sendTextureSafe("gGodsRaysMap", gbuffer.getTexture(GBufferType::GodRays), 2);

    Engine::Renderer::renderFullscreenQuad();

    Engine::Renderer::clearTexture(0, GL_TEXTURE_2D);
    Engine::Renderer::clearTexture(1, GL_TEXTURE_2D);
    Engine::Renderer::clearTexture(2, GL_TEXTURE_2D);
}