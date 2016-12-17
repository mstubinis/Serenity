#version 120

uniform sampler2D gDiffuseMap;
uniform sampler2D gLightMap;
uniform sampler2D gBloomMap;
uniform sampler2D gNormalMap;
uniform sampler2D gMiscMap;
uniform sampler2D gGodsRaysMap;

uniform int HasSSAO;
uniform int HasLighting;
uniform int HasBloom;
uniform int HasHDR;

uniform float gamma;

uniform vec3 gAmbientColor;

void main(void){
    vec2 uv = gl_TexCoord[0].st;
    vec4 diffuse = texture2D(gDiffuseMap, uv);
	vec3 normals = texture2D(gNormalMap,uv).rgb;
    vec3 bloom = texture2D(gBloomMap,uv).rgb;
	vec3 rays = texture2D(gGodsRaysMap,uv).rgb;
	vec3 hdr = texture2D(gMiscMap,uv).rgb;
	vec4 lightMap = max(vec4(gAmbientColor,1.0),texture2D(gLightMap, uv));
	float ssao = texture2D(gBloomMap,uv).a;

	if(HasLighting == 0){ lightMap = diffuse; }
	if(HasHDR == 1){ lightMap = vec4(hdr,1.0); }
	if(HasSSAO == 1){ 
		float brightness = dot(lightMap.xyz, vec3(0.2126, 0.7152, 0.0722));

		lightMap *= min(1.0,ssao*(brightness+0.65)); //this minimizes the ssao effect on very brightly light areas
	}
	if(HasBloom == 1){ lightMap += (vec4(bloom,0.0) / gamma); }

	if(normals.r > 0.999 && normals.g > 0.999 && normals.b > 0.999){
		gl_FragColor = diffuse;
	}
	else{
		gl_FragColor = lightMap;
	}
	gl_FragColor += vec4(rays,1.0);
}