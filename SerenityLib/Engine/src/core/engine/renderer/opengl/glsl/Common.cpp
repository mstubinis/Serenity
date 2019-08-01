#include <core/engine/renderer/opengl/glsl/Common.h>
#include <core/engine/shaders/ShaderHelper.h>

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/lexical_cast.hpp>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;


void epriv::opengl::glsl::Common::convert(string& code) {

#pragma region normal map
    if (ShaderHelper::sfind(code, "CalcBumpedNormal(") || ShaderHelper::sfind(code, "CalcBumpedNormalCompressed(")) {
        if (!ShaderHelper::sfind(code, "vec3 CalcBumpedNormal(")) {
            if (ShaderHelper::sfind(code, "varying mat3 TBN;")) {
                boost::replace_all(code, "varying mat3 TBN;", "");
            }
            string normal_map =
                "varying mat3 TBN;\n"
                "vec3 CalcBumpedNormal(vec2 _uv,sampler2D _inTexture){//generated\n"
                "    vec3 _t = (texture2D(_inTexture, _uv).xyz) * 2.0 - 1.0;\n"
                "    return normalize(TBN * _t);\n"
                "}\n"
                "vec3 CalcBumpedNormalCompressed(vec2 _uv,sampler2D _inTexture){//generated\n"
                "    vec2 _t = (texture2D(_inTexture, _uv).yx) * 2.0 - 1.0;\n" //notice the yx flip, its needed
                "    float _z = sqrt(1.0 - _t.x * _t.x - _t.y * _t.y);\n"
                "    vec3 normal = vec3(_t.xy, _z);\n"//recalc z in the shader
                "    return normalize(TBN * normal);\n"
                "}\n";
            ShaderHelper::insertStringAtLine(code, normal_map, 1);
        }
    }
#pragma endregion

#pragma region pack nibble
    if (ShaderHelper::sfind(code, "Pack2NibblesInto8BitChannel(")) {
        if (!ShaderHelper::sfind(code, "float Pack2NibblesInto8BitChannel(")) {
            string pack_nibble =
                "float Pack2NibblesInto8BitChannel(float x,float y){\n"
                "    float xF = round(x / 0.0666);\n"
                "    float yF = round(y / 0.0666) * 16.0;\n"
                "    return (xF + yF) / 255.0;\n"
                "}\n";
            ShaderHelper::insertStringAtLine(code, pack_nibble, 1);
        }
    }
#pragma endregion

#pragma region unpack nibble
    if (ShaderHelper::sfind(code, "Unpack2NibblesFrom8BitChannel(")) {
        if (!ShaderHelper::sfind(code, "vec2 Unpack2NibblesFrom8BitChannel(")) {
            string unpack_nibble =
                "vec2 Unpack2NibblesFrom8BitChannel(float data){\n"
                "    float d = data * 255.0;\n"
                "    float y = fract(d / 16.0);\n"
                "    float x = (d - (y * 16.0));\n"
                "    return vec2(y, x / 255.0);\n"
                "}\n";
            ShaderHelper::insertStringAtLine(code, unpack_nibble, 1);
        }
    }
#pragma endregion

#pragma region painters algorithm
    if (ShaderHelper::sfind(code, "PaintersAlgorithm(")) {
        if (!ShaderHelper::sfind(code, "vec4 PaintersAlgorithm(")) {
            string painters = "\n"
                "vec4 PaintersAlgorithm(vec4 paint, vec4 canvas){//generated\n"
                "    float alpha = paint.a + canvas.a * (1.0 - paint.a);\n"
                "    vec4 ret = vec4(0.0);\n"
                "    ret = ((paint * paint.a + canvas * canvas.a * (1.0 - paint.a)) / alpha);\n"
                "    ret.a = alpha;\n"
                "    return ret;\n"
                "}\n";
            ShaderHelper::insertStringAtLine(code, painters, 1);
        }
    }
#pragma endregion

#pragma region invert color
    if (ShaderHelper::sfind(code, "InvertColor(")) {
        if (!ShaderHelper::sfind(code, "vec4 InvertColor(")) {
            string invCol = "\n"
                "vec4 InvertColor(vec4 color){//generated\n"
                "    return vec4(vec4(1.0) - color);\n"
                "}\n";
            ShaderHelper::insertStringAtLine(code, invCol, 1);
        }
    }
#pragma endregion

#pragma region invert color 255
    if (ShaderHelper::sfind(code, "InvertColor255(")) {
        if (!ShaderHelper::sfind(code, "vec4 InvertColor255(")) {
            string invCol255 = "\n"
                "vec4 InvertColor255(vec4 color){//generated\n"
                "    return vec4(vec4(255.0) - color);\n"
                "}\n";
            ShaderHelper::insertStringAtLine(code, invCol255, 1);
        }
    }
#pragma endregion

#pragma region range to 255
    if (ShaderHelper::sfind(code, "RangeTo255(")) {
        if (!ShaderHelper::sfind(code, "vec4 RangeTo255(")) {
            string range255 = "\n"
                "vec4 RangeTo255(vec4 color){//generated\n"
                "    return color * 255.0;\n"
                "}\n";
            ShaderHelper::insertStringAtLine(code, range255, 1);
        }
    }
#pragma endregion

#pragma region range to 1
    if (ShaderHelper::sfind(code, "RangeTo1(")) {
        if (!ShaderHelper::sfind(code, "vec4 RangeTo1(")) {
            string range1 = "\n"
                "vec4 RangeTo1(vec4 color){//generated\n"
                "    return color / 255.0;\n"
                "}\n";
            ShaderHelper::insertStringAtLine(code, range1, 1);
        }
    }
#pragma endregion
}