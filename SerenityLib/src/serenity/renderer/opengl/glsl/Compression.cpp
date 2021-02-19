
#include <serenity/renderer/opengl/glsl/Compression.h>
#include <serenity/resources/shader/ShaderHelper.h>
#include <serenity/resources/material/MaterialEnums.h>

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/lexical_cast.hpp>

using namespace Engine;
using namespace Engine::priv;

void opengl::glsl::Compression::convert(std::string& code, uint32_t versionNumber) {
#pragma region Pack2NibblesInto8BitChannel
    if (ShaderHelper::lacksDefinition(code, "Pack2NibblesInto8BitChannel(", "float Pack2NibblesInto8BitChannel(")) {
        ShaderHelper::insertStringAtLine(code, R"(
float Pack2NibblesInto8BitChannel(float x, float y){
    float xF = round(x / 0.0666666666666666);
    float yF = round(y / 0.0666666666666666) * 16.0;
    return (xF + yF) / 255.0;
}
)", 1);
    }
#pragma endregion

#pragma region Unpack2NibblesFrom8BitChannel
    if (ShaderHelper::lacksDefinition(code, "Unpack2NibblesFrom8BitChannel(", "vec2 Unpack2NibblesFrom8BitChannel(")) {
        ShaderHelper::insertStringAtLine(code, R"(
vec2 Unpack2NibblesFrom8BitChannel(float data){
    float d = data * 255.0;
    float y = fract(d / 16.0);
    float x = (d - (y * 16.0));
    return vec2(y, x / 255.0);
}
)", 1);
    }
#pragma endregion

#pragma region Unpack3FloatsInto1FloatUnsigned
    if (ShaderHelper::lacksDefinition(code, "Unpack3FloatsInto1FloatUnsigned", "vec3 Unpack3FloatsInto1FloatUnsigned")) {
        ShaderHelper::insertStringAtLine(code, R"(
vec3 Unpack3FloatsInto1FloatUnsigned(float v){
    vec3 ret;
    ret.r = mod(v,           1.0);
    ret.g = mod(v * 256.0,   1.0);
    ret.b = mod(v * 65536.0, 1.0);
    return ret;
}
)", 1);
    }
#pragma endregion

#pragma region Unpack3FloatsInto1Float
    if (ShaderHelper::lacksDefinition(code, "Unpack3FloatsInto1Float", "vec3 Unpack3FloatsInto1Float")) {
        ShaderHelper::insertStringAtLine(code, R"(
vec3 Unpack3FloatsInto1Float(float v){
    vec3 ret;
    ret.r = mod(v,           1.0);
    ret.g = mod(v * 256.0,   1.0);
    ret.b = mod(v * 65536.0, 1.0);
    ret = ret * 2.0 - 1.0;
    return ret;
}
)", 1);
    }
#pragma endregion

#pragma region Pack2FloatIntoFloat16
    if (ShaderHelper::lacksDefinition(code, "Pack2FloatIntoFloat16", "float Pack2FloatIntoFloat16")) {
        ShaderHelper::insertStringAtLine(code, R"(
float Pack2FloatIntoFloat16(float x, float y){
    x = clamp(x, 0.0001, 0.9999);
    y = clamp(y, 0.0001, 0.9999);
    float x_ = (x + 1.0) * 0.5;
    float y_ = (y + 1.0) * 0.5;
    return floor(x_ * 100.0) + y_;
}
)", 1);
    }
#pragma endregion

#pragma region UnpackFloat16Into2Floats
    if (ShaderHelper::lacksDefinition(code, "UnpackFloat16Into2Floats", "vec2 UnpackFloat16Into2Floats")) {
        ShaderHelper::insertStringAtLine(code, R"(
vec2 UnpackFloat16Into2Floats(float i){
    vec2 res;
    res.y = i - floor(i);
    res.x = (i - res.y) * 0.01;
    res.x = (res.x - 0.5) * 2.0;
    res.y = (res.y - 0.5) * 2.0;
    return res;
}
)", 1);
    }
#pragma endregion

#pragma region Pack2FloatIntoFloat32
    if (ShaderHelper::lacksDefinition(code, "Pack2FloatIntoFloat32", "float Pack2FloatIntoFloat32")) {
        ShaderHelper::insertStringAtLine(code, R"(
float Pack2FloatIntoFloat32(float x,float y){
    x = clamp(x,0.0001,0.9999);
    y = clamp(y,0.0001,0.9999);
    float _x = (x + 1.0) * 0.5;
    float _y = (y + 1.0) * 0.5;
    return floor(_x * 1000.0) + _y;
}
)", 1);
    }
#pragma endregion

#pragma region UnpackFloat32Into2Floats
    if (ShaderHelper::lacksDefinition(code, "UnpackFloat32Into2Floats", "vec2 UnpackFloat32Into2Floats")) {
        ShaderHelper::insertStringAtLine(code, R"(
vec2 UnpackFloat32Into2Floats(float i){
    vec2 res;
    res.y = i - floor(i);
    res.x = (i - res.y) * 0.001;
    res.x = (res.x - 0.5) * 2.0;
    res.y = (res.y - 0.5) * 2.0;
    return res;
}
)", 1);
    }
#pragma endregion

#pragma region EncodeOctahedron
    if (ShaderHelper::lacksDefinition(code, "EncodeOctahedron", "vec2 EncodeOctahedron")) {
        ShaderHelper::insertStringAtLine(code, R"(
vec2 EncodeOctahedron(vec3 n) {
    if(   all(greaterThan(n, vec3(0.999, 0.999, 0.999)))   )
        return vec2(1.0, 1.0);
        n.xy /= dot(abs(n), vec3(1.0, 1.0, 1.0));
        return mix(n.xy, (1.0 - abs(n.yx)) * SignNotZero(n.xy), step(n.z, 0.0));
}
)", 1);
    }
#pragma endregion

#pragma region DecodeOctahedron
    if (ShaderHelper::lacksDefinition(code, "DecodeOctahedron", "vec3 DecodeOctahedron")) {
        ShaderHelper::insertStringAtLine(code, R"(
vec3 DecodeOctahedron(vec2 n) {
    if(    all(greaterThan(n, vec2(0.999, 0.999)))    )
        return vec3(1.0, 1.0, 1.0);
        vec3 v = vec3(n.xy, 1.0 - abs(n.x) - abs(n.y));
        if (v.z < 0) v.xy = (1.0 - abs(v.yx)) * SignNotZero(v.xy);
        return normalize(v);
}
)", 1);
    }
#pragma endregion

#pragma region Unpack32BitUIntTo4ColorFloats
    if (ShaderHelper::lacksDefinition(code, "Unpack32BitUIntTo4ColorFloats", "vec4 Unpack32BitUIntTo4ColorFloats")) {
        ShaderHelper::insertStringAtLine(code, R"(
vec4 Unpack32BitUIntTo4ColorFloats(uint n) {
    int nasInt = int(n);
    vec4 ret;
    ret.r = (nasInt >> 24) & 255;
    ret.g = (nasInt >> 16) & 255;
    ret.b = (nasInt >> 8) & 255;
    ret.a = (nasInt & 255);
    return ret * 0.00392156862;
}
)", 1);
    }
#pragma endregion

#pragma region Unpack16BitUIntTo4ColorFloats
    if (ShaderHelper::lacksDefinition(code, "Unpack16BitUIntTo4ColorFloats", "vec4 Unpack16BitUIntTo4ColorFloats")) {
        ShaderHelper::insertStringAtLine(code, R"(
vec4 Unpack16BitUIntTo4ColorFloats(uint n) {
    int nasInt = int(n);
    vec4 ret;
    ret.r = (nasInt >> 12) & 15;
    ret.g = (nasInt >> 8) & 15;
    ret.b = (nasInt >> 4) & 15;
    ret.a = (nasInt & 15);
    return ret * 0.0666666666;
}
)", 1);
    }
#pragma endregion

#pragma region SignNotZero
    if (ShaderHelper::lacksDefinition(code, "SignNotZero", "vec2 SignNotZero")) {
        ShaderHelper::insertStringAtLine(code, R"(
vec2 SignNotZero(vec2 v) {
    return vec2(v.x >= 0 ? 1.0 : -1.0, v.y >= 0 ? 1.0 : -1.0);
}
)", 1);
    }
#pragma endregion

}