
#include <serenity/renderer/postprocess/DepthOfField.h>
#include <serenity/renderer/Renderer.h>
#include <serenity/renderer/GBuffer.h>
#include <serenity/resources/shader/ShaderProgram.h>
#include <serenity/resources/shader/Shader.h>
#include <serenity/resources/Engine_BuiltInShaders.h>
#include <serenity/threading/ThreadingModule.h>
#include <serenity/scene/Viewport.h>
#include <serenity/resources/Engine_Resources.h>

Engine::priv::DepthOfField Engine::priv::DepthOfField::STATIC_DOF;

void Engine::priv::DepthOfField::internal_init_fragment_code() {
    STATIC_DOF.m_GLSL_frag_code = R"(
const float DOFWeight[4] = float[](1.0,0.9,0.7,0.4);

uniform SAMPLER_TYPE_2D inTexture;
uniform SAMPLER_TYPE_2D depthTexture;

uniform vec4 Data; //x = blurClamp, y = bias, z = focus, w = UNUSED

varying vec2 texcoords;
void main(){
    vec2 aspectcorrect = vec2(1.0, ScreenInfo.z / ScreenInfo.w);
    float depth = texture2D(depthTexture, texcoords).r;
    float factor = (depth - Data.z);
    vec2 dofblur = vec2(clamp(factor * Data.y, -Data.x, Data.x));
//   vec4 col = DOFExecute(inTexture, texcoords, aspectcorrect, dofblur);
//TODO: use the above commented function only and test if it works.
    vec4 col = vec4(0.0);
    col += texture2D(inTexture, texcoords);
    col += texture2D(inTexture, texcoords + (vec2(0.0, 0.4) * aspectcorrect)     * dofblur);
    col += texture2D(inTexture, texcoords + (vec2(0.0, -0.4) * aspectcorrect)    * dofblur);
    col += texture2D(inTexture, texcoords + (vec2(0.4, 0.0) * aspectcorrect)     * dofblur);
    col += texture2D(inTexture, texcoords + (vec2(-0.4, 0.0) * aspectcorrect)    * dofblur);
    col += texture2D(inTexture, texcoords + (vec2(0.29, 0.29) * aspectcorrect)   * dofblur);
    col += texture2D(inTexture, texcoords + (vec2(-0.29, 0.29) * aspectcorrect)  * dofblur);
    col += texture2D(inTexture, texcoords + (vec2(0.29, -0.29) * aspectcorrect)  * dofblur);
    col += texture2D(inTexture, texcoords + (vec2(-0.29, -0.29) * aspectcorrect) * dofblur);
    for (int i = 0; i < 2; ++i) {
        int k = i+2;
        col += texture2D(inTexture, texcoords + (vec2(0.15, 0.37) * aspectcorrect)   * dofblur * DOFWeight[i]);
        col += texture2D(inTexture, texcoords + (vec2(-0.15, -0.37) * aspectcorrect) * dofblur * DOFWeight[i]);
        col += texture2D(inTexture, texcoords + (vec2(-0.15, 0.37) * aspectcorrect)  * dofblur * DOFWeight[i]);
        col += texture2D(inTexture, texcoords + (vec2(0.15, -0.37) * aspectcorrect)  * dofblur * DOFWeight[i]);
        col += texture2D(inTexture, texcoords + (vec2(-0.37, 0.15) * aspectcorrect)  * dofblur * DOFWeight[i]);
        col += texture2D(inTexture, texcoords + (vec2(0.37, -0.15) * aspectcorrect)  * dofblur * DOFWeight[i]);
        col += texture2D(inTexture, texcoords + (vec2(0.37, 0.15) * aspectcorrect)   * dofblur * DOFWeight[i]);
        col += texture2D(inTexture, texcoords + (vec2(-0.37, -0.15) * aspectcorrect) * dofblur * DOFWeight[i]);

        col += texture2D(inTexture, texcoords + (vec2(0.29, 0.29) * aspectcorrect)   * dofblur * DOFWeight[k]);
        col += texture2D(inTexture, texcoords + (vec2(0.4, 0.0) * aspectcorrect)     * dofblur * DOFWeight[k]);
        col += texture2D(inTexture, texcoords + (vec2(0.29, -0.29) * aspectcorrect)  * dofblur * DOFWeight[k]);
        col += texture2D(inTexture, texcoords + (vec2(0.0, -0.4) * aspectcorrect)    * dofblur * DOFWeight[k]);
        col += texture2D(inTexture, texcoords + (vec2(-0.29, 0.29) * aspectcorrect)  * dofblur * DOFWeight[k]);
        col += texture2D(inTexture, texcoords + (vec2(-0.4, 0.0) * aspectcorrect)    * dofblur * DOFWeight[k]);
        col += texture2D(inTexture, texcoords + (vec2(-0.29, -0.29) * aspectcorrect) * dofblur * DOFWeight[k]);
        col += texture2D(inTexture, texcoords + (vec2(0.0, 0.4) * aspectcorrect)     * dofblur * DOFWeight[k]);
    }
    gl_FragColor.rgb = col.rgb * 0.02439; //0.02439 = 1.0 / 41.0
}
    )";
}

bool Engine::priv::DepthOfField::init() {
    if (!STATIC_DOF.m_GLSL_frag_code.empty())
        return false;

    STATIC_DOF.internal_init_fragment_code();

    Engine::priv::threading::addJobWithPostCallback([]() {
        STATIC_DOF.m_Vertex_Shader   = Engine::Resources::addResource<Shader>(Engine::priv::EShaders::fullscreen_quad_vertex, ShaderType::Vertex, false);
        STATIC_DOF.m_Fragment_Shader = Engine::Resources::addResource<Shader>(STATIC_DOF.m_GLSL_frag_code, ShaderType::Fragment, false);
    }, 
    []() {
        STATIC_DOF.m_Shader_Program  = Engine::Resources::addResource<ShaderProgram>("DepthOfField", STATIC_DOF.m_Vertex_Shader, STATIC_DOF.m_Fragment_Shader);
    });

    return true;
}
void Engine::priv::DepthOfField::pass(GBuffer& gbuffer, const Viewport& viewport, uint32_t sceneTexture, const Engine::priv::RenderModule& renderer) {
    renderer.bind(m_Shader_Program.get<ShaderProgram>());

    Engine::Renderer::sendUniform4Safe("Data", blur_radius, bias, focus, 0.0f);

    Engine::Renderer::sendTextureSafe("inTexture", gbuffer.getTexture(sceneTexture), 0);
    Engine::Renderer::sendTextureSafe("textureDepth", gbuffer.getTexture(GBufferType::Depth), 1);

    Engine::Renderer::renderFullscreenQuad();

    Engine::Renderer::clearTexture(0, GL_TEXTURE_2D);
    Engine::Renderer::clearTexture(1, GL_TEXTURE_2D);
}
