
USE_LOG_DEPTH_FRAGMENT
USE_MAX_MATERIAL_LAYERS_PER_COMPONENT
USE_MAX_MATERIAL_COMPONENTS
USE_MAX_LIGHTS_PER_PASS

struct InData {
    vec2  uv;
    vec4  diffuse;
    vec4  objectColor;
    vec3  normals;
    float glow;
    float specular;
    float ao;
    float metalness;
    float smoothness;
    vec3  materialF0;
    vec3  worldPosition;
};
struct Layer {
    vec4 data1; //x = blend mode | y = texture enabled? | z = mask enabled? | w = cubemap enabled?
    vec4 data2;
    sampler2D texture;
    sampler2D mask;
    samplerCube cubemap;
    vec2 uvModifications;
};
struct Component {
    int numLayers;
    int componentType;
    Layer layers[MAX_MATERIAL_LAYERS_PER_COMPONENT];
};
struct Light{
    vec4 DataA; //x = ambient, y = diffuse, z = specular, w = LightDirection.x
    vec4 DataB; //x = LightDirection.y, y = LightDirection.z, z = const, w = linear
    vec4 DataC; //x = exp, y = LightPosition.x, z = LightPosition.y, w = LightPosition.z
    vec4 DataD; //x = LightColor.r, y = LightColor.g, z = LightColor.b, w = LightType
    vec4 DataE; //x = cutoff, y = outerCutoff, z = AttenuationModel, w = UNUSED
};
uniform Light       light[MAX_LIGHTS_PER_PASS];
uniform int         numLights;
uniform Component   components[MAX_MATERIAL_COMPONENTS];
uniform int         numComponents;
uniform vec4        MaterialBasePropertiesOne; //x = BaseGlow, y = BaseAO, z = BaseMetalness, w = BaseSmoothness
uniform vec4        MaterialBasePropertiesTwo; //x = BaseAlpha, y = diffuseModel, z = specularModel, w = UNUSED

uniform int Shadeless;

uniform vec4 Object_Color;
uniform vec4 Material_F0AndID;
uniform vec3 Gods_Rays_Color;
uniform float OfflineGlowFactor;

varying vec3 WorldPosition;
varying vec2 UV;
varying vec3 Normals;
varying mat3 TBN;
flat varying vec3 CamPosition;
flat varying vec3 CamRealPosition;
varying vec3 TangentCameraPos;
varying vec3 TangentFragPos;

uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;
uniform vec4 ScreenData; //x = GIContribution, y = gamma, z = winSize.x, w = winSize.y

const float MAX_REFLECTION_LOD = 5.0;

vec3 SchlickFrenselRoughness(float _theta, vec3 _F0, float _roughness){
    vec3 ret = _F0 + (max(vec3(1.0 - _roughness),_F0) - _F0) * pow(1.0 - _theta,5.0);
    return ret;
}
void main(){
    InData inData;
    inData.uv = UV;
    inData.diffuse = vec4(0.0,0.0,0.0,0.0001); //this is extremely wierd, but we need some form of alpha to get painters algorithm to work...
    inData.objectColor = Object_Color;
    inData.normals = normalize(Normals);
    inData.glow = MaterialBasePropertiesOne.x;
    inData.specular = 1.0;
    inData.ao = MaterialBasePropertiesOne.y;
    inData.metalness = MaterialBasePropertiesOne.z;
    inData.smoothness = MaterialBasePropertiesOne.w;
    inData.materialF0 = Material_F0AndID.rgb;
    inData.worldPosition = WorldPosition;

    for (int j = 0; j < numComponents; ++j) {
        ProcessComponent(components[j], inData);
    }
    vec2 encodedNormals = EncodeOctahedron(inData.normals); //yes these two lines are evil and not needed, but they sync up the results with the deferred pass...
    inData.normals = DecodeOctahedron(encodedNormals);
    vec3 lightTotal = ConstantZeroVec3;

    inData.glow *= OfflineGlowFactor;
    inData.diffuse.a *= MaterialBasePropertiesTwo.x;

    vec4 GodRays = vec4(Gods_Rays_Color,1.0);
    float GodRaysRG = Pack2NibblesInto8BitChannel(GodRays.r,GodRays.g);

    gl_FragData[1] = vec4(encodedNormals, 0.0, 0.0); //old: OutMatIDAndAO, OutPackedMetalnessAndSmoothness. keeping normals around for possible decals later
    gl_FragData[2] = vec4(inData.glow, inData.specular, GodRaysRG, GodRays.b);
    gl_FragData[3] = vec4(inData.diffuse.rgb, 1.0);
    if(Shadeless != 1){
        vec3 lightCalculation = ConstantZeroVec3;
        for (int j = 0; j < numLights; ++j) {
           Light currentLight = light[j];
            vec3 lightCalculation = ConstantZeroVec3;
            vec3 LightPosition = vec3(currentLight.DataC.yzw) - CamRealPosition;
            vec3 LightDirection = normalize(vec3(currentLight.DataA.w,currentLight.DataB.x,currentLight.DataB.y));
            if(currentLight.DataD.w == 0.0){       //sun
                lightCalculation = CalcLightInternalForward(currentLight, normalize(LightPosition - WorldPosition),WorldPosition,inData.normals,inData);
            }else if(currentLight.DataD.w == 1.0){ //point
                lightCalculation = CalcPointLightForward(currentLight, LightPosition,WorldPosition,inData.normals,inData);
            }else if(currentLight.DataD.w == 2.0){ //directional
                lightCalculation = CalcLightInternalForward(currentLight, LightDirection,WorldPosition,inData.normals,inData);
            }else if(currentLight.DataD.w == 3.0){ //spot
                lightCalculation = CalcSpotLightForward(currentLight, LightDirection,LightPosition,WorldPosition,inData.normals,inData);
            }else if(currentLight.DataD.w == 4.0){ //rod
                lightCalculation = CalcRodLightForward(currentLight, vec3(currentLight.DataA.w,currentLight.DataB.xy),currentLight.DataC.yzw,WorldPosition,inData.normals,inData);
            }
            lightTotal += lightCalculation;
        } 
        vec3 ViewDir = normalize(CameraPosition - WorldPosition);
        vec3 R = reflect(-ViewDir, inData.normals);
        float VdotN = max(0.0, dot(ViewDir,inData.normals));
//      float ssaoValue = 1.0 - texture2D(gSSAOMap,uv).a;
//      float ao = (fract(matIDandAO)+0.0001) * ssaoValue;
        float ao = inData.ao;
        vec3 F0 = mix(inData.materialF0, inData.diffuse.rgb, vec3(inData.metalness));
        vec3 Frensel = F0;
        float roughness = 1.0 - inData.smoothness;
        vec3 irradianceColor = textureCube(irradianceMap, inData.normals).rgb;
        vec3 kS = SchlickFrenselRoughness(VdotN,Frensel,roughness);
        vec3 kD = ConstantOneVec3 - kS;
        kD *= 1.0 - inData.metalness;
        vec3 GIContribution = Unpack3FloatsInto1FloatUnsigned(ScreenData.x); //x = diffuse, y = specular, z = global
        vec3 GIDiffuse = irradianceColor * inData.diffuse.rgb * kD * GIContribution.x;

        vec3 prefilteredColor = textureCubeLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;
        vec2 brdf  = texture2D(brdfLUT, vec2(VdotN, roughness)).rg;
        vec3 GISpecular = prefilteredColor * (kS * brdf.x + brdf.y) * GIContribution.y;

        vec3 TotalIrradiance = (GIDiffuse + GISpecular) * ao;
        TotalIrradiance = pow(TotalIrradiance, vec3(1.0 / ScreenData.y)); //ScreenData.y is gamma
        lightTotal += TotalIrradiance * GIContribution.z * MaterialBasePropertiesTwo.x;

        inData.diffuse.rgb = lightTotal;
    }
    gl_FragData[0] = inData.diffuse;
}