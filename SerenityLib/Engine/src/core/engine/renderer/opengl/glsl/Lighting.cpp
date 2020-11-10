#include <core/engine/utils/PrecompiledHeader.h>
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
using namespace Engine::priv;

void opengl::glsl::Lighting::convert(std::string& code, unsigned int versionNumber, ShaderType shaderType) {

#pragma region Projection Light
    if (ShaderHelper::sfind(code, "CalcProjectionLight(")) {
        if (!ShaderHelper::sfind(code, "vec3 CalcProjectionLight(")) {
            ShaderHelper::insertStringRightBeforeLineContent(code, 
                "vec3 CalcProjectionLight(in Light inLight, vec3 A, vec3 B,vec3 PxlWorldPos, vec3 PxlNormal, float Specular, vec3 Albedo, float SSAO, vec2 MetalSmooth, float MatAlpha, vec3 MatF0, float MatTypeDiffuse, float MatTypeSpecular, float AO){//generated\n"
                /*
                //TODO: implement
                "    vec3 BMinusA = B - A;\n"
                "    vec3 CMinusA = PxlWorldPos - A;\n"
                "    float Dist = length(BMinusA);\n"
                "    vec3 _Normal = BMinusA / Dist;\n"
                "    float t = clamp(dot(CMinusA, _Normal / Dist), 0.0, 1.0);\n"
                "    vec3 LightPos = A + t * BMinusA;\n"
                "    vec3 c = CalcPointLight(inLight, LightPos, PxlWorldPos, PxlNormal, Specular, Albedo, SSAO, MetalSmooth, MatAlpha, MatF0, MatTypeDiffuse, MatTypeSpecular, AO);\n"
                "    return c;\n"
                */
                "    return vec3(0.0, 1.0, 0.0);\n"
                "}\n"
            , "void main(");
        }
    }
#pragma endregion

#pragma region Rod Light
    if (ShaderHelper::sfind(code, "CalcRodLight(")) {
        if (!ShaderHelper::sfind(code, "vec3 CalcRodLight(")) {
            ShaderHelper::insertStringRightBeforeLineContent(code, 
                "vec3 CalcRodLight(in Light inLight, vec3 A, vec3 B, vec3 PxlWorldPos, vec3 PxlNormal, float Specular, vec3 Albedo, float SSAO, vec2 MetalSmooth, float MatAlpha, vec3 MatF0, float MatTypeDiffuse, float MatTypeSpecular, float AO){//generated\n"
                "    vec3 BMinusA = B - A;\n"
                "    vec3 CMinusA = PxlWorldPos - A;\n"
                "    float Dist = length(BMinusA);\n"
                "    vec3 _Normal = BMinusA / Dist;\n"
                "    float t = clamp(dot(CMinusA, _Normal / Dist), 0.0, 1.0);\n"
                "    vec3 LightPos = A + t * BMinusA;\n"
                "    vec3 c = CalcPointLight(inLight, LightPos, PxlWorldPos, PxlNormal, Specular, Albedo, SSAO, MetalSmooth, MatAlpha, MatF0, MatTypeDiffuse, MatTypeSpecular, AO);\n"
                "    return c;\n"
                "}\n"
            , "void main(");
        }
    }
#pragma endregion

#pragma region Spot Light
    if (ShaderHelper::sfind(code, "CalcSpotLight(")) {
        if (!ShaderHelper::sfind(code, "vec3 CalcSpotLight(")) {
            ShaderHelper::insertStringRightBeforeLineContent(code, 
                "vec3 CalcSpotLight(in Light inLight, vec3 SpotLightDir, vec3 LightPos,vec3 PxlWorldPos, vec3 PxlNormal, float Specular, vec3 Albedo, float SSAO, vec2 MetalSmooth, float MatAlpha, vec3 MatF0, float MatTypeDiffuse, float MatTypeSpecular, float AO){//generated\n"
                "    vec3 LightDir = normalize(LightPos - PxlWorldPos);\n"
                "    vec3 c = CalcPointLight(inLight, LightPos, PxlWorldPos, PxlNormal, Specular, Albedo, SSAO, MetalSmooth, MatAlpha, MatF0, MatTypeDiffuse, MatTypeSpecular, AO);\n"
                "    float cosAngle = dot(LightDir, -SpotLightDir);\n"
                "    float spotEffect = smoothstep(inLight.DataE.y, inLight.DataE.x, cosAngle);\n"
                "    return c * spotEffect;\n"
                "}\n"
            , "vec3 CalcRodLight(");
        }
    }
#pragma endregion

#pragma region Point Light
    if (ShaderHelper::sfind(code, "CalcPointLight(")) {
        if (!ShaderHelper::sfind(code, "vec3 CalcPointLight(")) {
            ShaderHelper::insertStringRightBeforeLineContent(code, 
                "vec3 CalcPointLight(in Light inLight, vec3 LightPos,vec3 PxlWorldPos, vec3 PxlNormal, float Specular, vec3 Albedo, float SSAO, vec2 MetalSmooth, float MatAlpha, vec3 MatF0, float MatTypeDiffuse, float MatTypeSpecular, float AO){//generated\n"
                "    vec3 RawDirection = LightPos - PxlWorldPos;\n"
                "    float Dist = length(RawDirection);\n"
                "    vec3 LightDir = RawDirection / Dist;\n"
                "    vec3 c = CalcLightInternal(inLight, LightDir, PxlWorldPos, PxlNormal, Specular, Albedo, SSAO, MetalSmooth, MatAlpha, MatF0, MatTypeDiffuse, MatTypeSpecular, AO);\n"
                "    float attenuation = CalculateAttenuation(inLight, Dist, 1.0);\n"
                "    return c * attenuation;\n"
                "}\n"
            , "vec3 CalcSpotLight(");
        }
    }
#pragma endregion

#pragma region Lighting Internal Function
    if (ShaderHelper::sfind(code, "CalcLightInternal(")) {
        if (!ShaderHelper::sfind(code, "vec3 CalcLightInternal(")) {
            ShaderHelper::insertStringRightBeforeLineContent(code, 
                "vec3 CalcLightInternal(in Light currentLight, vec3 LightDir, vec3 PxlWorldPos, vec3 PxlNormal, float Specular, vec3 Albedo, float SSAO, vec2 MetalSmooth, float MatAlpha, vec3 MatF0, float MatTypeDiffuse, float MatTypeSpecular, float AO){//generated\n"
                "    vec3 LightDiffuseColor  = currentLight.DataD.xyz;\n"
                "    vec3 LightSpecularColor = currentLight.DataD.xyz * Specular;\n"
                "    vec3 TotalLight         = ConstantZeroVec3;\n"
                "    vec3 SpecularFactor     = ConstantZeroVec3;\n"

                "    float ao                = AO * SSAO;\n"
                "    float metalness         = MetalSmooth.x;\n"
                "    float smoothness        = MetalSmooth.y;\n"
                "    vec3 F0                 = mix(MatF0, Albedo, vec3(metalness));\n"
                "    vec3 Frensel            = F0;\n"

                "    float roughness         = 1.0 - smoothness;\n"
                "    float roughnessSquared  = roughness * roughness;\n"

                "    vec3 ViewDir            = normalize(CameraPosition - PxlWorldPos);\n"
                "    vec3 Half               = normalize(LightDir + ViewDir);\n"
                "    float NdotL             = clamp(dot(PxlNormal, LightDir), 0.0, 1.0);\n"
                "    float NdotH             = clamp(dot(PxlNormal, Half), 0.0, 1.0);\n"
                "    float VdotN             = clamp(dot(ViewDir, PxlNormal), 0.0, 1.0);\n"
                "    float VdotH             = clamp(dot(ViewDir, Half), 0.0, 1.0);\n"

                "    if(MatTypeDiffuse == 2.0){\n"
                "        LightDiffuseColor *= DiffuseOrenNayar(ViewDir, LightDir, NdotL, VdotN, roughnessSquared);\n"
                "    }else if(MatTypeDiffuse == 3.0){\n"
                "        LightDiffuseColor *= DiffuseAshikhminShirley(smoothness, Albedo, NdotL, VdotN);\n"
                "    }else if(MatTypeDiffuse == 4.0){\n"//this is minneart
                "        LightDiffuseColor *= pow(VdotN * NdotL, smoothness);\n"
                "    }\n"
                "    if(MatTypeSpecular == 1.0){\n"
                "        SpecularFactor = SpecularBlinnPhong(smoothness, NdotH);\n"
                "    }else if(MatTypeSpecular == 2.0){\n"
                "        SpecularFactor = SpecularPhong(smoothness, LightDir, PxlNormal, ViewDir);\n"
                "    }else if(MatTypeSpecular == 3.0){\n"
                "        SpecularFactor = SpecularGGX(Frensel, LightDir, Half, roughnessSquared, NdotH, F0, NdotL);\n"
                "    }else if(MatTypeSpecular == 4.0){\n"
                "        SpecularFactor = SpecularCookTorrance(Frensel, F0, VdotH, NdotH, roughnessSquared, VdotN, roughness, NdotL);\n"
                "    }else if(MatTypeSpecular == 5.0){\n"
                "        SpecularFactor = SpecularGaussian(NdotH, smoothness);\n"
                "    }else if(MatTypeSpecular == 6.0){\n"
                "        SpecularFactor = vec3(BeckmannDist(NdotH, roughnessSquared));\n"
                "    }else if(MatTypeSpecular == 7.0){\n"
                "        SpecularFactor = SpecularAshikhminShirley(PxlNormal, Half, NdotH, LightDir, NdotL, VdotN);\n"
                "    }\n"
                "    LightDiffuseColor     *= currentLight.DataA.y;\n"
                "    LightSpecularColor    *= (SpecularFactor * currentLight.DataA.z);\n"

                "    vec3 componentDiffuse  = ConstantOneVec3 - Frensel;\n"
                "    componentDiffuse      *= 1.0 - metalness;\n"

                "    TotalLight             = (componentDiffuse * ao) * Albedo;\n"
                "    TotalLight            /= KPI;\n"
                "    TotalLight            += LightSpecularColor;\n"
                "    TotalLight            *= (LightDiffuseColor * NdotL);\n"
                "    TotalLight            *= MatAlpha;\n"
                "    return TotalLight;\n"
                "}\n"
            , "vec3 CalcPointLight(");
        }
    }
#pragma endregion

#pragma region GI Lighting Internal

    if (ShaderHelper::sfind(code, "CalcGILight(")) {
        if (!ShaderHelper::sfind(code, "vec4 CalcGILight(")) {
            ShaderHelper::insertStringRightBeforeMainFunc(code,
                "vec4 CalcGILight(float SSAO, vec3 Normals, vec3 Albedo, vec3 WorldPosition, float AO, float Metal, float Smooth, float Glow, vec3 MatF0, float MatAlpha, vec3 GIContribution){//generated\n"
                "    vec3 ViewDir          = normalize(CameraPosition - WorldPosition);\n"
                "    vec3 R                = reflect(-ViewDir, Normals);\n"
                "    float VdotN           = max(0.0, dot(ViewDir, Normals));\n"
                "    float ao              = AO * SSAO;\n"
                "    vec3 Frensel          = mix(MatF0, Albedo, vec3(Metal));\n"
                "    float roughness       = 1.0 - Smooth;\n"
                "    vec3 irradianceColor  = textureCube(irradianceMap, Normals).rgb;\n"
                "    vec3 kS               = SchlickFrenselRoughness(VdotN, Frensel, roughness);\n"
                "    vec3 kD               = ConstantOneVec3 - kS;\n"
                "    kD                   *= 1.0 - Metal;\n"
                "    vec3 GIDiffuse        = irradianceColor * Albedo * kD * GIContribution.x;\n"

                "    vec3 prefilteredColor = textureCubeLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;\n"
                "    vec2 brdf             = texture2D(brdfLUT, vec2(VdotN, roughness)).rg;\n"
                "    vec3 GISpecular       = prefilteredColor * (kS * brdf.x + brdf.y) * GIContribution.y;\n"

                "    vec3 TotalIrradiance  = (GIDiffuse + GISpecular) * ao;\n"
                "    TotalIrradiance       = pow(TotalIrradiance, vec3(1.0 / ScreenData.y));\n" //ScreenData.y is gamma

                "    vec4 FinalColor       = vec4(0.0, 0.0, 0.0, 0.0);\n"
                "    FinalColor           += (vec4(TotalIrradiance, 1.0) * vec4(vec3(GIContribution.z), 1.0)) * MatAlpha;\n"
                "    FinalColor.rgb        = max(FinalColor.rgb, Glow * Albedo);\n"
                "    return FinalColor;\n"
                "}\n"
            );
        }
    }

#pragma endregion

#pragma region Diffuse oryen nayar
    if (ShaderHelper::sfind(code, "DiffuseOrenNayar(")) {
        if (!ShaderHelper::sfind(code, "float DiffuseOrenNayar(")) {
            ShaderHelper::insertStringAtLine(code, 
                "float DiffuseOrenNayar(vec3 _ViewDir, vec3 _LightDir, float _NdotL, float _VdotN, float _roughnessSquared){//generated\n"
                "    float A = 1.0 - 0.5 * _roughnessSquared / (_roughnessSquared + 0.33);\n"
                "    float B = 0.45 * _roughnessSquared / (_roughnessSquared + 0.09);\n"
                "    float cosAzimuthSinPolarTanPolar = (dot(_LightDir, _ViewDir) - _VdotN * _NdotL) / max(_VdotN, _NdotL);\n"
                "    return (A + B * max(0.0, cosAzimuthSinPolarTanPolar));\n"
                "}\n"
            , 1);
        }
    }
#pragma endregion

#pragma region Diffuse Ashikhmin Shirley
    if (ShaderHelper::sfind(code, "DiffuseAshikhminShirley(")) {
        if (!ShaderHelper::sfind(code, "vec3 DiffuseAshikhminShirley(")) {
            ShaderHelper::insertStringAtLine(code, 
                "vec3 DiffuseAshikhminShirley(float _smoothness, vec3 _MaterialAlbedoTexture, float _NdotL, float _VdotN){//generated\n"
                "    vec3 ret;\n"
                "    float s = clamp(_smoothness,0.01,0.76);\n" //this lighting model has to have some form of roughness in it to look good. cant be 1.0
                "    vec3 A = (28.0 * _MaterialAlbedoTexture) / vec3(23.0 * KPI);\n"
                "    float B = 1.0 - s;\n"
                "    float C = (1.0 - pow((1.0 - (_NdotL * 0.5)),5.0));\n"
                "    float D = (1.0 - pow((1.0 - (_VdotN * 0.5)),5.0));\n"
                "    ret = A * B * C * D;\n"
                "    ret *= KPI;\n" //i know this isnt proper, but the diffuse component is *way* too dark otherwise...
                "    return ret;\n"
                "}\n"
            , 1);
        }
    }
#pragma endregion

#pragma region Specular Blinn Phong
    if (ShaderHelper::sfind(code, "SpecularBlinnPhong(")) {
        if (!ShaderHelper::sfind(code, "vec3 SpecularBlinnPhong(")) {
            ShaderHelper::insertStringAtLine(code, 
                "vec3 SpecularBlinnPhong(float _smoothness,float _NdotH){//generated\n"
                "    float gloss = exp2(10.0 * _smoothness + 1.0);\n"
                "    float kS = (8.0 + gloss ) / (8.0 * KPI);\n"
                "    return vec3(kS * pow(_NdotH, gloss));\n"
                "}\n"
            , 1);
        }
    }
#pragma endregion

#pragma region Specular Phong
    if (ShaderHelper::sfind(code, "SpecularPhong(")) {
        if (!ShaderHelper::sfind(code, "vec3 SpecularPhong(")) {
            ShaderHelper::insertStringAtLine(code, 
                "vec3 SpecularPhong(float _smoothness,vec3 _LightDir,vec3 _PxlNormal,vec3 _ViewDir){//generated\n"
                "    float gloss = exp2(10.0 * _smoothness + 1.0);\n"
                "    float kS = (2.0 + gloss ) / (2.0 * KPI);\n"
                "    vec3 Reflect = reflect(-_LightDir, _PxlNormal);\n"
                "    float VdotR = max(0.0, dot(_ViewDir,Reflect));\n"
                "    return vec3(kS * pow(VdotR, gloss));\n"
                "}\n"
            , 1);
        }
    }
#pragma endregion

#pragma region Specular Gaussian
    if (ShaderHelper::sfind(code, "SpecularGaussian(")) {
        if (!ShaderHelper::sfind(code, "vec3 SpecularGaussian(")) {
            ShaderHelper::insertStringAtLine(code, 
                "vec3 SpecularGaussian(float _NdotH,float _smoothness){//generated\n"
                "    float b = acos(_NdotH);\n" //this might also be cos. find out
                "    float fin = b / _smoothness;\n"
                "    return vec3(exp(-fin*fin));\n"
                "}\n"
            , 1);
        }
    }
#pragma endregion

#pragma region Specular GGX
    if (ShaderHelper::sfind(code, "SpecularGGX(")) {
        if (!ShaderHelper::sfind(code, "vec3 SpecularGGX(")) {
            ShaderHelper::insertStringAtLine(code, 
                "vec3 SpecularGGX(inout vec3 _Frensel,vec3 _LightDir,vec3 _Half,float _roughnessSquared,float _NdotH,vec3 F0_,float _NdotL){//generated\n"
                "    float LdotH = max(0.0, dot(_LightDir, _Half));\n"
                "    float alphaSqr = _roughnessSquared * _roughnessSquared;\n"
                "    float denom = _NdotH * _NdotH * (alphaSqr - 1.0) + 1.0;\n"
                "    float NDF = alphaSqr / (KPI * denom * denom);\n"
                "    _Frensel = SchlickFrensel(LdotH, F0_);\n"
                "    float k = _roughnessSquared * 0.5;\n"
                "    float k2 = k * k;\n"
                "    float invk2 = 1.0 - k2;\n"
                "    float G = 1.0 / (LdotH * LdotH * invk2 + k2);\n"
                "    return _NdotL * NDF * _Frensel * G;\n"
                "}\n"
            , 1);
        }
    }
#pragma endregion

#pragma region Specular Cook Torrance
    if (ShaderHelper::sfind(code, "SpecularCookTorrance(")) {
        if (!ShaderHelper::sfind(code, "vec3 SpecularCookTorrance(")) {
            ShaderHelper::insertStringAtLine(code, 
                "vec3 SpecularCookTorrance(inout vec3 _Frensel,vec3 _F0,float _VdotH,float _NdotH,float _roughnessSquared,float _VdotN,float _roughness,float _NdotL){//generated\n"
                "    _Frensel = SchlickFrensel(_VdotH,_F0);\n"
                "    float NDF = GGXDist(_NdotH * _NdotH, _roughnessSquared * _roughnessSquared);\n"
                "    float roughnessPlusOne = _roughness + 1.0;\n"
                "    float a = roughnessPlusOne * roughnessPlusOne;\n"
                "    float G = GeometrySchlickGGX(_VdotN,a) * GeometrySchlickGGX(_NdotL,a);\n"
                "    vec3 Top = NDF * _Frensel * G;\n"
                "    float Bottom = max(4.0 * _VdotN * _NdotL,0.0);\n"
                "    return Top / (Bottom + 0.001);\n"
                "}\n"
            , 1);
        }
    }
#pragma endregion

#pragma region Specular Ashikhmin Shirley
    if (ShaderHelper::sfind(code, "SpecularAshikhminShirley(")) {
        if (!ShaderHelper::sfind(code, "vec3 SpecularAshikhminShirley(")) {
            ShaderHelper::insertStringAtLine(code, 
                "vec3 SpecularAshikhminShirley(vec3 _PxlNormal,vec3 _Half,float _NdotH,vec3 _LightDir,float _NdotL,float _VdotN){//generated\n"
                "    const float Nu = 1000.0;\n"//make these controllable uniforms
                "    const float Nv = 1000.0;\n"//make these controllable uniforms
                "    vec3 epsilon = vec3(1.0,0.0,0.0);\n"
                "    vec3 tangent = normalize(cross(_PxlNormal,epsilon));\n"
                "    vec3 bitangent = normalize(cross(_PxlNormal,tangent));\n"
                "    float hdotT = dot(_Half,tangent);\n"
                "    float hDotB = dot(_Half,bitangent);\n"
                "    float A = sqrt( (Nu + 1.0) * (Nv + 1.0) );\n"
                "    float B = pow(_NdotH,((Nu * hdotT * hdotT + Nv * hDotB * hDotB) / (1.0 - (_NdotH * _NdotH)) ));\n"
                "    float HdotL = max(0.0, dot(_Half, _LightDir));\n"
                "    float C = 8.0 * KPI * HdotL * max(_NdotL,_VdotN);\n"
                "    return vec3((A * B) / C);\n"
                "}\n"
            , 1);
        }
    }
#pragma endregion

#pragma region Beckmann Distribution
    if (ShaderHelper::sfind(code, "BeckmannDist(")) {
        if (!ShaderHelper::sfind(code, "float BeckmannDist(")) {
            ShaderHelper::insertStringAtLine(code, 
                "float BeckmannDist(float cos2a, float _alpha){//generated\n"
                "    float b = (1.0 - cos2a) / (cos2a * _alpha);\n"
                "    return (exp(-b)) / (KPI * _alpha * cos2a * cos2a);\n"
                "}\n"
            , 1);
        }
    }
#pragma endregion

#pragma region GGX Distribution
    if (ShaderHelper::sfind(code, "GGXDist(")) {
        if (!ShaderHelper::sfind(code, "float GGXDist(")) {
            ShaderHelper::insertStringAtLine(code, 
                "float GGXDist(float NdotHSquared, float alphaSquared){//generated\n"
                "    float denom = (NdotHSquared * (alphaSquared - 1.0) + 1.0);\n"
                "    denom = KPI * denom * denom;\n"
                "    return alphaSquared / denom;\n"
                "}\n"
            , 1);
        }
    }
#pragma endregion

#pragma region Geometry Schlick GGX Distribution
    if (ShaderHelper::sfind(code, "GeometrySchlickGGX(")) {
        if (!ShaderHelper::sfind(code, "float GeometrySchlickGGX(")) {
            ShaderHelper::insertStringAtLine(code, 
                "float GeometrySchlickGGX(float NdotV, float a){//generated\n"
                "    float k = a * 0.125;\n"
                "    float denom = NdotV * (1.0 - k) + k;\n"
                "    return NdotV / denom;\n"
                "}\n"
            , 1);
        }
    }
#pragma endregion

#pragma region Schlick Frensel
    if (ShaderHelper::sfind(code, "SchlickFrensel(")) {
        if (!ShaderHelper::sfind(code, "vec3 SchlickFrensel(")) {
            ShaderHelper::insertStringAtLine(code, 
                "vec3 SchlickFrensel(float inTheta, vec3 inF0){//generated\n"
                "    return inF0 + (ConstantOneVec3 - inF0) * pow(1.0 - inTheta, 5.0);\n"
                "}\n"
            , 1);
        }
    }
#pragma endregion

#pragma region Attenuation Function
    if (ShaderHelper::sfind(code, "CalculateAttenuation(")) {
        if (!ShaderHelper::sfind(code, "float CalculateAttenuation(")) {
            ShaderHelper::insertStringRightBeforeLineContent(code, 
                "float CalculateAttenuation(in Light currentLight, float Dist, float radius){//generated\n"
                "   float attenuation = 0.0;\n"
                "   if(currentLight.DataE.z == 0.0){\n"       //constant
                "       attenuation = 1.0 / max(1.0 , currentLight.DataB.z);\n"
                "   }else if(currentLight.DataE.z == 1.0){\n" //distance
                "       attenuation = 1.0 / max(1.0 , Dist);\n"
                "   }else if(currentLight.DataE.z == 2.0){\n" //distance squared
                "       attenuation = 1.0 / max(1.0 , Dist * Dist);\n"
                "   }else if(currentLight.DataE.z == 3.0){\n" //constant linear exponent
                "       attenuation = 1.0 / max(1.0 , currentLight.DataB.z + (currentLight.DataB.w * Dist) + (currentLight.DataC.x * Dist * Dist));\n"
                "   }else if(currentLight.DataE.z == 4.0){\n" //distance radius squared
                "       attenuation = 1.0 / max(1.0 ,pow((Dist / radius) + 1.0, 2.0));\n"
                "   }\n"
                "   return attenuation;\n"
                "}\n"
            , "vec3 CalcLightInternal(");
        }
    }
#pragma endregion

}