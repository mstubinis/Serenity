#include <core/engine/renderer/opengl/glsl/Lighting.h>

#include <core/engine/renderer/opengl/Extensions.h>
#include <core/engine/shaders/ShaderHelper.h>
#include <core/engine/materials/MaterialEnums.h>

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/lexical_cast.hpp>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;


void opengl::glsl::Lighting::convert(string& code, const unsigned int& versionNumber, const ShaderType::Type& shaderType) {

#pragma region Beckmann Distribution
    if (ShaderHelper::sfind(code, "BeckmannDist(")) {
        if (!ShaderHelper::sfind(code, "float BeckmannDist(")) {
            const string beckmann =
                "float BeckmannDist(float cos2a, float _alpha){//generated\n"
                "    float b = (1.0 - cos2a) / (cos2a * _alpha);\n"
                "    return (exp(-b)) / (KPI * _alpha * cos2a * cos2a);\n"
                "}\n";
            ShaderHelper::insertStringAtLine(code, beckmann, 1);
        }
    }
#pragma endregion

#pragma region GGX Distribution
    if (ShaderHelper::sfind(code, "GGXDist(")) {
        if (!ShaderHelper::sfind(code, "float GGXDist(")) {
            const string ggx =
                "float GGXDist(float NdotHSquared, float alphaSquared){//generated\n"
                "    float denom = (NdotHSquared * (alphaSquared - 1.0) + 1.0);\n"
                "    denom = KPI * denom * denom;\n"
                "    return alphaSquared / denom;\n"
                "}\n";
            ShaderHelper::insertStringAtLine(code, ggx, 1);
        }
    }
#pragma endregion

#pragma region Geometry Schlick GGX Distribution
    if (ShaderHelper::sfind(code, "GeometrySchlickGGX(")) {
        if (!ShaderHelper::sfind(code, "float GeometrySchlickGGX(")) {
            const string geo_slick_ggx =
                "float GeometrySchlickGGX(float NdotV, float a){//generated\n"
                "    float k = a * 0.125;\n"
                "    float denom = NdotV * (1.0 - k) + k;\n"
                "    return NdotV / denom;\n"
                "}\n";
            ShaderHelper::insertStringAtLine(code, geo_slick_ggx, 1);
        }
    }
#pragma endregion

#pragma region Schlick Frensel
    if (ShaderHelper::sfind(code, "SchlickFrensel(")) {
        if (!ShaderHelper::sfind(code, "vec3 SchlickFrensel(")) {
            const string schlick_frensel =
                "vec3 SchlickFrensel(float theta, vec3 _F0){//generated\n"
                "    vec3 ret = _F0 + (ConstantOneVec3 - _F0) * pow(1.0 - theta,5.0);\n"
                "    return ret;\n"
                "}\n";
            ShaderHelper::insertStringAtLine(code, schlick_frensel, 1);
        }
    }
#pragma endregion

#pragma region Diffuse oryen nayar
    if (ShaderHelper::sfind(code, "DiffuseOrenNayar(")) {
        if (!ShaderHelper::sfind(code, "float DiffuseOrenNayar(")) {
            const string diffuse_oren_nayar =
                "float DiffuseOrenNayar(vec3 _ViewDir, vec3 _LightDir, float _NdotL, float _VdotN, float _alpha){//generated\n"
                "    float A = 1.0 - 0.5 * _alpha / (_alpha + 0.33);\n"
                "    float B = 0.45 * _alpha / (_alpha + 0.09);\n"
                "    float cosAzimuthSinPolarTanPolar = (dot(_LightDir, _ViewDir) - _VdotN * _NdotL) / max(_VdotN, _NdotL);\n"
                "    return (A + B * max(0.0, cosAzimuthSinPolarTanPolar));\n"
                "}\n";
            ShaderHelper::insertStringAtLine(code, diffuse_oren_nayar, 1);
        }
    }
#pragma endregion

#pragma region Diffuse Ashikhmin Shirley
    if (ShaderHelper::sfind(code, "DiffuseAshikhminShirley(")) {
        if (!ShaderHelper::sfind(code, "vec3 DiffuseAshikhminShirley(")) {
            const string diffuse_ash_shir =
                "vec3 DiffuseAshikhminShirley(float _smoothness, vec3 _MaterialAlbedoTexture, float _NdotL, float _VdotN){\n"
                "    vec3 ret;\n"
                "    float s = clamp(_smoothness,0.01,0.76);\n" //this lighting model has to have some form of roughness in it to look good. cant be 1.0
                "    vec3 A = (28.0 * _MaterialAlbedoTexture) / vec3(23.0 * KPI);\n"
                "    float B = 1.0 - s;\n"
                "    float C = (1.0 - pow((1.0 - (_NdotL * 0.5)),5.0));\n"
                "    float D = (1.0 - pow((1.0 - (_VdotN * 0.5)),5.0));\n"
                "    ret = A * B * C * D;\n"
                "    ret *= KPI;\n" //i know this isnt proper, but the diffuse component is *way* too dark otherwise...
                "    return ret;\n"
                "}\n";
            ShaderHelper::insertStringAtLine(code, diffuse_ash_shir, 1);
        }
    }
#pragma endregion

#pragma region Specular Blinn Phong
    if (ShaderHelper::sfind(code, "SpecularBlinnPhong(")) {
        if (!ShaderHelper::sfind(code, "vec3 SpecularBlinnPhong(")) {
            const string specular_blinn_phong =
                "vec3 SpecularBlinnPhong(float _smoothness,float _NdotH){\n"
                "    float gloss = exp2(10.0 * _smoothness + 1.0);\n"
                "    float kS = (8.0 + gloss ) / (8.0 * KPI);\n"
                "    return vec3(kS * pow(_NdotH, gloss));\n"
                "}\n";
            ShaderHelper::insertStringAtLine(code, specular_blinn_phong, 1);
        }
    }
#pragma endregion

#pragma region Specular Phong
    if (ShaderHelper::sfind(code, "SpecularPhong(")) {
        if (!ShaderHelper::sfind(code, "vec3 SpecularPhong(")) {
            const string specular_phong =
                "vec3 SpecularPhong(float _smoothness,vec3 _LightDir,vec3 _PxlNormal,vec3 _ViewDir){\n"
                "    float gloss = exp2(10.0 * _smoothness + 1.0);\n"
                "    float kS = (2.0 + gloss ) / (2.0 * KPI);\n"
                "    vec3 Reflect = reflect(-_LightDir, _PxlNormal);\n"
                "    float VdotR = max(0.0, dot(_ViewDir,Reflect));\n"
                "    return vec3(kS * pow(VdotR, gloss));\n"
                "}\n";
            ShaderHelper::insertStringAtLine(code, specular_phong, 1);
        }
    }
#pragma endregion

#pragma region Specular Gaussian
    if (ShaderHelper::sfind(code, "SpecularGaussian(")) {
        if (!ShaderHelper::sfind(code, "vec3 SpecularGaussian(")) {
            const string specular_gaussian =
                "vec3 SpecularGaussian(float _NdotH,float _smoothness){\n"
                "    float b = acos(_NdotH);\n" //this might also be cos. find out
                "    float fin = b / _smoothness;\n"
                "    return vec3(exp(-fin*fin));\n"
                "}\n";
            ShaderHelper::insertStringAtLine(code, specular_gaussian, 1);
        }
    }
#pragma endregion
}