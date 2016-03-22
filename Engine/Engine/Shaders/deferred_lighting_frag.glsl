#version 120

uniform int gLightType;

uniform vec3 gColor;
uniform float gAmbientIntensity;
uniform float gDiffuseIntensity;

uniform float gConstant;
uniform float gLinear;
uniform float gExp;


uniform vec3 gDirection;
uniform vec3 gLightPosition;

uniform sampler2D gNormalMap;
uniform sampler2D gPositionMap;
uniform sampler2D gGlowMap;

uniform vec3 gCameraPosition;
uniform vec2 gScreenSize;
uniform mat4 VPInverse;

uniform float gSpecularPower;

vec2 CalcTexCoord(){return gl_FragCoord.xy / gScreenSize;}

vec4 CalcLightInternal(vec3 _lightDir,vec3 _worldPos,vec3 _norm){
    vec4 AmbientColor = vec4(gColor, 1.0) * gAmbientIntensity;
    float Lambertian = dot(_norm, -_lightDir);

    vec4 DiffuseColor  = vec4(0.0);
    vec4 SpecularColor = vec4(0.0);

    if (Lambertian > 0.0) {

        DiffuseColor = vec4(gColor, 1.0) * gDiffuseIntensity * Lambertian;

        vec3 LightSourceToEye = normalize(gCameraPosition - _worldPos);
        vec3 ReflectionVector = normalize(reflect(_lightDir, _norm));
        float SpecularFactor = dot(LightSourceToEye, ReflectionVector);
        SpecularFactor = pow(SpecularFactor, gSpecularPower);
        if (SpecularFactor > 0.0) {
			float materialSpecularity = texture2D(gGlowMap,CalcTexCoord()).b;
            SpecularColor = vec4(gColor, 1.0) * materialSpecularity * SpecularFactor;
        }
    }
    return (AmbientColor + DiffuseColor + SpecularColor);
}
vec4 CalcPointLight(vec3 PxlWorldPos, vec3 PxlNormal){
    vec3 LightDir = PxlWorldPos - gLightPosition;
    float Distance = length(LightDir);
    LightDir = normalize(LightDir);

    vec4 c = CalcLightInternal(LightDir, PxlWorldPos, PxlNormal);

    float a =  gConstant + (gLinear * Distance) + (gExp * Distance * Distance);
    a = max(1.0, a);
    return c / a;
}
vec4 CalcSpotLight(vec3 _worldPos, vec3 _norm){
	return vec4(0);
}
void main(){
	vec2 texCoord = CalcTexCoord();
	vec3 position = texture2D(gPositionMap,texCoord).xyz;
    vec3 normal = normalize(texture2D( gNormalMap, texCoord).rgb);

	vec4 lightCalculation = vec4(0);

	if(gLightType == 0)
		lightCalculation = CalcLightInternal(normalize(position - gLightPosition),position,normal);
	else if(gLightType == 1)
		lightCalculation = CalcPointLight(position,normal);
	else if(gLightType == 2)
		lightCalculation = CalcLightInternal(gDirection,position,normal);
	else if(gLightType == 3)
		lightCalculation = CalcSpotLight(position,normal);

	gl_FragColor = lightCalculation;
}