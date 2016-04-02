#version 120

uniform sampler2D gColorMap;
uniform sampler2D gLightMap;
uniform sampler2D gNormalMap;
uniform sampler2D gGlowMap;
uniform sampler2D gBloomMap;

uniform int HasLighting;
uniform int HasBloom;

uniform vec2 gScreenSize;
uniform vec3 gAmbientColor;

void main(){
	vec2 texCoords = gl_FragCoord.xy / gScreenSize;
	vec4 image = texture2D(gColorMap, texCoords);
	vec4 lightMap = texture2D(gLightMap, texCoords);
	vec4 normals = texture2D(gNormalMap,texCoords);
	float glow = texture2D(gGlowMap, texCoords).r;
	vec4 bloom = texture2D(gBloomMap,texCoords);

	if(normals.r > 0.9999 && normals.g > 0.9999 && normals.b > 0.9999){
		if(HasBloom == 1){
			image += bloom;
		}
		gl_FragColor = image;
	}
	else{
		vec4 step2 = max(vec4(glow*image),lightMap);
		lightMap = max(vec4(gAmbientColor,1.0),step2);
		if(HasLighting == 0){
			lightMap = image;
		}
		vec4 illumination = glow*image;
		if(HasBloom == 1){
			illumination = max((bloom*(1.0-(lightMap/image))), illumination);
		}
		gl_FragColor = lightMap+illumination;
		//gl_FragColor = image;
	}
}