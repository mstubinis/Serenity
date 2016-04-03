#version 120

uniform int LightType;

uniform int HasSSAO;

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
uniform sampler2D gGlowMap;
uniform sampler2D gDiffuseMap;
uniform sampler2D gRegularDepthMap;

uniform vec3 gCameraPosition;
uniform vec2 gScreenSize;

uniform mat4 VPInverse;

vec4 CalcLightInternal(vec3 LightDir,vec3 PxlWorldPos,vec3 PxlNormal,vec2 uv){
    vec4 AmbientColor = vec4(LightColor, 1.0) * LightAmbientIntensity;
    float Lambertian = max(dot(LightDir,PxlNormal), 0.0);

	vec4 diffuseMapColor = vec4(texture2D(gDiffuseMap,uv).rgb, 1.0);

    vec4 DiffuseColor  = vec4(0.0);
    vec4 SpecularColor = vec4(0.0);

    if (Lambertian > 0.0) {
        DiffuseColor = vec4(LightColor, 1.0) * LightDiffuseIntensity * Lambertian;
        vec3 ViewVector = normalize(-PxlWorldPos + gCameraPosition);

		// this is blinn phong
		vec3 halfDir = normalize(LightDir + ViewVector);
		float SpecularAngle = max(dot(halfDir, PxlNormal), 0.0);
		SpecularAngle = pow(SpecularAngle, LightSpecularPower);

        if (SpecularAngle > 0.0 && LightSpecularPower > 0.001f) {
			float materialSpecularity = texture2D(gGlowMap,uv).b;
            SpecularColor = vec4(LightColor, 1.0) * materialSpecularity * SpecularAngle;
        }
    }
	if(PxlNormal.r > 0.9999 && PxlNormal.g > 0.9999 && PxlNormal.b > 0.9999){
		return diffuseMapColor;
	}
	float ssao = 1.0;
	if(HasSSAO == 1){
		ssao = texture2D(gGlowMap,uv).g;
	}
    return (((AmbientColor + DiffuseColor) * diffuseMapColor ) * vec4(ssao)) + SpecularColor;
}
vec4 CalcPointLight(vec3 PxlWorldPos, vec3 PxlNormal, vec2 uv){
    vec3 LightDir = LightPosition - PxlWorldPos;
    float Distance = length(LightDir);
    LightDir = normalize(LightDir);

    vec4 c = CalcLightInternal(LightDir, PxlWorldPos, PxlNormal, uv);

    float a =  max(1.0 , LightConstant + (LightLinear * Distance) + (LightExp * Distance * Distance));
    return c / a;
}
vec4 CalcSpotLight(vec3 PxlWorldPos, vec3 PxlNormal, vec2 uv){
	return vec4(0);
}
vec3 DecodePosition(vec2 uv){
	vec4 clipSpace = vec4(uv * 2.0 - 1.0,texture2D(gRegularDepthMap, uv).r * 2.0 - 1.0,1.0);
	vec4 pos = VPInverse * clipSpace;
	return pos.xyz / pos.w;
}
void main(){
	vec2 uv = gl_FragCoord.xy / gScreenSize;
	vec3 PxlPosition = DecodePosition(uv);
    vec3 PxlNormal = texture2D(gNormalMap, uv).rgb;

	vec4 lightCalculation = vec4(0);

	if(LightType == 0)
		lightCalculation = CalcLightInternal(normalize(LightPosition - PxlPosition),PxlPosition,PxlNormal,uv);
	else if(LightType == 1)
		lightCalculation = CalcPointLight(PxlPosition,PxlNormal,uv);
	else if(LightType == 2)
		lightCalculation = CalcLightInternal(LightDirection,PxlPosition,PxlNormal,uv);
	else if(LightType == 3)
		lightCalculation = CalcSpotLight(PxlPosition,PxlNormal,uv);

	gl_FragColor = lightCalculation;
}