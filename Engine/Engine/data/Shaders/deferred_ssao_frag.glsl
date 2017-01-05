#version 120

uniform sampler2D gNormalMap;
uniform sampler2D gRandomMap;
uniform sampler2D gMiscMap;
uniform sampler2D gLightMap;
uniform sampler2D gDepthMap;

uniform int doSSAO;
uniform int doBloom;

uniform vec3 gCameraPosition;
uniform float gRadius;
uniform float gIntensity;
uniform float gBias;
uniform float gScale;
uniform int gSampleCount;
uniform int gNoiseTextureSize;

uniform vec2 poisson[32];

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
float l(float a, float b, float w){
    return a + w*(b-a);
}

float occlude(vec2 uv, vec2 offsetUV, vec3 origin, vec3 normal){
    vec3 diff = (reconstruct_world_pos(uv+offsetUV)) - origin;
    vec3 vec = normalize(diff);
    float dist = length(diff)/gScale;
    return max(0.0,dot(normal,vec)-gBias)*(1.0/(1.0+dist))*gIntensity;
}
void main(void){
    vec2 uv = gl_TexCoord[0].st*2.0;
	vec3 worldPosition = reconstruct_world_pos(uv);
    vec3 normal = texture2D(gNormalMap, uv).xyz;
    vec2 randomVector = normalize(texture2D(gRandomMap, gl_TexCoord[0].st / gNoiseTextureSize).xy * 2.0 - 1.0);

    float camDist = distance(worldPosition,gCameraPosition);
    float rad = gRadius / camDist;

    if(doSSAO == 1){
        if(normal.r > 0.9999 && normal.g > 0.9999 && normal.b > 0.9999){ gl_FragColor.a = 1.0; }
        else{
            float occlusion = 0.0;
            for (int i = 0; i < gSampleCount; ++i) {
                vec2 coord1 = reflect(poisson[i], randomVector)*rad;
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
        if(brightness > 2.4 || Glow > 0.01f){
            float brightScale = log(brightness) * 0.7;
			float glowScale = Glow * 0.73;
            gl_FragColor.rgb = vec3(lighting * max(glowScale, brightScale)) * max(glowScale, brightScale);
        }
    }
    else{
        gl_FragColor.rgb = vec3(0.0);
    }
}