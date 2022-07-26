
#include <serenity/renderer/opengl/glsl/DepthOfFieldCode.h>

#include <serenity/renderer/opengl/Extensions.h>
#include <serenity/resources/shader/ShaderHelper.h>
#include <serenity/resources/material/MaterialEnums.h>

#include <boost/algorithm/string/replace.hpp>

void Engine::priv::opengl::glsl::DepthOfFieldCode::convert(std::string& code, uint32_t versionNumber, ShaderType shaderType) {
    if (shaderType == ShaderType::Fragment) {
        #pragma region DOF function
        if (ShaderHelper::lacksDefinition(code, "DOFExecute(", "vec4 DOFExecute(")) {
            ShaderHelper::insertStringRightBeforeLineContent(code, R"(
vec4 DOFExecute(sampler2D in_texture, vec2 in_uvs, vec2 in_aspect, vec2 in_blur_factor){
    vec4 col = vec4(0.0);
    col += texture2D(in_texture, in_uvs);
    col += texture2D(in_texture, in_uvs + (vec2(0.0, 0.4)*in_aspect)     * in_blur_factor);
    col += texture2D(in_texture, in_uvs + (vec2(0.0, -0.4)*in_aspect)    * in_blur_factor);
    col += texture2D(in_texture, in_uvs + (vec2(0.4, 0.0)*in_aspect)     * in_blur_factor);
    col += texture2D(in_texture, in_uvs + (vec2(-0.4, 0.0)*in_aspect)    * in_blur_factor);
    col += texture2D(in_texture, in_uvs + (vec2(0.29, 0.29)*in_aspect)   * in_blur_factor);
    col += texture2D(in_texture, in_uvs + (vec2(-0.29, 0.29)*in_aspect)  * in_blur_factor);
    col += texture2D(in_texture, in_uvs + (vec2(0.29, -0.29)*in_aspect)  * in_blur_factor);
    col += texture2D(in_texture, in_uvs + (vec2(-0.29, -0.29)*in_aspect) * in_blur_factor);
    for (int i = 0; i < 2; ++i) {
        int k = i+2;
        col += texture2D(in_texture, in_uvs + (vec2(0.15, 0.37)*in_aspect)   * in_blur_factor * DOFWeight[i]);
        col += texture2D(in_texture, in_uvs + (vec2(-0.15, -0.37)*in_aspect) * in_blur_factor * DOFWeight[i]);
        col += texture2D(in_texture, in_uvs + (vec2(-0.15, 0.37)*in_aspect)  * in_blur_factor * DOFWeight[i]);
        col += texture2D(in_texture, in_uvs + (vec2(0.15, -0.37)*in_aspect)  * in_blur_factor * DOFWeight[i]);
        col += texture2D(in_texture, in_uvs + (vec2(-0.37, 0.15)*in_aspect)  * in_blur_factor * DOFWeight[i]);
        col += texture2D(in_texture, in_uvs + (vec2(0.37, -0.15)*in_aspect)  * in_blur_factor * DOFWeight[i]);
        col += texture2D(in_texture, in_uvs + (vec2(0.37, 0.15)*in_aspect)   * in_blur_factor * DOFWeight[i]);
        col += texture2D(in_texture, in_uvs + (vec2(-0.37, -0.15)*in_aspect) * in_blur_factor * DOFWeight[i]);

        col += texture2D(in_texture, in_uvs + (vec2(0.29, 0.29)*in_aspect)   * in_blur_factor * DOFWeight[k]);
        col += texture2D(in_texture, in_uvs + (vec2(0.4, 0.0)*in_aspect)     * in_blur_factor * DOFWeight[k]);
        col += texture2D(in_texture, in_uvs + (vec2(0.29, -0.29)*in_aspect)  * in_blur_factor * DOFWeight[k]);
        col += texture2D(in_texture, in_uvs + (vec2(0.0, -0.4)*in_aspect)    * in_blur_factor * DOFWeight[k]);
        col += texture2D(in_texture, in_uvs + (vec2(-0.29, 0.29)*in_aspect)  * in_blur_factor * DOFWeight[k]);
        col += texture2D(in_texture, in_uvs + (vec2(-0.4, 0.0)*in_aspect)    * in_blur_factor * DOFWeight[k]);
        col += texture2D(in_texture, in_uvs + (vec2(-0.29, -0.29)*in_aspect) * in_blur_factor * DOFWeight[k]);
        col += texture2D(in_texture, in_uvs + (vec2(0.0, 0.4)*in_aspect)     * in_blur_factor * DOFWeight[k]);
    }
    return col;
}
)", "void main(");
        }
        #pragma endregion

        #pragma region DOF Weight Data
        if (ShaderHelper::lacksDefinition(code, "DOFWeight[", "float DOFWeight[")) {
            ShaderHelper::insertStringRightBeforeLineContent(code, "const float DOFWeight[4] = float[](1.0,0.9,0.7,0.4);\n", "vec4 DOFExecute(");
        }
        #pragma endregion
    }
}