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

uniform vec3 gCameraPosition;
uniform vec2 gScreenSize;
uniform mat4 VPInverse;

uniform float gMatSpecularIntensity;
uniform float gSpecularPower;

vec2 CalcTexCoord(){return gl_FragCoord.xy / gScreenSize;}

vec4 CalcLightInternal(vec3 _lightDir,vec3 _worldPos,vec3 _norm){
    vec4 AmbientColor = vec4(gColor, 1.0) * gAmbientIntensity;
    float DiffuseFactor = dot(_norm, -_lightDir);


    vec4 DiffuseColor = vec4(0.0);

    vec3 L = normalize(gLightPosition - gCameraPosition);
    vec3 H = normalize(L - gCameraPosition);

    float df = max(0.0, dot(_norm, L));
    float sf = max(0.0, dot(_norm, H));
    sf = pow(sf, gMatSpecularIntensity);

    if (DiffuseFactor > 0.0) {
        DiffuseColor = vec4(gColor, 1.0) * gDiffuseIntensity * DiffuseFactor;
    }
    return AmbientColor + DiffuseColor * (df + vec4(gColor,1.0) * sf);
}
vec4 CalcPointLight(vec3 _worldPos, vec3 _norm){
    vec3 LightDirection = _worldPos - gLightPosition;
    float Distance = length(LightDirection);
    LightDirection = normalize(LightDirection);

    vec4 c = CalcLightInternal(LightDirection, _worldPos, _norm);

    float a =  gConstant + (gLinear * Distance) + (gExp * Distance * Distance);
    a = max(1.0, a);
    return c / a;
}
vec4 CalcSunLight(vec3 _worldPos, vec3 _norm){
    vec3 LightDirection = normalize(_worldPos - gLightPosition);
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
    clipSpaceLocation.z = texture2D(gPositionMap, texCoord).r * 2.0 - 1.0;
    clipSpaceLocation.w = 1.0;
    vec4 homogenousLocation = VPInverse * clipSpaceLocation;
    return homogenousLocation.xyz / homogenousLocation.w;
}
void main(){
	vec2 texCoord = CalcTexCoord();
	vec3 position = decodeLocation(texCoord);
	vec4 normalTexture = texture2D( gNormalMap, texCoord);
    vec3 normal = normalTexture.rgb * 2.0 - 1.0;
	normal = normalize(normal);

	if(gLightType == 0)
		gl_FragColor = max(vec4(normalTexture.a),CalcSunLight(position,normal));
	else if(gLightType == 1)
		gl_FragColor = max(vec4(normalTexture.a),CalcPointLight(position,normal));
	else if(gLightType == 2)
		gl_FragColor = max(vec4(normalTexture.a),CalcDirectionalLight(position,normal));
	else if(gLightType == 3)
		gl_FragColor = max(vec4(normalTexture.a),CalcSpotLight(position,normal));
}