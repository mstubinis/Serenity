#version 120

uniform sampler2D gColorMap;
uniform sampler2D gLightMap;
uniform sampler2D gBloomMap;
uniform sampler2D gNormalMap;

uniform int HasLighting;
uniform int HasBloom;

uniform vec2 gScreenSize;
uniform vec3 gAmbientColor;

void main(){
    vec2 uv = gl_FragCoord.xy / gScreenSize;
    vec4 image = texture2D(gColorMap, uv);
	vec3 normals = texture2D(gNormalMap,uv).rgb;
    vec4 lightMap = texture2D(gLightMap, uv);
    vec4 bloom = texture2D(gBloomMap,uv);

	vec4 calc = vec4(0); //calc will emulate image * glow. replace calc with image * glow if desired
	if(distance(lightMap,image) < 0.0008){
		calc = image;
	}
	if(normals.r > 0.999 && normals.g > 0.999 && normals.b > 0.999){
		if(HasBloom == 1){
			image += bloom;
		}
		gl_FragColor = image;
	}
	else{
		lightMap = max(vec4(gAmbientColor,1.0),lightMap);
		if(HasLighting == 0){
			lightMap = image;
		}
		vec4 illumination = vec4(0);
		if(HasBloom == 1){
			illumination = calc;
			illumination = max((bloom*(1.0-(lightMap/image))), illumination);
		}
		gl_FragColor = illumination + lightMap;
	}
}