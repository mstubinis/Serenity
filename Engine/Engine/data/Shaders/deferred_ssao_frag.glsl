#version 120

uniform sampler2D gNormalMap;
uniform sampler2D gPositionMap;
uniform sampler2D gRandomMap;
uniform sampler2D gMiscMap;
uniform sampler2D gLightMap;

uniform int doSSAO;
uniform int doBloom;

uniform vec2 gScreenSize;
uniform float gRadius;
uniform float gIntensity;
uniform float gBias;
uniform float gScale;
uniform int gSampleCount;
uniform int gNoiseTextureSize;
uniform int far;

uniform vec2 poisson[64];

float l(float a, float b, float w){
	return a + w*(b-a);
}

float occlude(vec2 uv, vec2 offsetUV, vec3 origin, vec3 normal){
    vec3 diff = texture2D(gPositionMap,uv+offsetUV).xyz - origin;
    vec3 vec = normalize(diff);
    float dist = length(diff)/gScale;
    return max(0.0,dot(normal,vec)-gBias)*(1.0/(1.0+dist))*gIntensity;
}
void main(){
    vec2 uv = gl_FragCoord.xy / gScreenSize*2.0;
    vec3 worldPosition = texture2D(gPositionMap,uv).xyz;
    vec3 normal = texture2D(gNormalMap, uv).xyz;
    vec2 randomVector = normalize(texture2D(gRandomMap, gScreenSize * uv / gNoiseTextureSize).xy * 2.0 - 1.0);

	if(doSSAO == 1){
		if(normal.r > 0.9999 && normal.g > 0.9999 && normal.b > 0.9999){ gl_FragColor.a = 1.0; }
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
			gl_FragColor.a = 1.0-occlusion;
		}
	}
	else{
		gl_FragColor.a = 1.0;
	}

	if(doBloom == 1){
		float Glow = texture2D(gMiscMap,uv).r;
		vec3 lighting = texture2D(gLightMap,uv).rgb;
		float brightness = dot(lighting, vec3(0.2126, 0.7152, 0.0722));
		if(brightness > 1.14 || Glow > 0.01f)
			gl_FragColor.rgb = vec3(lighting*max(Glow,brightness));
	}
	else{
		gl_FragColor.rgb = vec3(0.0);
	}
}