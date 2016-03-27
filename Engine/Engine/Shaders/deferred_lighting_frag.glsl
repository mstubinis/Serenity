#version 120

uniform int LightType;

uniform vec3 LightColor;
uniform float LightAmbientIntensity;
uniform float LightDiffuseIntensity;
uniform float LightSpecularPower;

uniform float LightConstant;
uniform float LightLinear;
uniform float LightExp;

uniform vec3 LightDirection;
uniform vec3 LightPosition;

uniform sampler2D gNormalMap;
uniform sampler2D gPositionMap;
uniform sampler2D gGlowMap;

uniform vec3 gCameraPosition;
uniform vec2 gScreenSize;
uniform mat4 VPInverse;

vec2 CalcTexCoord(){return gl_FragCoord.xy / gScreenSize;}

vec4 CalcLightInternal(vec3 LightDir,vec3 PxlWorldPos,vec3 PxlNormal){
    vec4 AmbientColor = vec4(LightColor, 1.0) * LightAmbientIntensity;
    float Lambertian = max(dot(LightDir,PxlNormal), 0.0);

    vec4 DiffuseColor  = vec4(0.0);
    vec4 SpecularColor = vec4(0.0);

    if (Lambertian > 0.0) {
        DiffuseColor = vec4(LightColor, 1.0) * LightDiffuseIntensity * Lambertian;
        vec3 ViewVector = normalize(-PxlWorldPos + gCameraPosition);

		// this is blinn phong
		vec3 halfDir = normalize(LightDir + ViewVector);
		float SpecularAngle = max(dot(halfDir, PxlNormal), 0.0);
		SpecularAngle = pow(SpecularAngle, LightSpecularPower);

		//this is regular phong
		//vec3 reflectDir = reflect(-LightDir, PxlNormal);
		//float SpecularAngle = max(dot(reflectDir, ViewVector), 0.0);
		//SpecularAngle = pow(SpecularAngle, LightSpecularPower/4.0);

        if (SpecularAngle > 0.0 && LightSpecularPower > 0.001f) {
			float materialSpecularity = texture2D(gGlowMap,CalcTexCoord()).b;
            SpecularColor = vec4(LightColor, 1.0) * materialSpecularity * SpecularAngle;
        }
    }
    return (AmbientColor + DiffuseColor + SpecularColor);
}
vec4 CalcPointLight(vec3 PxlWorldPos, vec3 PxlNormal){
    vec3 LightDir = PxlWorldPos + LightPosition;
    float Distance = length(LightDir);
    LightDir = normalize(LightDir);

    vec4 c = CalcLightInternal(LightDir, PxlWorldPos, PxlNormal);

    float a =  LightConstant + (LightLinear * Distance) + (LightExp * Distance * Distance);
    a = max(1.0, a);
    return c / a;
}
vec4 CalcSpotLight(vec3 PxlWorldPos, vec3 PxlNormal){
	return vec4(0);
}
void main(){
	vec2 texCoord = CalcTexCoord();
	vec3 PxlPosition = texture2D(gPositionMap,texCoord).xyz;
    vec3 PxlNormal = normalize(texture2D(gNormalMap, texCoord).rgb);

	vec4 lightCalculation = vec4(0);

	if(LightType == 0)
		lightCalculation = CalcLightInternal(normalize(PxlPosition + LightPosition),PxlPosition,PxlNormal);
	else if(LightType == 1)
		lightCalculation = CalcPointLight(PxlPosition,PxlNormal);
	else if(LightType == 2)
		lightCalculation = CalcLightInternal(LightDirection,PxlPosition,PxlNormal);
	else if(LightType == 3)
		lightCalculation = CalcSpotLight(PxlPosition,PxlNormal);

	gl_FragColor = lightCalculation;
}