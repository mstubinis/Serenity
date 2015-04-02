#version 330

uniform samplerCube cubemap;

in vec3 TexCoord0;
in vec3 WP;

layout (location = 0) out vec4 DiffuseOut; 
layout (location = 1) out vec4 NormalOut; 
layout (location = 2) out vec4 GlowOut;

void main(){
    DiffuseOut = texture(cubemap, TexCoord0);
    NormalOut = vec4(1);
	GlowOut = vec4(1);
}