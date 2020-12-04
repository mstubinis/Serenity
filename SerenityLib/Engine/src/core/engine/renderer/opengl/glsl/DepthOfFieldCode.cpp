#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/renderer/opengl/glsl/DepthOfFieldCode.h>

#include <core/engine/renderer/opengl/Extensions.h>
#include <core/engine/shaders/ShaderHelper.h>
#include <core/engine/materials/MaterialEnums.h>

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/lexical_cast.hpp>

using namespace Engine;
using namespace Engine::priv;

void opengl::glsl::DepthOfFieldCode::convert(std::string& code, unsigned int versionNumber, ShaderType shaderType) {
    if (shaderType == ShaderType::Fragment) {
        #pragma region DOF function
        if (ShaderHelper::sfind(code, "DOFExecute(")) {
            if (!ShaderHelper::sfind(code, "vec4 DOFExecute(")) {
                ShaderHelper::insertStringRightBeforeLineContent(code, R"(
                    vec4 DOFExecute(sampler2D in_texture, vec2 in_uvs, vec2 in_aspect, vec2 in_blur_factor){ //generated
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
        }
        #pragma endregion

        #pragma region DOF Weight Data
        if (ShaderHelper::sfind(code, "DOFWeight[")) {
            if (!ShaderHelper::sfind(code, "float DOFWeight[")) {
                ShaderHelper::insertStringRightBeforeLineContent(code, "const float DOFWeight[4] = float[](1.0,0.9,0.7,0.4);\n", "vec4 DOFExecute(");
            }
        }
        #pragma endregion
    }
}