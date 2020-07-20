#include "core/engine/utils/PrecompiledHeader.h"
#include <core/engine/renderer/opengl/glsl/SSAOCode.h>

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
using namespace std;


void opengl::glsl::SSAOCode::convert(string& code, const unsigned int versionNumber, const ShaderType::Type shaderType) {

    if (shaderType == ShaderType::Fragment) {

#pragma region SSAO function
        if (ShaderHelper::sfind(code, "SSAOExecute(")) {
            if (!ShaderHelper::sfind(code, "float SSAOExecute(")) {
                const string execute_code =
                    "float SSAOExecute(sampler2D inDepthSampler, vec2 in_uvs, int in_numSamples, int in_noiseTextureSize, vec2 in_randomVector, float in_radius, vec3 in_position, vec3 in_normals, float in_intensity, float in_bias, float in_scale){//generated\n"
                    "    float res = 0.0;\n"
                    "    for (int i = 0; i < in_numSamples; ++i) {\n"
                    "       vec2 coord1 = reflect(SSAOPoisson[i].xy * vec2(in_noiseTextureSize), in_randomVector) * in_radius;\n"
                    "       vec2 coord2 = vec2(coord1.x * 0.707 - coord1.y * 0.707, coord1.x * 0.707 + coord1.y * 0.707);\n"
                    "       res += SSAOOcclude(inDepthSampler, in_uvs + (coord1 * 0.25), in_position, in_normals, in_intensity, in_bias, in_scale);\n"
                    "       res += SSAOOcclude(inDepthSampler, in_uvs + (coord2 * 0.50), in_position, in_normals, in_intensity, in_bias, in_scale);\n"
                    "       res += SSAOOcclude(inDepthSampler, in_uvs + (coord1 * 0.75), in_position, in_normals, in_intensity, in_bias, in_scale);\n"
                    "       res += SSAOOcclude(inDepthSampler, in_uvs + coord2,          in_position, in_normals, in_intensity, in_bias, in_scale);\n"
                    "    }\n"
                    "    res /= (in_numSamples * 4.0);\n"
                    "    return res;\n"
                    "}\n";
                ShaderHelper::insertStringRightBeforeLineContent(code, execute_code, "void main(");
            }
        }
#pragma endregion

#pragma region Occlude function
        if (ShaderHelper::sfind(code, "SSAOOcclude(")) {
            if (!ShaderHelper::sfind(code, "float SSAOOcclude(")) {
                const string occlude_code =
                    "float SSAOOcclude(sampler2D inDepthSampler, vec2 in_offsetUV, vec3 in_origin, vec3 in_normal, in float in_intensity, in float in_bias, in float in_scale){//generated\n"
                    "    vec3 ViewPos          = GetViewPosition(inDepthSampler, in_offsetUV, CameraNear, CameraFar);\n"
                    "    vec3 PositionOffset   = ViewPos - in_origin;\n"
                    "    float Length          = length(PositionOffset);\n"
                    "    vec3 VectorNormalized = PositionOffset / Length;\n"
                    "    float Dist            = Length * in_scale;\n"
                    "    float attenuation     = 1.0 / (1.0 + Dist);\n"
                    "    float angleMath       = max(0.0, dot(in_normal, VectorNormalized) - in_bias);\n"
                    "    return angleMath * attenuation * in_intensity;\n"
                    "}\n";
                ShaderHelper::insertStringRightBeforeLineContent(code, occlude_code, "float SSAOExecute(");
            }
        }
#pragma endregion


#pragma region Poisson Data
        if (ShaderHelper::sfind(code, "SSAOPoisson[")) {
            if (!ShaderHelper::sfind(code, "vec2 SSAOPoisson[")) {
                const string poisson_code =
                    "const vec2 SSAOPoisson[16] = vec2[](vec2(1.0, 0.0), vec2(-1.0, 0.0),vec2(0.0, 1.0),vec2(0.0, -1.0),\n"
                    "                               vec2(-0.707, 0.707), vec2(0.707, -0.707),vec2(-0.707, -0.707),vec2(0.707, 0.707),\n"
                    "                               vec2(-0.375, 0.927), vec2(0.375, -0.927), vec2(-0.375, -0.927), vec2(0.375, 0.927),\n"
                    "                               vec2(-0.927, 0.375), vec2(0.927, -0.375), vec2(-0.927, -0.375), vec2(0.927, 0.375));\n"
                    "\n";
                ShaderHelper::insertStringRightBeforeLineContent(code, poisson_code, "float SSAOOcclude(");
            }
        }
#pragma endregion

    }

}