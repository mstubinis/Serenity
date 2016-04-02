#version 120

uniform sampler2D gNormalMap;
uniform sampler2D gPositionMap;
uniform sampler2D gRandomMap;

uniform vec2 gScreenSize;
uniform float gRadius;
uniform float gIntensity;
uniform float gBias;
uniform float gScale;
uniform int gSampleCount;
uniform int gNoiseTextureSize;

uniform vec2 poisson[64];

vec3 calculatePosition(vec2 uv){
    return texture2D(gPositionMap, uv).xyz;
}
float occlude(vec2 uv, vec2 offsetUV, vec3 origin, vec3 normal){
    vec3 diff = calculatePosition(uv+offsetUV)-origin;
    vec3 vec = normalize(diff);
    float dist = length(diff)/gScale;
    return max(0.0,dot(normal,vec)-gBias)*(1.0/(1.0+dist))*gIntensity;
}
void main(){
	vec2 uv = gl_FragCoord.xy / gScreenSize;
    vec3 worldPosition = calculatePosition(uv);
    vec3 normal = texture2D(gNormalMap, uv).xyz;
    vec2 randomVector = normalize(texture2D(gRandomMap, gScreenSize * uv / gNoiseTextureSize).xy * 2.0 - 1.0);

	if(normal.r > 0.9999 && normal.g > 0.9999 && normal.b > 0.9999){ gl_FragColor.g = 1.0; }
	else{
		float occlusion = 0.0;
		for (int i = 0; i < gSampleCount; i++) {
			vec2 coord1 = reflect(poisson[i], randomVector)*gRadius;
			vec2 coord2 = vec2(coord1.x*0.707 - coord1.y*0.707, coord1.x*0.707 + coord1.y*0.707);
			occlusion += occlude(uv, coord1 * 0.25, worldPosition, normal);
			occlusion += occlude(uv, coord2 * 0.50, worldPosition, normal);
			occlusion += occlude(uv, coord1 * 0.75, worldPosition, normal);
			occlusion += occlude(uv, coord2, worldPosition, normal);
		}
		occlusion /= (gSampleCount*4.0);
		gl_FragColor.g = 1.0-occlusion;
	}
}