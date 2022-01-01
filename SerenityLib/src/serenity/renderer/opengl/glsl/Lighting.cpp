
#include <serenity/renderer/opengl/glsl/Lighting.h>

#include <serenity/renderer/opengl/Extensions.h>
#include <serenity/resources/shader/ShaderHelper.h>
#include <serenity/resources/material/MaterialEnums.h>

#include <boost/algorithm/string/replace.hpp>

void Engine::priv::opengl::glsl::Lighting::convert(std::string& code, uint32_t versionNumber, ShaderType shaderType) {

#pragma region Projection Light
    if (ShaderHelper::lacksDefinition(code, "CalcProjectionLight(", "vec3 CalcProjectionLight(")) {
        ShaderHelper::insertStringRightBeforeLineContent(code, 
            "vec3 CalcProjectionLight(in Light inLight, vec3 A, vec3 B,vec3 PxlWorldPos, vec3 PxlViewPos, vec3 PxlNormal, float Specular, vec3 Albedo, float SSAO, vec2 MetalSmooth, float MatAlpha, vec3 MatF0, float MatTypeDiffuse, float MatTypeSpecular, float AO){//generated\n"
            /*
            //TODO: implement
            "    vec3 BMinusA = B - A;\n"
            "    vec3 CMinusA = PxlWorldPos - A;\n"
            "    float Dist = length(BMinusA);\n"
            "    vec3 _Normal = BMinusA / Dist;\n"
            "    float t = clamp(dot(CMinusA, _Normal / Dist), 0.0, 1.0);\n"
            "    vec3 LightPos = A + t * BMinusA;\n"
            "    vec3 c = CalcPointLight(inLight, LightPos, PxlWorldPos, PxlViewPos, PxlNormal, Specular, Albedo, SSAO, MetalSmooth, MatAlpha, MatF0, MatTypeDiffuse, MatTypeSpecular, AO);\n"
            "    return c;\n"
            */
            "    return vec3(0.0, 1.0, 0.0);\n"
            "}\n"
        , "void main(");
    }
#pragma endregion

#pragma region Rod Light
    if (ShaderHelper::lacksDefinition(code, "CalcRodLight(", "vec3 CalcRodLight(")) {
        ShaderHelper::insertStringRightBeforeLineContent(code, R"(
vec3 CalcRodLight(in Light inLight, vec3 A, vec3 B, vec3 PxlWorldPos, vec3 PxlViewPos, vec3 PxlNormal, float Specular, vec3 Albedo, float SSAO, vec2 MetalSmooth, float MatAlpha, vec3 MatF0, float MatTypeDiffuse, float MatTypeSpecular, float AO){ //generated
    vec3 BMinusA  = B - A;
    vec3 CMinusA  = PxlWorldPos - A;
    float Dist    = length(BMinusA);
    vec3 _Normal  = BMinusA / Dist;
    float t       = clamp(dot(CMinusA, _Normal / Dist), 0.0, 1.0);
    vec3 LightPos = A + t * BMinusA;
    vec3 c        = CalcPointLight(inLight, LightPos, PxlWorldPos, PxlViewPos, PxlNormal, Specular, Albedo, SSAO, MetalSmooth, MatAlpha, MatF0, MatTypeDiffuse, MatTypeSpecular, AO);
    return c;
}
)", "void main(");
    }
#pragma endregion

#pragma region Spot Light
    if (ShaderHelper::lacksDefinition(code, "CalcSpotLight(", "vec3 CalcSpotLight(")) {
        ShaderHelper::insertStringRightBeforeLineContent(code, R"(
vec3 CalcSpotLight(in Light inLight, vec3 SpotLightDir, vec3 LightPos,vec3 PxlWorldPos, vec3 PxlViewPos, vec3 PxlNormal, float Specular, vec3 Albedo, float SSAO, vec2 MetalSmooth, float MatAlpha, vec3 MatF0, float MatTypeDiffuse, float MatTypeSpecular, float AO){ //generated
    vec3 LightDir    = normalize(LightPos - PxlWorldPos);
    vec3 c           = CalcPointLight(inLight, LightPos, PxlWorldPos, PxlViewPos, PxlNormal, Specular, Albedo, SSAO, MetalSmooth, MatAlpha, MatF0, MatTypeDiffuse, MatTypeSpecular, AO);
    float theta      = dot(LightDir, -SpotLightDir);
    float epsilon    = inLight.DataE.x - inLight.DataE.y;
    float spotEffect = smoothstep(0.0, 1.0, (theta - inLight.DataE.y) / epsilon);
    return c * spotEffect;
}
)", "vec3 CalcRodLight(");
    }
#pragma endregion

#pragma region Point Light
    if (ShaderHelper::lacksDefinition(code, "CalcPointLight(", "vec3 CalcPointLight(")) {
        ShaderHelper::insertStringRightBeforeLineContent(code, R"(
vec3 CalcPointLight(in Light inLight, vec3 LightPos,vec3 PxlWorldPos, vec3 PxlViewPos, vec3 PxlNormal, float Specular, vec3 Albedo, float SSAO, vec2 MetalSmooth, float MatAlpha, vec3 MatF0, float MatTypeDiffuse, float MatTypeSpecular, float AO){ //generated
    vec3 RawDirection = LightPos - PxlWorldPos;
    float Dist        = length(RawDirection);
    vec3 LightDir     = RawDirection / Dist;
    vec3 c            = CalcLightInternal(inLight, LightDir, PxlWorldPos, PxlViewPos, PxlNormal, Specular, Albedo, SSAO, MetalSmooth, MatAlpha, MatF0, MatTypeDiffuse, MatTypeSpecular, AO);
    float attenuation = CalculateAttenuation(inLight, Dist, 1.0);
    return c * attenuation;
}
)", "vec3 CalcSpotLight(");
    }
#pragma endregion

#pragma region Lighting Internal Function
    if (ShaderHelper::lacksDefinition(code, "CalcLightInternal(", "vec3 CalcLightInternal(")) {
        ShaderHelper::insertStringRightBeforeLineContent(code, R"(
vec3 CalcLightInternal(in Light currentLight, vec3 LightDir, vec3 PxlWorldPos, vec3 PxlViewPos, vec3 PxlNormal, float Specular, vec3 Albedo, float SSAO, vec2 MetalSmooth, float MatAlpha, vec3 MatF0, float MatTypeDiffuse, float MatTypeSpecular, float AO){
    vec3 LightDiffuseColor  = currentLight.DataD.xyz;
    vec3 LightSpecularColor = currentLight.DataD.xyz * Specular;
    vec3 SpecularFactor     = ConstantZeroVec3;

    vec2 uvs = gl_FragCoord.xy / ScreenInfo.zw;
    vec4 ShadowPxlWorldPos = vec4(PxlWorldPos + CamRealPosition, 1.0);

    float shadow = ShadowCalculationLightingShader(PxlViewPos, LightDir, PxlNormal, ShadowPxlWorldPos);

    float ao                = AO * SSAO;
    float metalness         = MetalSmooth.x;
    float smoothness        = MetalSmooth.y;
    vec3 F0                 = mix(MatF0, Albedo, vec3(metalness));
    vec3 Frensel            = F0;

    float roughness         = 1.0 - smoothness;
    float roughnessSquared  = roughness * roughness;

    vec3 ViewDir            = normalize(CameraPosition - PxlWorldPos);
    vec3 Half               = normalize(LightDir + ViewDir);
    float NdotL             = max(dot(PxlNormal, LightDir), 0.0);
    float NdotH             = max(dot(PxlNormal, Half), 0.0);
    float VdotN             = max(dot(ViewDir, PxlNormal), 0.0);
    float VdotH             = max(dot(ViewDir, Half), 0.0);

    if(MatTypeDiffuse == 2.0){
        LightDiffuseColor *= DiffuseOrenNayar(ViewDir, LightDir, NdotL, VdotN, roughnessSquared);
    }else if(MatTypeDiffuse == 3.0){
        LightDiffuseColor *= DiffuseAshikhminShirley(smoothness, Albedo, NdotL, VdotN);
    }else if(MatTypeDiffuse == 4.0){
        LightDiffuseColor *= pow(VdotN * NdotL, smoothness);
    }
    if(MatTypeSpecular == 1.0){
        SpecularFactor = SpecularBlinnPhong(smoothness, NdotH);
    }else if(MatTypeSpecular == 2.0){
        SpecularFactor = SpecularPhong(smoothness, LightDir, PxlNormal, ViewDir);
    }else if(MatTypeSpecular == 3.0){
        SpecularFactor = SpecularGGX(Frensel, LightDir, Half, roughnessSquared, NdotH, F0, NdotL);
    }else if(MatTypeSpecular == 4.0){
        SpecularFactor = SpecularCookTorrance(Frensel, F0, VdotH, NdotH, roughnessSquared, VdotN, roughness, NdotL);
    }else if(MatTypeSpecular == 5.0){
        SpecularFactor = SpecularGaussian(NdotH, smoothness);
    }else if(MatTypeSpecular == 6.0){
        SpecularFactor = vec3(BeckmannDist(NdotH, roughnessSquared));
    }else if(MatTypeSpecular == 7.0){
        SpecularFactor = SpecularAshikhminShirley(PxlNormal, Half, NdotH, LightDir, NdotL, VdotN);
    }
    LightDiffuseColor     *= currentLight.DataA.y;
    LightSpecularColor    *= (SpecularFactor * currentLight.DataA.z);

    vec3 componentDiffuse  = ConstantOneVec3 - Frensel;
    componentDiffuse      *= 1.0 - metalness;

    vec3 TotalLight        = (componentDiffuse * min(ao, shadow)) * Albedo;
    TotalLight            /= KPI;
    TotalLight            += LightSpecularColor * shadow;
    TotalLight            *= (LightDiffuseColor * NdotL);
    TotalLight            *= MatAlpha;
    return TotalLight;
}
)", "vec3 CalcPointLight(");
    }
#pragma endregion

#pragma region GI Lighting Internal
    if (ShaderHelper::lacksDefinition(code, "CalcGILight(", "vec4 CalcGILight(")) {
        ShaderHelper::insertStringRightBeforeMainFunc(code, R"(
vec4 CalcGILight(float SSAO, vec3 Normals, vec3 Albedo, vec3 WorldPosition, float AO, float Metal, float Smooth, float Glow, vec3 MatF0, float MatAlpha, vec3 GIContribution){
    vec3 ViewDir          = normalize(CameraPosition - WorldPosition);
    vec3 R                = reflect(-ViewDir, Normals);
    float VdotN           = max(dot(ViewDir, Normals), 0.0);
    float ao              = AO * SSAO;
    vec3 Frensel          = mix(MatF0, Albedo, vec3(Metal));
    float roughness       = 1.0 - Smooth;
    vec3 irradianceColor  = textureCube(irradianceMap, Normals).rgb;
    vec3 kS               = SchlickFrenselRoughness(VdotN, Frensel, roughness);
    vec3 kD               = ConstantOneVec3 - kS;
    kD                   *= 1.0 - Metal;
    vec3 GIDiffuse        = irradianceColor * Albedo * kD * GIContribution.x;

    vec3 prefilteredColor = textureCubeLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf             = texture2D(brdfLUT, vec2(VdotN, roughness)).rg;
    vec3 GISpecular       = prefilteredColor * (kS * brdf.x + brdf.y) * GIContribution.y;

    vec3 TotalIrradiance  = (GIDiffuse + GISpecular) * ao;

    vec4 result = (vec4(TotalIrradiance, 1.0) * vec4(vec3(GIContribution.z), 1.0)) * MatAlpha;
    result.rgb = pow(result.rgb, vec3(1.0 / RendererInfo1.y));
    return result;
}
)");
    }
#pragma endregion

#pragma region Diffuse oryen nayar
    if (ShaderHelper::lacksDefinition(code, "DiffuseOrenNayar(", "float DiffuseOrenNayar(")) {
        ShaderHelper::insertStringAtLine(code, R"(
float DiffuseOrenNayar(vec3 _ViewDir, vec3 _LightDir, float _NdotL, float _VdotN, float _roughnessSquared){
    float A = 1.0 - 0.5 * _roughnessSquared / (_roughnessSquared + 0.33);
    float B = 0.45 * _roughnessSquared / (_roughnessSquared + 0.09);
    float cosAzimuthSinPolarTanPolar = (dot(_LightDir, _ViewDir) - _VdotN * _NdotL) / max(_VdotN, _NdotL);
    return (A + B * max(0.0, cosAzimuthSinPolarTanPolar));
}
)", 1);
    }
#pragma endregion

#pragma region Diffuse Ashikhmin Shirley
    //float s = clamp(_smoothness, 0.01, 0.76);\n" //this lighting model has to have some form of roughness in it to look good. cant be 1.0
    //ret *= KPI;\n" //i know this isnt proper, but the diffuse component is *way* too dark otherwise...
    if (ShaderHelper::lacksDefinition(code, "DiffuseAshikhminShirley(", "vec3 DiffuseAshikhminShirley(")) {
        ShaderHelper::insertStringAtLine(code, R"(
vec3 DiffuseAshikhminShirley(float _smoothness, vec3 _MaterialAlbedoTexture, float _NdotL, float _VdotN){
    vec3 ret;
    float s = clamp(_smoothness, 0.01, 0.76);
    vec3 A = (28.0 * _MaterialAlbedoTexture) / vec3(23.0 * KPI);
    float B = 1.0 - s;
    float C = (1.0 - pow((1.0 - (_NdotL * 0.5)), 5.0));
    float D = (1.0 - pow((1.0 - (_VdotN * 0.5)), 5.0));
    ret = A * B * C * D;
    ret *= KPI;
    return ret;
}
)", 1);
    }
#pragma endregion

#pragma region Specular Blinn Phong
    if (ShaderHelper::lacksDefinition(code, "SpecularBlinnPhong(", "vec3 SpecularBlinnPhong(")) {
        ShaderHelper::insertStringAtLine(code, R"(
vec3 SpecularBlinnPhong(float smoothness_, float NdotH_){
    float gloss = exp2(10.0 * smoothness_ + 1.0);
    float kS = (8.0 + gloss ) / (8.0 * KPI);
    return vec3(kS * pow(NdotH_, gloss));
}
)", 1);
    }
#pragma endregion

#pragma region Specular Phong
    if (ShaderHelper::lacksDefinition(code, "SpecularPhong(", "vec3 SpecularPhong(")) {
        ShaderHelper::insertStringAtLine(code, R"(
vec3 SpecularPhong(float _smoothness,vec3 _LightDir,vec3 _PxlNormal,vec3 _ViewDir){
    float gloss = exp2(10.0 * _smoothness + 1.0);
    float kS = (2.0 + gloss ) / (2.0 * KPI);
    vec3 Reflect = reflect(-_LightDir, _PxlNormal);
    float VdotR = max(0.0, dot(_ViewDir,Reflect));
    return vec3(kS * pow(VdotR, gloss));
}
)", 1);
    }
#pragma endregion

#pragma region Specular Gaussian
    if (ShaderHelper::lacksDefinition(code, "SpecularGaussian(", "vec3 SpecularGaussian(")) {
        ShaderHelper::insertStringAtLine(code, R"(
vec3 SpecularGaussian(float _NdotH,float smoothness_){
    float b = acos(_NdotH);
    float fin = b / smoothness_;
    return vec3(exp(-fin * fin));
}
)", 1);
    }
#pragma endregion

#pragma region Specular GGX
    if (ShaderHelper::lacksDefinition(code, "SpecularGGX(", "vec3 SpecularGGX(")) {
        ShaderHelper::insertStringAtLine(code, R"(
vec3 SpecularGGX(inout vec3 _Frensel,vec3 _LightDir,vec3 _Half,float _roughnessSquared,float _NdotH,vec3 F0_,float _NdotL){
    float LdotH = max(0.0, dot(_LightDir, _Half));
    float alphaSqr = _roughnessSquared * _roughnessSquared;
    float denom = _NdotH * _NdotH * (alphaSqr - 1.0) + 1.0;
    float NDF = alphaSqr / (KPI * denom * denom);
    _Frensel = SchlickFrensel(LdotH, F0_);
    float k = _roughnessSquared * 0.5;
    float k2 = k * k;
    float invk2 = 1.0 - k2;
    float G = 1.0 / (LdotH * LdotH * invk2 + k2);
    return _NdotL * NDF * _Frensel * G;
}
)", 1);
    }
#pragma endregion

#pragma region Specular Cook Torrance
    if (ShaderHelper::lacksDefinition(code, "SpecularCookTorrance(", "vec3 SpecularCookTorrance(")) {
        ShaderHelper::insertStringAtLine(code, R"(
vec3 SpecularCookTorrance(inout vec3 _Frensel,vec3 _F0,float _VdotH,float _NdotH,float _roughnessSquared,float _VdotN,float _roughness,float _NdotL){
    _Frensel = SchlickFrensel(_VdotH,_F0);
    float NDF = GGXDist(_NdotH * _NdotH, _roughnessSquared * _roughnessSquared);
    float roughnessPlusOne = _roughness + 1.0;
    float a = roughnessPlusOne * roughnessPlusOne;
    float G = GeometrySchlickGGX(_VdotN,a) * GeometrySchlickGGX(_NdotL,a);
    vec3 Top = NDF * _Frensel * G;
    float Bottom = max(4.0 * _VdotN * _NdotL,0.0);
    return Top / (Bottom + 0.001);
}
)", 1);
    }
#pragma endregion

#pragma region Specular Ashikhmin Shirley
    if (ShaderHelper::lacksDefinition(code, "SpecularAshikhminShirley(", "vec3 SpecularAshikhminShirley(")) {
        ShaderHelper::insertStringAtLine(code, R"(
vec3 SpecularAshikhminShirley(vec3 _PxlNormal,vec3 _Half,float _NdotH,vec3 _LightDir,float _NdotL,float _VdotN){
    const float Nu = 1000.0;
    const float Nv = 1000.0;
    vec3 epsilon = vec3(1.0,0.0,0.0);
    vec3 tangent = normalize(cross(_PxlNormal,epsilon));
    vec3 bitangent = normalize(cross(_PxlNormal,tangent));
    float hdotT = dot(_Half,tangent);
    float hDotB = dot(_Half,bitangent);
    float A = sqrt( (Nu + 1.0) * (Nv + 1.0) );
    float B = pow(_NdotH,((Nu * hdotT * hdotT + Nv * hDotB * hDotB) / (1.0 - (_NdotH * _NdotH)) ));
    float HdotL = max(0.0, dot(_Half, _LightDir));
    float C = 8.0 * KPI * HdotL * max(_NdotL,_VdotN);
    return vec3((A * B) / C);
}
)", 1);
    }
#pragma endregion

#pragma region Beckmann Distribution
    if (ShaderHelper::lacksDefinition(code, "BeckmannDist(", "float BeckmannDist(")) {
        ShaderHelper::insertStringAtLine(code, R"(
float BeckmannDist(float cos2a, float _alpha){
    float b = (1.0 - cos2a) / (cos2a * _alpha);
    return (exp(-b)) / (KPI * _alpha * cos2a * cos2a);
}
)", 1);
    }
#pragma endregion

#pragma region GGX Distribution
    if (ShaderHelper::lacksDefinition(code, "GGXDist(", "float GGXDist(")) {
        ShaderHelper::insertStringAtLine(code, R"(
float GGXDist(float NdotHSquared, float alphaSquared){ //generated
    float denom = (NdotHSquared * (alphaSquared - 1.0) + 1.0);
    denom = KPI * denom * denom;
    return alphaSquared / denom;
}
)", 1);
    }
#pragma endregion

#pragma region Geometry Schlick GGX Distribution
    if (ShaderHelper::lacksDefinition(code, "GeometrySchlickGGX(", "float GeometrySchlickGGX(")) {
        ShaderHelper::insertStringAtLine(code, R"(
float GeometrySchlickGGX(float NdotV, float a){ //generated
    float k = a * 0.125;
    float denom = NdotV * (1.0 - k) + k;
    return NdotV / denom;
}
)", 1);
    }
#pragma endregion

#pragma region Schlick Frensel
    if (ShaderHelper::lacksDefinition(code, "SchlickFrensel(", "vec3 SchlickFrensel(")) {
        ShaderHelper::insertStringAtLine(code, R"(
vec3 SchlickFrensel(float inTheta, vec3 inF0){ //generated
    return inF0 + (ConstantOneVec3 - inF0) * pow(1.0 - inTheta, 5.0);
}
)", 1);
    }
#pragma endregion

#pragma region Attenuation Function
/*
if(currentLight.DataE.z == 0.0){       //constant
}else if(currentLight.DataE.z == 1.0){ //distance
}else if(currentLight.DataE.z == 2.0){ //distance squared
}else if(currentLight.DataE.z == 3.0){ //constant linear exponent
}else if(currentLight.DataE.z == 4.0){ //distance radius squared
*/
    if (ShaderHelper::lacksDefinition(code, "CalculateAttenuation(", "float CalculateAttenuation(")) {
        ShaderHelper::insertStringRightBeforeLineContent(code, R"(
float CalculateAttenuation(in Light inLight, float Dist, float radius) { //generated
    float attenuation = 0.0;
    if (inLight.DataE.z == 0.0) {
        attenuation = 1.0 / max(1.0, inLight.DataB.z);
    } else if (inLight.DataE.z == 1.0) {
        attenuation = 1.0 / max(1.0, Dist);
    } else if (inLight.DataE.z == 2.0) {
        attenuation = 1.0 / max(1.0, Dist * Dist);
    } else if (inLight.DataE.z == 3.0) {
        attenuation = 1.0 / max(1.0, inLight.DataB.z + (inLight.DataB.w * Dist) + (inLight.DataC.x * Dist * Dist));
    } else if (inLight.DataE.z == 4.0) {
        attenuation = 1.0 / max(1.0, pow((Dist / radius) + 1.0, 2.0));
    }
    return attenuation;
}
)", "vec3 CalcLightInternal(");
    }
#pragma endregion




#pragma region Projection Light Basic
    if (ShaderHelper::lacksDefinition(code, "CalcProjectionLightBasic(", "vec3 CalcProjectionLightBasic(")) {
        ShaderHelper::insertStringRightBeforeLineContent(code,
"vec3 CalcProjectionLightBasic(in Light inLight, vec3 A, vec3 B, vec3 PxlWorldPos, vec3 PxlViewPos, vec3 PxlNormal, float Specular, vec3 Albedo, float SSAO, float MatAlpha, float AO){\n"
/*
//TODO: implement
"    vec3 BMinusA = B - A;\n"
"    vec3 CMinusA = PxlWorldPos - A;\n"
"    float Dist = length(BMinusA);\n"
"    vec3 _Normal = BMinusA / Dist;\n"
"    float t = clamp(dot(CMinusA, _Normal / Dist), 0.0, 1.0);\n"
"    vec3 LightPos = A + t * BMinusA;\n"
"    vec3 c = CalcPointLightBasic(inLight, LightPos, PxlWorldPos, PxlViewPos, PxlNormal, Specular, Albedo, SSAO, MatAlpha, AO);\n"
"    return c;\n"
*/
"    return vec3(0.0, 1.0, 0.0);\n"
"}\n"
, "void main(");
    }
#pragma endregion

#pragma region Rod Light Basic
    if (ShaderHelper::lacksDefinition(code, "CalcRodLightBasic(", "vec3 CalcRodLightBasic(")) {
        ShaderHelper::insertStringRightBeforeLineContent(code, R"(
vec3 CalcRodLightBasic(in Light inLight, vec3 A, vec3 B, vec3 PxlWorldPos, vec3 PxlViewPos, vec3 PxlNormal, float Specular, vec3 Albedo, float SSAO, float MatAlpha, float AO){
    vec3 BMinusA = B - A;
    vec3 CMinusA = PxlWorldPos - A;
    float Dist = length(BMinusA);
    vec3 _Normal = BMinusA / Dist;
    float t = clamp(dot(CMinusA, _Normal / Dist), 0.0, 1.0);
    vec3 LightPos = A + t * BMinusA;
    vec3 c = CalcPointLightBasic(inLight, LightPos, PxlWorldPos, PxlViewPos, PxlNormal, Specular, Albedo, SSAO, MatAlpha, AO);
    return c;
}
)", "void main(");
    }
#pragma endregion

#pragma region Spot Light Basic
    if (ShaderHelper::lacksDefinition(code, "CalcSpotLightBasic(", "vec3 CalcSpotLightBasic(")) {
        ShaderHelper::insertStringRightBeforeLineContent(code, R"(
vec3 CalcSpotLightBasic(in Light inLight, vec3 SpotLightDir, vec3 LightPos, vec3 PxlWorldPos, vec3 PxlViewPos, vec3 PxlNormal, float Specular, vec3 Albedo, float SSAO, float MatAlpha, float AO){
    vec3 LightDir    = normalize(LightPos - PxlWorldPos);
    vec3 c           = CalcPointLightBasic(inLight, LightPos, PxlWorldPos, PxlViewPos, PxlNormal, Specular, Albedo, SSAO, MatAlpha, AO);
    float theta      = dot(LightDir, -SpotLightDir);
    float epsilon    = inLight.DataE.x - inLight.DataE.y;
    float spotEffect = smoothstep(0.0, 1.0, (theta - inLight.DataE.y) / epsilon);
    return c * spotEffect;
}
)", "vec3 CalcRodLightBasic(");
    }
#pragma endregion

#pragma region Point Light Basic
    if (ShaderHelper::lacksDefinition(code, "CalcPointLightBasic(", "vec3 CalcPointLightBasic(")) {
        ShaderHelper::insertStringRightBeforeLineContent(code, R"(
vec3 CalcPointLightBasic(in Light inLight, vec3 LightPos, vec3 PxlWorldPos, vec3 PxlViewPos, vec3 PxlNormal, float Specular, vec3 Albedo, float SSAO, float MatAlpha, float AO){
    vec3 RawDirection = LightPos - PxlWorldPos;
    float Dist        = length(RawDirection);
    vec3 LightDir     = RawDirection / Dist;
    return CalcLightInternalBasic(inLight, LightDir, PxlWorldPos, PxlViewPos, PxlNormal, Specular, Albedo, SSAO, MatAlpha, AO) * CalculateAttenuationBasic(inLight, Dist, 1.0);
}
)", "vec3 CalcSpotLightBasic(");
    }
#pragma endregion

#pragma region Lighting Internal Function Basic
    //TODO: modify the 64 constant to use either metalness or roughness (or both?). the higher this constant, the smaller the brightness spot is. roughness would probably be best then
    if (ShaderHelper::lacksDefinition(code, "CalcLightInternalBasic(", "vec3 CalcLightInternalBasic(")) {
        ShaderHelper::insertStringRightBeforeLineContent(code, R"(
vec3 CalcLightInternalBasic(in Light currentLight, vec3 LightDir, vec3 PxlWorldPos, vec3 PxlViewPos, vec3 PxlNormal, float Specular, vec3 Albedo, float SSAO, float MatAlpha, float AO){
    vec3 LightDiffuseColor  = currentLight.DataD.xyz;
    vec3 LightSpecularColor = currentLight.DataD.xyz * Specular;

    vec2 uvs                = gl_FragCoord.xy / ScreenInfo.zw;
    vec4 ShadowPxlWorldPos  = vec4(PxlWorldPos + CamRealPosition, 1.0);
    float shadow            = ShadowCalculationLightingShader(PxlViewPos, LightDir, PxlNormal, ShadowPxlWorldPos);
    vec3 ShadowColor        = max(vec3(shadow), RendererInfo2.rgb); 

    vec3 ViewDir            = normalize(CameraPosition - PxlWorldPos);
    vec3 Half               = normalize(LightDir + ViewDir);
    float NdotL             = max(dot(PxlNormal, LightDir), 0.0);
    float NdotH             = max(dot(PxlNormal, Half), 0.0);

    float SpecularFactor    = pow(NdotH, 64);

    LightDiffuseColor      *= (currentLight.DataA.y * NdotL);
    LightSpecularColor     *= (SpecularFactor * currentLight.DataA.z) / KPI;
    
    vec3 ao                 = min(vec3(AO * SSAO), ShadowColor);
    vec3 TotalLight         = Albedo;
    TotalLight             += LightSpecularColor * ShadowColor;
    TotalLight             *= LightDiffuseColor;
    TotalLight             /= KPI;
    TotalLight             *= MatAlpha;
    TotalLight             *= ao;
    return max(TotalLight, RendererInfo2.rgb * Albedo);
}
)", "vec3 CalcPointLightBasic(");
    }
#pragma endregion

#pragma region Attenuation Function Basic
    if (ShaderHelper::lacksDefinition(code, "CalculateAttenuationBasic(", "float CalculateAttenuationBasic(")) {
        ShaderHelper::insertStringRightBeforeLineContent(code, R"(
float CalculateAttenuationBasic(in Light inLight, float Dist, float radius){
    float attenuation = 0.0;
    if (inLight.DataE.z == 0.0) {
        attenuation = 1.0 / max(1.0, inLight.DataB.z);
    } else if (inLight.DataE.z == 1.0) {
        attenuation = 1.0 / max(1.0, Dist);
    } else if (inLight.DataE.z == 2.0) {
        attenuation = 1.0 / max(1.0, Dist * Dist);
    } else if (inLight.DataE.z == 3.0) {
        attenuation = 1.0 / max(1.0, inLight.DataB.z + (inLight.DataB.w * Dist) + (inLight.DataC.x * Dist * Dist));
    } else if (inLight.DataE.z == 4.0) {
        attenuation = 1.0 / max(1.0, pow((Dist / radius) + 1.0, 2.0));
    }
    return attenuation;
}
)", "vec3 CalcLightInternalBasic(");
    }
#pragma endregion

}