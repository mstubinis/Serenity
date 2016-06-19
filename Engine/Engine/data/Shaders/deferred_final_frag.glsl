#version 120

uniform sampler2D gDiffuseMap;
uniform sampler2D gLightMap;
uniform sampler2D gBloomMap;
uniform sampler2D gNormalMap;
uniform sampler2D gMiscMap;
uniform sampler2D gGodsRaysMap;

uniform int HasLighting;
uniform int HasBloom;
uniform int HasHDR;

uniform float gamma;

uniform vec2 gScreenSize;
uniform vec3 gAmbientColor;

void main(){
    vec2 uv = gl_FragCoord.xy / gScreenSize;
    vec4 diffuse = texture2D(gDiffuseMap, uv);
	vec3 normals = texture2D(gNormalMap,uv).rgb;
    vec4 lightMap = texture2D(gLightMap, uv);
    vec4 bloom = texture2D(gBloomMap,uv);

	vec3 rays = texture2D(gGodsRaysMap,uv).rgb;

	float ssao = texture2D(gMiscMap,uv).g;
	vec3 hdr = texture2D(gMiscMap,uv).rgb;

	lightMap = max(vec4(gAmbientColor,1.0),lightMap);
	if(HasLighting == 0){
		lightMap = diffuse;
	}
	if(HasHDR == 1){
		lightMap = vec4(hdr,1.0);
	}
	if(HasBloom == 1){
		lightMap += (bloom / gamma);
	}
	if(normals.r > 0.999 && normals.g > 0.999 && normals.b > 0.999){
		gl_FragColor = diffuse;
	}
	else{
		gl_FragColor = lightMap;
	}
	gl_FragColor += vec4(rays,1.0);
}