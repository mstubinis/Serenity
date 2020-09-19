#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/renderer/postprocess/DepthOfField.h>
#include <core/engine/renderer/Renderer.h>
#include <core/engine/renderer/GBuffer.h>
#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/shaders/Shader.h>
#include <core/engine/resources/Engine_BuiltInShaders.h>
#include <core/engine/threading/ThreadingModule.h>
#include <core/engine/scene/Viewport.h>

Engine::priv::DepthOfField Engine::priv::DepthOfField::STATIC_DOF;

bool Engine::priv::DepthOfField::init_shaders() {
    if (!m_GLSL_frag_code.empty())
        return false;

#pragma region DepthOfField
    m_GLSL_frag_code =
        "\n"
        "const float DOFWeight[4] = float[](1.0,0.9,0.7,0.4);\n"
        "\n"
        "uniform SAMPLER_TYPE_2D inTexture;\n"
        "uniform SAMPLER_TYPE_2D depthTexture;\n"
        "\n"
        "uniform vec4 Data;\n" //x = blurClamp, y = bias, z = focus, w = UNUSED
        "\n"
        "varying vec2 texcoords;\n"
        "void main(){\n"
        "    vec2 aspectcorrect = vec2(1.0, ScreenInfo.z / ScreenInfo.w);\n"
        "    float depth = texture2D(depthTexture, texcoords).r;\n"
        "    float factor = (depth - Data.z);\n"
        "    vec2 dofblur = vec2(clamp(factor * Data.y, -Data.x, Data.x));\n"
        //   vec4 col = DOFExecute(inTexture, texcoords, aspectcorrect, dofblur);
        //TODO: use the above commented function only and test if it works.
        "    vec4 col = vec4(0.0);\n"
        "    col += texture2D(inTexture, texcoords);\n"
        "    col += texture2D(inTexture, texcoords + (vec2(0.0, 0.4) * aspectcorrect)     * dofblur);\n"
        "    col += texture2D(inTexture, texcoords + (vec2(0.0, -0.4) * aspectcorrect)    * dofblur);\n"
        "    col += texture2D(inTexture, texcoords + (vec2(0.4, 0.0) * aspectcorrect)     * dofblur);\n"
        "    col += texture2D(inTexture, texcoords + (vec2(-0.4, 0.0) * aspectcorrect)    * dofblur);\n"
        "    col += texture2D(inTexture, texcoords + (vec2(0.29, 0.29) * aspectcorrect)   * dofblur);\n"
        "    col += texture2D(inTexture, texcoords + (vec2(-0.29, 0.29) * aspectcorrect)  * dofblur);\n"
        "    col += texture2D(inTexture, texcoords + (vec2(0.29, -0.29) * aspectcorrect)  * dofblur);\n"
        "    col += texture2D(inTexture, texcoords + (vec2(-0.29, -0.29) * aspectcorrect) * dofblur);\n"
        "    for (int i = 0; i < 2; ++i) {\n"
        "        int k = i+2;\n"
        "        col += texture2D(inTexture, texcoords + (vec2(0.15, 0.37) * aspectcorrect)   * dofblur * DOFWeight[i]);\n"
        "        col += texture2D(inTexture, texcoords + (vec2(-0.15, -0.37) * aspectcorrect) * dofblur * DOFWeight[i]);\n"
        "        col += texture2D(inTexture, texcoords + (vec2(-0.15, 0.37) * aspectcorrect)  * dofblur * DOFWeight[i]);\n"
        "        col += texture2D(inTexture, texcoords + (vec2(0.15, -0.37) * aspectcorrect)  * dofblur * DOFWeight[i]);\n"
        "        col += texture2D(inTexture, texcoords + (vec2(-0.37, 0.15) * aspectcorrect)  * dofblur * DOFWeight[i]);\n"
        "        col += texture2D(inTexture, texcoords + (vec2(0.37, -0.15) * aspectcorrect)  * dofblur * DOFWeight[i]);\n"
        "        col += texture2D(inTexture, texcoords + (vec2(0.37, 0.15) * aspectcorrect)   * dofblur * DOFWeight[i]);\n"
        "        col += texture2D(inTexture, texcoords + (vec2(-0.37, -0.15) * aspectcorrect) * dofblur * DOFWeight[i]);\n"
        "\n"
        "        col += texture2D(inTexture, texcoords + (vec2(0.29, 0.29) * aspectcorrect)   * dofblur * DOFWeight[k]);\n"
        "        col += texture2D(inTexture, texcoords + (vec2(0.4, 0.0) * aspectcorrect)     * dofblur * DOFWeight[k]);\n"
        "        col += texture2D(inTexture, texcoords + (vec2(0.29, -0.29) * aspectcorrect)  * dofblur * DOFWeight[k]);\n"
        "        col += texture2D(inTexture, texcoords + (vec2(0.0, -0.4) * aspectcorrect)    * dofblur * DOFWeight[k]);\n"
        "        col += texture2D(inTexture, texcoords + (vec2(-0.29, 0.29) * aspectcorrect)  * dofblur * DOFWeight[k]);\n"
        "        col += texture2D(inTexture, texcoords + (vec2(-0.4, 0.0) * aspectcorrect)    * dofblur * DOFWeight[k]);\n"
        "        col += texture2D(inTexture, texcoords + (vec2(-0.29, -0.29) * aspectcorrect) * dofblur * DOFWeight[k]);\n"
        "        col += texture2D(inTexture, texcoords + (vec2(0.0, 0.4) * aspectcorrect)     * dofblur * DOFWeight[k]);\n"
        "    }\n"
        "    gl_FragColor.rgb = col.rgb * 0.02439; \n" //0.02439 = 1.0 / 41.0

        //"    gl_FragColor.a = 1.0; \n"
        "}\n"
        "\n";
#pragma endregion

    Engine::priv::threading::addJobWithPostCallback([this]() {
        m_Vertex_Shader   = std::make_unique<Shader>(Engine::priv::EShaders::fullscreen_quad_vertex, ShaderType::Vertex, false);
        m_Fragment_Shader = std::make_unique<Shader>(m_GLSL_frag_code, ShaderType::Fragment, false);
    }, [this]() {
        m_Shader_Program  = std::make_unique<ShaderProgram>("DepthOfField", *m_Vertex_Shader, *m_Fragment_Shader);
    });

    return true;
}
void Engine::priv::DepthOfField::pass(GBuffer& gbuffer, const Viewport& viewport, unsigned int sceneTexture, const Engine::priv::Renderer& renderer) {
    renderer.bind(m_Shader_Program.get());

    Engine::Renderer::sendUniform4Safe("Data", blur_radius, bias, focus, 0.0f);

    Engine::Renderer::sendTextureSafe("inTexture", gbuffer.getTexture(sceneTexture), 0);
    Engine::Renderer::sendTextureSafe("textureDepth", gbuffer.getTexture(GBufferType::Depth), 1);

    Engine::Renderer::renderFullscreenQuad();
}

void Engine::Renderer::depthOfField::enable(bool b) {
    Engine::priv::DepthOfField::STATIC_DOF.dof = b;
}
void Engine::Renderer::depthOfField::disable() {
    Engine::priv::DepthOfField::STATIC_DOF.dof = false;
}
bool Engine::Renderer::depthOfField::enabled() {
    return Engine::priv::DepthOfField::STATIC_DOF.dof;
}
float Engine::Renderer::depthOfField::getFocus() {
    return Engine::priv::DepthOfField::STATIC_DOF.focus;
}
void Engine::Renderer::depthOfField::setFocus(float f) {
    Engine::priv::DepthOfField::STATIC_DOF.focus = glm::max(0.0f, f);
}
float Engine::Renderer::depthOfField::getBias() {
    return Engine::priv::DepthOfField::STATIC_DOF.bias;
}
void Engine::Renderer::depthOfField::setBias(float b) {
    Engine::priv::DepthOfField::STATIC_DOF.bias = b;
}
float Engine::Renderer::depthOfField::getBlurRadius() {
    return Engine::priv::DepthOfField::STATIC_DOF.blur_radius;
}
void Engine::Renderer::depthOfField::setBlurRadius(float r) {
    Engine::priv::DepthOfField::STATIC_DOF.blur_radius = glm::max(0.0f, r);
}