#version 120
#define MATERIAL_COUNT_LIMIT 255

uniform int LightType;

uniform float gamma;

uniform vec3 LightColor;

uniform vec3 LightIntensities; //x = ambient, y = diffuse, z = specular
uniform vec3 LightData;        //x = constant, y = linear z = exponent

uniform vec3 LightDirection;
uniform vec3 LightPosition;

uniform sampler2D gNormalMap;
uniform sampler2D gMiscMap;
uniform sampler2D gDiffuseMap;
uniform sampler2D gDepthMap;
uniform vec2 gScreenSize;

uniform vec3 gCameraPosition;

uniform vec4 materials[MATERIAL_COUNT_LIMIT];

uniform mat4 invVP;
uniform float nearz;
uniform float farz;

float linearize_depth(float depth){
    float a = farz / (farz - nearz);
    float b = farz * nearz / (nearz - farz);
    return (a + b / depth);
}
float invertLogDepth(float log_depth){
    return linearize_depth(pow(farz + 1.0, log_depth) - 1.0);
}
vec3 reconstruct_world_pos(vec2 _uv){
    float depth = texture2D(gDepthMap, _uv).r;
    vec4 wpos = invVP * (vec4(_uv, invertLogDepth(depth), 1.0) * 2.0 - 1.0);
    return wpos.xyz / wpos.w;
}
vec3 CalcLightInternal(vec3 LightDir,vec3 PxlWorldPos,vec3 PxlNormal,vec2 uv){
    vec3 DiffuseMap = texture2D(gDiffuseMap,uv).rgb;
    float Glow = texture2D(gMiscMap,uv).r;
    if((PxlNormal.r > 0.9999 && PxlNormal.g > 0.9999 && PxlNormal.b > 0.9999) || Glow > 0.99 ){
        return DiffuseMap;
    }
    vec3 AmbientColor  = LightColor * LightIntensities.x;
    vec3 DiffuseColor  = vec3(0.0);
    vec3 SpecularColor = vec3(0.0);
    vec3 TotalLight    = vec3(0.0);

    highp int index = int(texture2D(gMiscMap,uv).b * float(MATERIAL_COUNT_LIMIT));

    float Lambertian = max(dot(LightDir,PxlNormal), 0.0);

    DiffuseColor = DiffuseMap * ((Lambertian * LightColor) * LightIntensities.y);

    vec3 ViewDir = normalize(gCameraPosition - PxlWorldPos);
    float SpecularAngle = 0.0;

    float kPi = 3.14159265;

    if(materials[index].b == 0.0){ // this is blinn phong
        float kEnergyConservation = ( 8.0 + materials[index].g ) / ( 8.0 * kPi );
        vec3 Half = normalize(LightDir + ViewDir);
        SpecularAngle = kEnergyConservation * pow(max(dot(PxlNormal, Half), 0.0), materials[index].g);
    }		
    else if(materials[index].b == 1.0){ //this is phong
        float kEnergyConservation = ( 2.0 + materials[index].g ) / ( 2.0 * kPi );
        vec3 Reflect = reflect(-LightDir, PxlNormal);
        SpecularAngle = kEnergyConservation * pow(max(dot(ViewDir, Reflect), 0.0), materials[index].g);
    }	
    else if(materials[index].b == 2.0){ //this is PBR
    }
    SpecularColor = (LightColor * LightIntensities.z * SpecularAngle) * texture2D(gMiscMap,uv).g; //texture2D is specular map

    TotalLight = AmbientColor + DiffuseColor + SpecularColor;
    return max( Glow * DiffuseMap, TotalLight);
}
vec3 CalcPointLight(vec3 PxlWorldPos, vec3 PxlNormal, vec2 uv){
    vec3 LightDir = normalize(LightPosition - PxlWorldPos);
    float Distance = length(LightDir);

    vec3 c = CalcLightInternal(LightDir, PxlWorldPos, PxlNormal, uv);

    float attenuation =  1.0 / (max(1.0 , LightData.x + (LightData.y * Distance) + (LightData.z * Distance * Distance)));
    return c * attenuation;
}
vec3 CalcSpotLight(vec3 PxlWorldPos, vec3 PxlNormal, vec2 uv){
    return vec3(0);
}
void main(void){
    //vec2 uv = gl_TexCoord[0].st; //this cannot be used for point light mesh
    vec2 uv = gl_FragCoord.xy / gScreenSize;

    vec3 PxlPosition = reconstruct_world_pos(uv);
    vec3 PxlNormal = texture2D(gNormalMap, uv).rgb;

    vec3 lightCalculation = vec3(0.0);

    if(LightType == 0){
        lightCalculation = CalcLightInternal(normalize(LightPosition - PxlPosition),PxlPosition,PxlNormal,uv);
    }
    else if(LightType == 1){
        lightCalculation = CalcPointLight(PxlPosition,PxlNormal,uv);
    }
    else if(LightType == 2){
        lightCalculation = CalcLightInternal(LightDirection,PxlPosition,PxlNormal,uv);
    }
    else if(LightType == 3){
        lightCalculation = CalcSpotLight(PxlPosition,PxlNormal,uv);
    }
    gl_FragData[0].rgb = lightCalculation;
}
