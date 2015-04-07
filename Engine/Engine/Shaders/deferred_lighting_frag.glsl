#version 330

uniform int gLightType;

uniform vec3 gColor;
uniform float gAmbientIntensity;
uniform float gDiffuseIntensity;

uniform float gConstant;
uniform float gLinear;
uniform float gExp;

uniform vec3 gDirection;
uniform vec3 gPosition;

uniform sampler2D gNormalMap;
uniform sampler2D gPositionMap;

uniform vec3 gEyeWorldPos;
uniform vec2 gScreenSize;
uniform mat4 VPInverse;

uniform float gMatSpecularIntensity;
uniform float gSpecularPower;

layout (location = 0) out vec4 LightOut;

vec2 CalcTexCoord(){return gl_FragCoord.xy / gScreenSize;}

vec4 CalcLightInternal(vec3 _lightDir,vec3 _worldPos,vec3 _norm){
    vec4 AmbientColor = vec4(gColor, 1.0) * gAmbientIntensity;
    float DiffuseFactor = dot(_norm, -_lightDir);

    vec4 DiffuseColor  = vec4(0);

	vec3 N = normalize(_norm);
    vec3 L = normalize(gPosition);
    vec3 H = normalize(gPosition + _worldPos);

    float df = max(0.0, dot(N, L));
    float sf = max(0.0, dot(N, H));
    sf = pow(sf, gMatSpecularIntensity);

    if (DiffuseFactor > 0.0) {
        DiffuseColor = vec4(gColor, 1.0) * gDiffuseIntensity * DiffuseFactor;
    }
    return AmbientColor + DiffuseColor * df + vec4(gColor,1) * sf;
}
vec4 CalcPointLight(vec3 _worldPos, vec3 _norm){
    vec3 LightDirection = _worldPos - gPosition;
    float Distance = length(LightDirection);
    LightDirection = normalize(LightDirection);

    vec4 c = CalcLightInternal(LightDirection, _worldPos, _norm);

    float a =  gConstant + (gLinear * Distance) + (gExp * Distance * Distance);
    a = max(1.0, a);
    return c / a;
}
vec4 CalcSunLight(vec3 _worldPos, vec3 _norm){
    vec3 LightDirection = normalize(_worldPos - gPosition);
    return CalcLightInternal(LightDirection, _worldPos, _norm);
}
vec4 CalcDirectionalLight(vec3 _worldPos, vec3 _norm){ 
	return CalcLightInternal(gDirection,_worldPos,_norm); 
}
vec4 CalcSpotLight(vec3 _worldPos, vec3 _norm){
	return vec4(0);
}
vec3 decodeLocation(vec2 texCoord){
	vec4 clipSpaceLocation;
    clipSpaceLocation.xy = texCoord * 2.0 - 1.0;
    clipSpaceLocation.z = texture(gPositionMap, texCoord).r * 2.0 - 1.0;
    clipSpaceLocation.w = 1.0;
    vec4 homogenousLocation = VPInverse * clipSpaceLocation;
    return homogenousLocation.xyz / homogenousLocation.w;
}
void main(){
	vec2 texCoord = CalcTexCoord();
	vec3 position = decodeLocation(texCoord);
    vec3 normal = (texture2D( gNormalMap, texCoord).xyz * 2.0 - 1.0);
	normal = normalize(normal);

	if(gLightType == 0)
		LightOut = CalcSunLight(position,normal);
	else if(gLightType == 1)
		LightOut = CalcPointLight(position,normal);
	else if(gLightType == 2)
		LightOut = CalcDirectionalLight(position,normal);
	else if(gLightType == 3)
		LightOut = CalcSpotLight(position,normal);
}