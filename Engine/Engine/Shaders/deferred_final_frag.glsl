#version 120

uniform sampler2D gColorMap;
uniform sampler2D gLightMap;
uniform sampler2D gNormalMap;
uniform sampler2D gGlowMap;
uniform sampler2D gBloomMap;

uniform int HasSSAO;
uniform int HasBloom;

uniform vec2 gScreenSize;
uniform vec3 gAmbientColor;

vec2 CalcTexCoord(){ return gl_FragCoord.xy / gScreenSize; }

void main(){
	vec2 texCoords = CalcTexCoord();
	vec4 image = texture2D(gColorMap, texCoords);
	vec4 lighting = texture2D(gLightMap, texCoords);
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

		if(HasSSAO == 1){
			float ssao = texture2D(gGlowMap, texCoords).g;
			lighting *= ssao;
		}
		vec4 step2 = max(vec4(glow),lighting);
		vec4 light = max(vec4(gAmbientColor,1),step2);

		//make the specular part of the lighting "stand out"
		float specLight = pow(lighting.r,8);

		vec4 imageLight = (image * light)+(specLight*0.5);

		vec4 illumination = glow*image;
		if(HasBloom == 1){
			illumination = max((bloom*(1.0-light)), illumination);
		}
		gl_FragColor = imageLight+illumination;
	}
}