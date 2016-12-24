#version 120

uniform int LightType;

uniform vec3 LightColor;

uniform vec3 LightIntensities; //x = ambient, y = diffuse, z = specular
uniform vec3 LightData; //x = constant, y = linear z = exponent

uniform vec3 LightDirection;
uniform vec3 LightPosition;

uniform sampler2D gNormalMap;
uniform sampler2D gPositionMap;
uniform sampler2D gMiscMap;
uniform sampler2D gDiffuseMap;
uniform vec2 gScreenSize;

uniform vec3 gCameraPosition;

uniform vec4 materials[255];

vec4 CalcLightInternal(vec3 LightDir,vec3 PxlWorldPos,vec3 PxlNormal,vec2 uv){
    if(PxlNormal.r > 0.9999 && PxlNormal.g > 0.9999 && PxlNormal.b > 0.9999){
        return vec4(0);
    }
    vec4 AmbientColor = vec4(LightColor, 1.0) * LightIntensities.x;
    float Lambertian = max(dot(LightDir,PxlNormal), 0.0);
    float Glow = texture2D(gMiscMap,uv).r;
    float SpecularMap = texture2D(gMiscMap,uv).g;

    vec4 diffuseMapColor = vec4(texture2D(gDiffuseMap,uv).rgb, 1.0);

    vec4 DiffuseColor  = vec4(0.0);
    vec4 SpecularColor = vec4(0.0);
    vec4 lightWithoutSpecular = vec4(0.0);
    if (Lambertian > 0.0) {
        DiffuseColor = vec4(LightColor, 1.0) * LightIntensities.y * (pow(Lambertian,0.75) * 1.2); //this modification to Lambertian makes lighting look more realistic
        vec3 ViewVector = normalize(-PxlWorldPos + gCameraPosition);

        // this is blinn phong
        vec3 halfDir = normalize(LightDir + ViewVector);
        float SpecularAngle = max(dot(halfDir, PxlNormal), 0.0);
        highp int index = int(texture2D(gMiscMap,uv).b * 255.0);
        float materialSpecularity = materials[index].g;
        SpecularAngle = pow(SpecularAngle, materialSpecularity);

        if (SpecularAngle > 0.0 && LightIntensities.z > 0.001) {
            SpecularColor = (vec4(LightColor, 1.0) * LightIntensities.z * SpecularAngle) * SpecularMap;
        }
        lightWithoutSpecular = max(AmbientColor, (AmbientColor + DiffuseColor) * diffuseMapColor);
    }
    else{
        lightWithoutSpecular = AmbientColor;
    }

    if(Glow > 0.99){ return diffuseMapColor; }
    return max(Glow*diffuseMapColor,lightWithoutSpecular + SpecularColor);
}
vec4 CalcPointLight(vec3 PxlWorldPos, vec3 PxlNormal, vec2 uv){
    vec3 LightDir = LightPosition - PxlWorldPos;
    float Distance = length(LightDir);
    LightDir = normalize(LightDir);

    vec4 c = CalcLightInternal(LightDir, PxlWorldPos, PxlNormal, uv);

    float a =  max(1.0 , LightData.x + (LightData.y * Distance) + (LightData.z * Distance * Distance));
    return c / a;
}
vec4 CalcSpotLight(vec3 PxlWorldPos, vec3 PxlNormal, vec2 uv){
    return vec4(0);
}
void main(void){
    //vec2 uv = gl_TexCoord[0].st; //this cannot be used for point light mesh
    vec2 uv = gl_FragCoord.xy / gScreenSize;

    vec3 PxlPosition = texture2D(gPositionMap,uv).xyz;
    vec3 PxlNormal = (texture2D(gNormalMap, uv).rgb);

    vec4 lightCalculation = vec4(0);

    if(LightType == 0)
        lightCalculation = CalcLightInternal(normalize(LightPosition - PxlPosition),PxlPosition,PxlNormal,uv);
    else if(LightType == 1)
        lightCalculation = CalcPointLight(PxlPosition,PxlNormal,uv);
    else if(LightType == 2)
        lightCalculation = CalcLightInternal(LightDirection,PxlPosition,PxlNormal,uv);
    else if(LightType == 3)
        lightCalculation = CalcSpotLight(PxlPosition,PxlNormal,uv);

    gl_FragData[0].rgb = lightCalculation.rgb;
}