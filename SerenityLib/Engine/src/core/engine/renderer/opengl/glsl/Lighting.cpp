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
using namespace std;


void opengl::glsl::Lighting::convert(string& code, const unsigned int versionNumber, const ShaderType::Type shaderType) {

#pragma region Projection Light
    if (ShaderHelper::sfind(code, "CalcProjectionLight(")) {
        if (!ShaderHelper::sfind(code, "vec3 CalcProjectionLight(")) {
            const string rod_light =
                "vec3 CalcProjectionLight(in Light currentLight, vec3 A, vec3 B,vec3 PxlWorldPos, vec3 PxlNormal, vec2 uv){//generated\n"
                /*
                //TODO: implement
                "    vec3 BMinusA = B - A;\n"
                "    vec3 CMinusA = PxlWorldPos - A;\n"
                "    float Dist = length(BMinusA);\n"
                "    vec3 _Normal = BMinusA / Dist;\n"
                "    float t = clamp(dot(CMinusA, _Normal / Dist), 0.0, 1.0);\n"
                "    vec3 LightPos = A + t * BMinusA;\n"
                "    vec3 c = CalcPointLight(currentLight, LightPos, PxlWorldPos, PxlNormal, uv);\n"
                "    return c;\n"
                */
                "    return vec3(0.0, 1.0, 0.0);\n"
                "}\n";
            ShaderHelper::insertStringRightBeforeLineContent(code, rod_light, "void main(");
        }
    }
#pragma endregion

#pragma region Rod Light
    if (ShaderHelper::sfind(code, "CalcRodLight(")) {
        if (!ShaderHelper::sfind(code, "vec3 CalcRodLight(")) {
            const string rod_light =
                "vec3 CalcRodLight(in Light currentLight, vec3 A, vec3 B, vec3 PxlWorldPos, vec3 PxlNormal, vec2 uv){//generated\n"
                "    vec3 BMinusA = B - A;\n"
                "    vec3 CMinusA = PxlWorldPos - A;\n"
                "    float Dist = length(BMinusA);\n"
                "    vec3 _Normal = BMinusA / Dist;\n"
                "    float t = clamp(dot(CMinusA, _Normal / Dist), 0.0, 1.0);\n"
                "    vec3 LightPos = A + t * BMinusA;\n"
                "    vec3 c = CalcPointLight(currentLight, LightPos, PxlWorldPos, PxlNormal, uv);\n"
                "    return c;\n"
                "}\n";
            ShaderHelper::insertStringRightBeforeLineContent(code, rod_light, "void main(");
        }
    }
#pragma endregion

#pragma region Spot Light
    if (ShaderHelper::sfind(code, "CalcSpotLight(")) {
        if (!ShaderHelper::sfind(code, "vec3 CalcSpotLight(")) {
            const string spot_light =
                "vec3 CalcSpotLight(in Light currentLight, vec3 SpotLightDir, vec3 LightPos,vec3 PxlWorldPos, vec3 PxlNormal, vec2 uv){//generated\n"
                "    vec3 LightDir = normalize(LightPos - PxlWorldPos);\n"
                "    vec3 c = CalcPointLight(currentLight, LightPos, PxlWorldPos, PxlNormal, uv);\n"
                "    float cosAngle = dot(LightDir, -SpotLightDir);\n"
                "    float spotEffect = smoothstep(currentLight.DataE.y, currentLight.DataE.x, cosAngle);\n"
                "    return c * spotEffect;\n"
                "}\n";
            ShaderHelper::insertStringRightBeforeLineContent(code, spot_light, "vec3 CalcRodLight(");
        }
    }
#pragma endregion

#pragma region Point Light
    if (ShaderHelper::sfind(code, "CalcPointLight(")) {
        if (!ShaderHelper::sfind(code, "vec3 CalcPointLight(")) {
            const string point_light =
                "vec3 CalcPointLight(in Light currentLight, vec3 LightPos,vec3 PxlWorldPos, vec3 PxlNormal, vec2 uv){//generated\n"
                "    vec3 RawDirection = LightPos - PxlWorldPos;\n"
                "    float Dist = length(RawDirection);\n"
                "    vec3 LightDir = RawDirection / Dist;\n"
                "    vec3 c = CalcLightInternal(currentLight, LightDir, PxlWorldPos, PxlNormal, uv);\n"
                "    float attenuation = CalculateAttenuation(Dist, 1.0);\n"
                "    return c * attenuation;\n"
                "}\n";
            ShaderHelper::insertStringRightBeforeLineContent(code, point_light, "vec3 CalcSpotLight(");
        }
    }
#pragma endregion

#pragma region Lighting Internal Function
    if (ShaderHelper::sfind(code, "CalcLightInternal(")) {
        if (!ShaderHelper::sfind(code, "vec3 CalcLightInternal(")) {
            const string lighting_internal =
                "vec3 CalcLightInternal(in Light currentLight, vec3 LightDir, vec3 PxlWorldPos, vec3 PxlNormal, vec2 uv){//generated\n"
                "    float SpecularStrength     = texture2D(gMiscMap, uv).g;\n"
                "    vec3 MaterialAlbedoTexture = texture2D(gDiffuseMap, uv).rgb;\n"
                "    vec3 LightDiffuseColor     = currentLight.DataD.xyz;\n"
                "    vec3 LightSpecularColor    = currentLight.DataD.xyz * SpecularStrength;\n"
                "    vec3 TotalLight            = ConstantZeroVec3;\n"
                "    vec3 SpecularFactor        = ConstantZeroVec3;\n"
                "\n"
                "    float matIDandAO           = texture2D(gNormalMap, uv).b;\n"
                "    highp int matID            = int(floor(matIDandAO));\n"
                "    float ssaoValue            = 1.0 - texture2D(gSSAOMap, uv).a;\n"
                "    float ao                   = (fract(matIDandAO) + 0.0001) * ssaoValue;\n"//the 0.0001 makes up for the clamp in material class
                "    vec2 stuff                 = UnpackFloat16Into2Floats(texture2D(gNormalMap, uv).a);\n"
                "    float metalness            = stuff.x;\n"
                "    float smoothness           = stuff.y;\n"
                "    float materialAlpha        = materials[matID].g;\n"
                "\n"
                "    vec3 MaterialF0            = Unpack3FloatsInto1FloatUnsigned(materials[matID].r);\n"
                "    vec3 F0                    = mix(MaterialF0, MaterialAlbedoTexture, vec3(metalness));\n"
                "    vec3 Frensel               = F0;\n"
                "\n"
                "    float roughness            = 1.0 - smoothness;\n"
                "    float roughnessSquared     = roughness * roughness;\n"
                "\n"
                "    vec3 ViewDir               = normalize(CameraPosition - PxlWorldPos);\n"
                "    vec3 Half                  = normalize(LightDir + ViewDir);\n"
                "    float NdotL                = clamp(dot(PxlNormal, LightDir), 0.0, 1.0);\n"
                "    float NdotH                = clamp(dot(PxlNormal, Half), 0.0, 1.0);\n"
                "    float VdotN                = clamp(dot(ViewDir,PxlNormal), 0.0, 1.0);\n"
                "    float VdotH                = clamp(dot(ViewDir,Half), 0.0, 1.0);\n"
                "\n"
                "    float MaterialTypeDiffuse  = materials[matID].a;\n"
                "    float MaterialTypeSpecular = materials[matID].b;\n"
                "\n"
                "    if(MaterialTypeDiffuse == 2.0){\n"
                "        LightDiffuseColor *= DiffuseOrenNayar(ViewDir, LightDir, NdotL, VdotN, roughnessSquared);\n"
                "    }else if(MaterialTypeDiffuse == 3.0){\n"
                "        LightDiffuseColor *= DiffuseAshikhminShirley(smoothness, MaterialAlbedoTexture, NdotL, VdotN);\n"
                "    }else if(MaterialTypeDiffuse == 4.0){\n"//this is minneart
                "        LightDiffuseColor *= pow(VdotN * NdotL, smoothness);\n"
                "    }\n"
                "\n"
                "    if(MaterialTypeSpecular == 1.0){\n"
                "        SpecularFactor = SpecularBlinnPhong(smoothness, NdotH);\n"
                "    }else if(MaterialTypeSpecular == 2.0){\n"
                "        SpecularFactor = SpecularPhong(smoothness, LightDir, PxlNormal, ViewDir);\n"
                "    }else if(MaterialTypeSpecular == 3.0){\n"
                "        SpecularFactor = SpecularGGX(Frensel, LightDir, Half, roughnessSquared, NdotH, F0, NdotL);\n"
                "    }else if(MaterialTypeSpecular == 4.0){\n"
                "        SpecularFactor = SpecularCookTorrance(Frensel, F0, VdotH, NdotH, roughnessSquared, VdotN, roughness, NdotL);\n"
                "    }else if(MaterialTypeSpecular == 5.0){\n"
                "        SpecularFactor = SpecularGaussian(NdotH, smoothness);\n"
                "    }else if(MaterialTypeSpecular == 6.0){\n"
                "        SpecularFactor = vec3(BeckmannDist(NdotH, roughnessSquared));\n"
                "    }else if(MaterialTypeSpecular == 7.0){\n"
                "        SpecularFactor = SpecularAshikhminShirley(PxlNormal, Half, NdotH, LightDir, NdotL, VdotN);\n"
                "    }\n"
                "    LightDiffuseColor       *= currentLight.DataA.y;\n"
                "    LightSpecularColor      *= (SpecularFactor * currentLight.DataA.z);\n"
                "\n"
                "    vec3 componentDiffuse    = ConstantOneVec3 - Frensel;\n"
                "    componentDiffuse        *= 1.0 - metalness;\n"
                "\n"
                "    TotalLight               = (componentDiffuse * ao) * MaterialAlbedoTexture;\n"
                "    TotalLight              /= KPI;\n"
                "    TotalLight              += LightSpecularColor;\n"
                "    TotalLight              *= (LightDiffuseColor * NdotL);\n"
                "    TotalLight              *= materialAlpha;\n"
                "    return TotalLight;\n"
                "}\n";
            ShaderHelper::insertStringRightBeforeLineContent(code, lighting_internal, "vec3 CalcPointLight(");
        }
    }
#pragma endregion

#pragma region Projection Light Forward
    if (ShaderHelper::sfind(code, "CalcProjectionLightForward(")) {
        if (!ShaderHelper::sfind(code, "vec3 CalcProjectionLightForward(")) {
            const string rod_light =
                "vec3 CalcProjectionLightForward(in Light currentLight, vec3 A, vec3 B, vec3 PxlWorldPos, vec3 PxlNormal, in InData inData){//generated\n"
                /*
                //TODO: implement
                "    vec3 BMinusA = B - A;\n"
                "    vec3 CMinusA = PxlWorldPos - A;\n"
                "    float Dist = length(BMinusA);\n"
                "    vec3 _Normal = BMinusA / Dist;\n"
                "    float t = clamp(dot(CMinusA, _Normal / Dist), 0.0, 1.0);\n"
                "    vec3 LightPos = A + t * BMinusA;\n"
                "    vec3 c = CalcPointLightForward(currentLight, LightPos, PxlWorldPos, PxlNormal, inData);\n"
                "    return c;\n"
                */
                "    return vec3(0.0);\n"
                "}\n";
            ShaderHelper::insertStringRightBeforeLineContent(code, rod_light, "void main(");
        }
    }
#pragma endregion

#pragma region Rod Light Forward
    if (ShaderHelper::sfind(code, "CalcRodLightForward(")) {
        if (!ShaderHelper::sfind(code, "vec3 CalcRodLightForward(")) {
            const string rod_light =
                "vec3 CalcRodLightForward(in Light currentLight, vec3 A, vec3 B,vec3 PxlWorldPos, vec3 PxlNormal, in InData inData){//generated\n"
                "    vec3 BMinusA = B - A;\n"
                "    vec3 CMinusA = PxlWorldPos - A;\n"
                "    float Dist = length(BMinusA);\n"
                "    vec3 _Normal = BMinusA / Dist;\n"
                "    float t = clamp(dot(CMinusA, _Normal / Dist), 0.0, 1.0);\n"
                "    vec3 LightPos = A + t * BMinusA;\n"
                "    vec3 c = CalcPointLightForward(currentLight, LightPos, PxlWorldPos, PxlNormal, inData);\n"
                "    return c;\n"
                "}\n";
            ShaderHelper::insertStringRightBeforeLineContent(code, rod_light, "void main(");
        }
    }
#pragma endregion

#pragma region Spot Light Forward
    if (ShaderHelper::sfind(code, "CalcSpotLightForward(")) {
        if (!ShaderHelper::sfind(code, "vec3 CalcSpotLightForward(")) {
            const string spot_light =
                "vec3 CalcSpotLightForward(in Light currentLight, vec3 SpotLightDir, vec3 LightPos,vec3 PxlWorldPos, vec3 PxlNormal, in InData inData){//generated\n"
                "    vec3 LightDir = normalize(LightPos - PxlWorldPos);\n"
                "    vec3 c = CalcPointLightForward(currentLight, LightPos, PxlWorldPos, PxlNormal, inData);\n"
                "    float cosAngle = dot(LightDir, -SpotLightDir);\n"
                "    float spotEffect = smoothstep(currentLight.DataE.y, currentLight.DataE.x, cosAngle);\n"
                "    return c * spotEffect;\n"
                "}\n";
            ShaderHelper::insertStringRightBeforeLineContent(code, spot_light, "vec3 CalcRodLightForward(");
        }
    }
#pragma endregion

#pragma region Point Light Forward
    if (ShaderHelper::sfind(code, "CalcPointLightForward(")) {
        if (!ShaderHelper::sfind(code, "vec3 CalcPointLightForward(")) {
            const string point_light =
                "vec3 CalcPointLightForward(in Light currentLight, vec3 LightPos,vec3 PxlWorldPos, vec3 PxlNormal, in InData inData){//generated\n"
                "    vec3 RawDirection = LightPos - PxlWorldPos;\n"
                "    float Dist = length(RawDirection);\n"
                "    vec3 LightDir = RawDirection / Dist;\n"
                "    vec3 c = CalcLightInternalForward(currentLight, LightDir, PxlWorldPos, PxlNormal, inData);\n"
                "    float attenuation = CalculateAttenuationForward(currentLight,Dist,1.0);\n"
                "    return c * attenuation;\n"
                "}\n";
            ShaderHelper::insertStringRightBeforeLineContent(code, point_light, "vec3 CalcSpotLightForward(");
        }
    }
#pragma endregion

#pragma region Lighting Internal Forward Function
    if (ShaderHelper::sfind(code, "CalcLightInternalForward(")) {
        if (!ShaderHelper::sfind(code, "vec3 CalcLightInternalForward(")) {
            const string lighting_internal_forward =
                "vec3 CalcLightInternalForward(in Light currentLight, vec3 LightDir, vec3 PxlWorldPos, vec3 PxlNormal, in InData inData){//generated\n"
                "    float SpecularStrength     = inData.specular;\n"
                "    vec3 MaterialAlbedoTexture = inData.diffuse.rgb;\n"
                "    vec3 LightDiffuseColor     = currentLight.DataD.xyz;\n"
                "    vec3 LightSpecularColor    = currentLight.DataD.xyz * SpecularStrength;\n"
                "    vec3 TotalLight            = ConstantZeroVec3;\n"
                "    vec3 SpecularFactor        = ConstantZeroVec3;\n"
                "\n"
                //"  float ssaoValue     = 1.0 - texture2D(gSSAOMap, uv).a;\n"
                //"  float ao            = inData.ao * ssaoValue;\n"
                "    float ao            = inData.ao;\n"//the 0.0001 makes up for the clamp in material class
                "    float metalness     = inData.metalness;\n"
                "    float smoothness    = inData.smoothness;\n"
                "    float materialAlpha = MaterialBasePropertiesTwo.x;\n"
                "\n"
                "    vec3 MaterialF0 = inData.materialF0;\n"
                "    vec3 F0         = mix(MaterialF0, MaterialAlbedoTexture, vec3(metalness));\n"
                "    vec3 Frensel    = F0;\n"
                "\n"
                "    float roughness        = 1.0 - smoothness;\n"
                "    float roughnessSquared = roughness * roughness;\n"
                "\n"
                "    vec3 ViewDir = normalize(CameraPosition - PxlWorldPos);\n"
                "    vec3 Half    = normalize(LightDir + ViewDir);\n"
                "    float NdotL  = clamp(dot(PxlNormal, LightDir), 0.0, 1.0);\n"
                "    float NdotH  = clamp(dot(PxlNormal, Half), 0.0, 1.0);\n"
                "    float VdotN  = clamp(dot(ViewDir,PxlNormal), 0.0, 1.0);\n"
                "    float VdotH  = clamp(dot(ViewDir,Half), 0.0, 1.0);\n"
                "\n"
                "    float MaterialTypeDiffuse  = MaterialBasePropertiesTwo.y;\n"
                "    float MaterialTypeSpecular = MaterialBasePropertiesTwo.z;\n"
                "\n"
                "    if(MaterialTypeDiffuse == 2.0){\n"
                "        LightDiffuseColor *= DiffuseOrenNayar(ViewDir, LightDir, NdotL, VdotN, roughnessSquared);\n"
                "    }else if(MaterialTypeDiffuse == 3.0){\n"
                "        LightDiffuseColor *= DiffuseAshikhminShirley(smoothness, MaterialAlbedoTexture, NdotL, VdotN);\n"
                "    }else if(MaterialTypeDiffuse == 4.0){\n"//this is minneart
                "        LightDiffuseColor *= pow(VdotN * NdotL, smoothness);\n"
                "    }\n"
                "\n"
                "    if(MaterialTypeSpecular == 1.0){\n"
                "        SpecularFactor = SpecularBlinnPhong(smoothness, NdotH);\n"
                "    }else if(MaterialTypeSpecular == 2.0){\n"
                "        SpecularFactor = SpecularPhong(smoothness, LightDir, PxlNormal, ViewDir);\n"
                "    }else if(MaterialTypeSpecular == 3.0){\n"
                "        SpecularFactor = SpecularGGX(Frensel, LightDir, Half, roughnessSquared, NdotH, F0, NdotL);\n"
                "    }else if(MaterialTypeSpecular == 4.0){\n"
                "        SpecularFactor = SpecularCookTorrance(Frensel, F0, VdotH, NdotH, roughnessSquared, VdotN, roughness, NdotL);\n"
                "    }else if(MaterialTypeSpecular == 5.0){\n"
                "        SpecularFactor = SpecularGaussian(NdotH, smoothness);\n"
                "    }else if(MaterialTypeSpecular == 6.0){\n"
                "        SpecularFactor = vec3(BeckmannDist(NdotH, roughnessSquared));\n"
                "    }else if(MaterialTypeSpecular == 7.0){\n"
                "        SpecularFactor = SpecularAshikhminShirley(PxlNormal, Half, NdotH, LightDir, NdotL, VdotN);\n"
                "    }\n"
                "    LightDiffuseColor *= currentLight.DataA.y;\n"
                "    LightSpecularColor *= (SpecularFactor * currentLight.DataA.z);\n"
                "\n"
                "    vec3 componentDiffuse = ConstantOneVec3 - Frensel;\n"
                "    componentDiffuse *= 1.0 - metalness;\n"
                "\n"
                "    TotalLight = (componentDiffuse * ao) * MaterialAlbedoTexture;\n"
                "    TotalLight /= KPI;\n"
                "    TotalLight += LightSpecularColor;\n"
                //"    NdotL = pow(NdotL,  2.2);\n" //gamma test. do not use this
                "    TotalLight *= (LightDiffuseColor * NdotL);\n"
                "    TotalLight *= materialAlpha;\n"
                "    return TotalLight;\n"
                "}\n";
            ShaderHelper::insertStringRightBeforeLineContent(code, lighting_internal_forward, "vec3 CalcPointLightForward(");
        }
    }
#pragma endregion



#pragma region Diffuse oryen nayar
    if (ShaderHelper::sfind(code, "DiffuseOrenNayar(")) {
        if (!ShaderHelper::sfind(code, "float DiffuseOrenNayar(")) {
            const string diffuse_oren_nayar =
                "float DiffuseOrenNayar(vec3 _ViewDir, vec3 _LightDir, float _NdotL, float _VdotN, float _roughnessSquared){//generated\n"
                "    float A = 1.0 - 0.5 * _roughnessSquared / (_roughnessSquared + 0.33);\n"
                "    float B = 0.45 * _roughnessSquared / (_roughnessSquared + 0.09);\n"
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
                "}\n";
            ShaderHelper::insertStringAtLine(code, diffuse_ash_shir, 1);
        }
    }
#pragma endregion

#pragma region Specular Blinn Phong
    if (ShaderHelper::sfind(code, "SpecularBlinnPhong(")) {
        if (!ShaderHelper::sfind(code, "vec3 SpecularBlinnPhong(")) {
            const string specular_blinn_phong =
                "vec3 SpecularBlinnPhong(float _smoothness,float _NdotH){//generated\n"
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
                "vec3 SpecularPhong(float _smoothness,vec3 _LightDir,vec3 _PxlNormal,vec3 _ViewDir){//generated\n"
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
                "vec3 SpecularGaussian(float _NdotH,float _smoothness){//generated\n"
                "    float b = acos(_NdotH);\n" //this might also be cos. find out
                "    float fin = b / _smoothness;\n"
                "    return vec3(exp(-fin*fin));\n"
                "}\n";
            ShaderHelper::insertStringAtLine(code, specular_gaussian, 1);
        }
    }
#pragma endregion

#pragma region Specular GGX
    if (ShaderHelper::sfind(code, "SpecularGGX(")) {
        if (!ShaderHelper::sfind(code, "vec3 SpecularGGX(")) {
            const string specular_ggx =
                "vec3 SpecularGGX(inout vec3 _Frensel,vec3 _LightDir,vec3 _Half,float _roughnessSquared,float _NdotH,vec3 _F0,float _NdotL){//generated\n"
                "    float LdotH = max(0.0, dot(_LightDir, _Half));\n"
                "    float alphaSqr = _roughnessSquared * _roughnessSquared;\n"
                "    float denom = _NdotH * _NdotH * (alphaSqr - 1.0) + 1.0;\n"
                "    float NDF = alphaSqr / (KPI * denom * denom);\n"
                "    _Frensel = SchlickFrensel(LdotH, _F0);\n"
                "    float k = _roughnessSquared * 0.5;\n"
                "    float k2 = k * k;\n"
                "    float invk2 = 1.0 - k2;\n"
                "    float G = 1.0 / (LdotH * LdotH * invk2 + k2);\n"
                "    return _NdotL * NDF * _Frensel * G;\n"
                "}\n";
            ShaderHelper::insertStringAtLine(code, specular_ggx, 1);
        }
    }
#pragma endregion

#pragma region Specular Cook Torrance
    if (ShaderHelper::sfind(code, "SpecularCookTorrance(")) {
        if (!ShaderHelper::sfind(code, "vec3 SpecularCookTorrance(")) {
            const string specular_cook_torrance =
                "vec3 SpecularCookTorrance(inout vec3 _Frensel,vec3 _F0,float _VdotH,float _NdotH,float _roughnessSquared,float _VdotN,float _roughness,float _NdotL){//generated\n"
                "    _Frensel = SchlickFrensel(_VdotH,_F0);\n"
                "    float NDF = GGXDist(_NdotH * _NdotH, _roughnessSquared * _roughnessSquared);\n"
                "    float roughnessPlusOne = _roughness + 1.0;\n"
                "    float a = roughnessPlusOne * roughnessPlusOne;\n"
                "    float G = GeometrySchlickGGX(_VdotN,a) * GeometrySchlickGGX(_NdotL,a);\n"
                "    vec3 Top = NDF * _Frensel * G;\n"
                "    float Bottom = max(4.0 * _VdotN * _NdotL,0.0);\n"
                "    return Top / (Bottom + 0.001);\n"
                "}\n";
            ShaderHelper::insertStringAtLine(code, specular_cook_torrance, 1);
        }
    }
#pragma endregion

#pragma region Specular Ashikhmin Shirley
    if (ShaderHelper::sfind(code, "SpecularAshikhminShirley(")) {
        if (!ShaderHelper::sfind(code, "vec3 SpecularAshikhminShirley(")) {
            const string specular_ash_shirley =
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
                "}\n";
            ShaderHelper::insertStringAtLine(code, specular_ash_shirley, 1);
        }
    }
#pragma endregion

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

#pragma region Attenuation Function
    if (ShaderHelper::sfind(code, "CalculateAttenuation(")) {
        if (!ShaderHelper::sfind(code, "float CalculateAttenuation(")) {
            const string attenuation_function =
                "float CalculateAttenuation(float Dist, float radius){//generated\n"
                "   float attenuation = 0.0;\n"
                "   if(light.DataE.z == 0.0){\n"       //constant
                "       attenuation = 1.0 / max(1.0 , light.DataB.z);\n"
                "   }else if(light.DataE.z == 1.0){\n" //distance
                "       attenuation = 1.0 / max(1.0 , Dist);\n"
                "   }else if(light.DataE.z == 2.0){\n" //distance squared
                "       attenuation = 1.0 / max(1.0 , Dist * Dist);\n"
                "   }else if(light.DataE.z == 3.0){\n" //constant linear exponent
                "       attenuation = 1.0 / max(1.0 , light.DataB.z + (light.DataB.w * Dist) + (light.DataC.x * Dist * Dist));\n"
                "   }else if(light.DataE.z == 4.0){\n" //distance radius squared
                "       attenuation = 1.0 / max(1.0 ,pow((Dist / radius) + 1.0,2.0));\n"
                "   }\n"
                "   return attenuation;\n"
                "}\n";
            ShaderHelper::insertStringRightBeforeLineContent(code, attenuation_function, "vec3 CalcLightInternal(");
        }
    }
#pragma endregion

#pragma region Attenuation Forward Function
    if (ShaderHelper::sfind(code, "CalculateAttenuationForward(")) {
        if (!ShaderHelper::sfind(code, "float CalculateAttenuationForward(")) {
            const string attenuation_function_forward =
                "float CalculateAttenuationForward(in Light currentLight, float Dist, float radius){//generated\n"
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
                "       attenuation = 1.0 / max(1.0 ,pow((Dist / radius) + 1.0,2.0));\n"
                "   }\n"
                "   return attenuation;\n"
                "}\n";
            ShaderHelper::insertStringRightBeforeLineContent(code, attenuation_function_forward, "vec3 CalcLightInternalForward(");
        }
    }
#pragma endregion

}