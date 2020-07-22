#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/renderer/opengl/glsl/Compression.h>
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


void opengl::glsl::Compression::convert(string& code, const unsigned int versionNumber) {
#pragma region Unpack3FloatsInto1FloatUnsigned
    if (ShaderHelper::sfind(code, "Unpack3FloatsInto1FloatUnsigned")) {
        if (!ShaderHelper::sfind(code, "vec3 Unpack3FloatsInto1FloatUnsigned")) {
            const string inserted_code =
                "vec3 Unpack3FloatsInto1FloatUnsigned(float v){\n"
                "    vec3 ret;\n"
                "    ret.r = mod(v,          1.0);\n"
                "    ret.g = mod(v * 256.0,  1.0);\n"
                "    ret.b = mod(v * 65536.0,1.0);\n"
                "    return ret;\n"
                "}\n";
            ShaderHelper::insertStringAtLine(code, inserted_code, 1);
        }
    }
#pragma endregion

#pragma region Unpack3FloatsInto1Float
    if (ShaderHelper::sfind(code, "Unpack3FloatsInto1Float")) {
        if (!ShaderHelper::sfind(code, "vec3 Unpack3FloatsInto1Float")) {
            const string inserted_code =
                "vec3 Unpack3FloatsInto1Float(float v){\n"
                "    vec3 ret;\n"
                "    ret.r = mod(v,          1.0);\n"
                "    ret.g = mod(v * 256.0,  1.0);\n"
                "    ret.b = mod(v * 65536.0,1.0);\n"
                "    ret = ret * 2.0 - 1.0;\n"
                "    return ret;\n"
                "}\n";
            ShaderHelper::insertStringAtLine(code, inserted_code, 1);
        }
    }
#pragma endregion

#pragma region Pack2FloatIntoFloat16
    if (ShaderHelper::sfind(code, "Pack2FloatIntoFloat16")) {
        if (!ShaderHelper::sfind(code, "float Pack2FloatIntoFloat16")) {
            const string inserted_code =
                "float Pack2FloatIntoFloat16(float x,float y){\n"
                "    x = clamp(x,0.0001,0.9999);\n"
                "    y = clamp(y,0.0001,0.9999);\n"
                "    float _x = (x + 1.0) * 0.5;\n"
                "    float _y = (y + 1.0) * 0.5;\n"
                "    return floor(_x * 100.0) + _y;\n"
                "}\n";
            ShaderHelper::insertStringAtLine(code, inserted_code, 1);
        }
    }
#pragma endregion

#pragma region UnpackFloat16Into2Floats
    if (ShaderHelper::sfind(code, "UnpackFloat16Into2Floats")) {
        if (!ShaderHelper::sfind(code, "vec2 UnpackFloat16Into2Floats")) {
            const string inserted_code =
                "vec2 UnpackFloat16Into2Floats(float i){\n"
                "    vec2 res;\n"
                "    res.y = i - floor(i);\n"
                "    res.x = (i - res.y) * 0.01;\n"
                "    res.x = (res.x - 0.5) * 2.0;\n"
                "    res.y = (res.y - 0.5) * 2.0;\n"
                "    return res;\n"
                "}\n";
            ShaderHelper::insertStringAtLine(code, inserted_code, 1);
        }
    }
#pragma endregion

#pragma region Pack2FloatIntoFloat32
    if (ShaderHelper::sfind(code, "Pack2FloatIntoFloat32")) {
        if (!ShaderHelper::sfind(code, "float Pack2FloatIntoFloat32")) {
            const string inserted_code =
                "float Pack2FloatIntoFloat32(float x,float y){\n"
                "    x = clamp(x,0.0001,0.9999);\n"
                "    y = clamp(y,0.0001,0.9999);\n"
                "    float _x = (x + 1.0) * 0.5;\n"
                "    float _y = (y + 1.0) * 0.5;\n"
                "    return floor(_x * 1000.0) + _y;\n"
                "}\n";
            ShaderHelper::insertStringAtLine(code, inserted_code, 1);
        }
    }
#pragma endregion

#pragma region UnpackFloat32Into2Floats
    if (ShaderHelper::sfind(code, "UnpackFloat32Into2Floats")) {
        if (!ShaderHelper::sfind(code, "vec2 UnpackFloat32Into2Floats")) {
            const string inserted_code =
                "vec2 UnpackFloat32Into2Floats(float i){\n"
                "    vec2 res;\n"
                "    res.y = i - floor(i);\n"
                "    res.x = (i - res.y) * 0.001;\n"
                "    res.x = (res.x - 0.5) * 2.0;\n"
                "    res.y = (res.y - 0.5) * 2.0;\n"
                "    return res;\n"
                "}\n";
            ShaderHelper::insertStringAtLine(code, inserted_code, 1);
        }
    }
#pragma endregion

#pragma region EncodeOctahedron
    if (ShaderHelper::sfind(code, "EncodeOctahedron")) {
        if (!ShaderHelper::sfind(code, "vec2 EncodeOctahedron")) {
            const string inserted_code =
                "vec2 EncodeOctahedron(vec3 n) {\n"
                "    if(   all(greaterThan(n,ConstantAlmostOneVec3))   )\n"
                "        return ConstantOneVec2;\n"
                "	 n.xy /= dot(abs(n), ConstantOneVec3);\n"
                "	 return mix(n.xy, (1.0 - abs(n.yx)) * SignNotZero(n.xy), step(n.z, 0.0));\n"
                "}\n";
            ShaderHelper::insertStringAtLine(code, inserted_code, 1);
        }
    }
#pragma endregion

#pragma region DecodeOctahedron
    if (ShaderHelper::sfind(code, "DecodeOctahedron")) {
        if (!ShaderHelper::sfind(code, "vec3 DecodeOctahedron")) {
            const string inserted_code =
                "vec3 DecodeOctahedron(vec2 n) {\n"
                "    if(    all(greaterThan(n,ConstantAlmostOneVec2))    )\n"
                "        return ConstantOneVec3;\n"
                "	 vec3 v = vec3(n.xy, 1.0 - abs(n.x) - abs(n.y));\n"
                "	 if (v.z < 0) v.xy = (1.0 - abs(v.yx)) * SignNotZero(v.xy);\n"
                "	 return normalize(v);\n"
                "}\n";
            ShaderHelper::insertStringAtLine(code, inserted_code, 1);
        }
    }
#pragma endregion

#pragma region Unpack32BitUIntTo4ColorFloats
    if (ShaderHelper::sfind(code, "Unpack32BitUIntTo4ColorFloats")) {
        if (!ShaderHelper::sfind(code, "vec4 Unpack32BitUIntTo4ColorFloats")) {
            const string inserted_code =
                "vec4 Unpack32BitUIntTo4ColorFloats(uint n) {\n"
                "    int nasInt = int(n);\n"
                "    vec4 ret;\n"
                "    ret.r = (nasInt >> 24) & 255;\n"
                "    ret.g = (nasInt >> 16) & 255; \n"
                "    ret.b = (nasInt >> 8) & 255; \n"
                "    ret.a = (nasInt & 255); \n"
                "    return ret * 0.00392156862; \n"
                "}\n";
            ShaderHelper::insertStringAtLine(code, inserted_code, 1);
        }
    }
#pragma endregion

#pragma region SignNotZero
    if (ShaderHelper::sfind(code, "SignNotZero")) {
        if (!ShaderHelper::sfind(code, "vec2 SignNotZero")) {
            const string inserted_code =
                "vec2 SignNotZero(vec2 v) {\n"
                "    return vec2(v.x >= 0 ? 1.0 : -1.0, v.y >= 0 ? 1.0 : -1.0);\n"
                "}\n";
            ShaderHelper::insertStringAtLine(code, inserted_code, 1);
        }
    }
#pragma endregion

}