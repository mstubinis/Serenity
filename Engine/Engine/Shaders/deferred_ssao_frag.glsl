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

const vec2 poisson[] =  vec2[]( vec2( 0.344, 0.293),
                                vec2( 0.537,-0.473),
                                vec2(-0.264,-0.418),
                                vec2(-0.814, 0.914));

vec3 calculatePosition(vec2 uv){
    return texture2D(gPositionMap, uv).xyz;
}
vec2 CalcTexCoord(){ 
	return gl_FragCoord.xy / gScreenSize; 
}
float occlude(vec2 uv, vec2 offsetUV, vec3 origin, vec3 normal){
    vec3 diff = calculatePosition(uv+offsetUV)-origin;
    vec3 vec = normalize(diff);
    float dist = length(diff)/gScale;
    return max(0.0,dot(normal,vec)-gBias)*(1.0/(1.0+dist))*gIntensity;
}
vec2 getRandom(vec2 uv){
	vec2 res = texture2D(gRandomMap, gScreenSize * uv / 64).xy * 2.0 - 1.0;
	return normalize(res);
}
void main(){
	vec2 samplePosition = CalcTexCoord();
    vec3 origin = calculatePosition(samplePosition);
    vec3 normal = texture2D(gNormalMap, samplePosition).xyz;
    vec2 random = getRandom(samplePosition);

    float radius = gRadius/origin.z;
    float occlusion = 0.0;

    for (int i = 0; i < gSampleCount; i++) {
        vec2 coord1 = reflect(poisson[i], random)*radius;
        vec2 coord2 = vec2(coord1.x*0.707 - coord1.y*0.707, coord1.x*0.707 + coord1.y*0.707);
        occlusion += occlude(samplePosition, coord1 * 0.25, origin, normal);
        occlusion += occlude(samplePosition, coord2 * 0.50, origin, normal);
        occlusion += occlude(samplePosition, coord1 * 0.75, origin, normal);
        occlusion += occlude(samplePosition, coord2, origin, normal);
    }
	occlusion /= (gSampleCount*4.0);
    //gl_FragColor.g = clamp(1-occlusion,0.01,0.99);//this clamp removes artifacts from gaussian blur. will need to fix later
	gl_FragColor.g = 1-occlusion;
}