#version 330

uniform float fExposure;

uniform sampler2D DiffuseMap;
uniform sampler2D NormalMap;
uniform sampler2D GlowMap;

uniform int DiffuseMapEnabled;
uniform int NormalMapEnabled;
uniform int GlowMapEnabled;

uniform vec3 Object_Color;

in vec3 c0;
in vec3 c1;

in vec4 Color;
in vec2 UV;

layout(location=0)out vec4 DiffuseOut; 
layout(location=1)out vec4 NormalOut; 
layout(location=2)out vec4 GlowOut; 

void main(){
	if(DiffuseMapEnabled == 1){
		vec4 diffuse = texture(DiffuseMap, UV) * vec4(Object_Color.xyz,1);
		DiffuseOut.rgb = 1.0 - exp( -fExposure * ((c0+diffuse.xyz) * c1) );
		DiffuseOut.a = 1.0;
	}
	else{
		DiffuseOut = vec4(0,0,0,0);
	}

	NormalOut = vec4(1,1,1,1);
	GlowOut = vec4(1,1,1,1);
}