#version 120

uniform samplerCube cubemap;

varying vec3 TexCoord0;
varying vec3 WorldPosition;

void main(){
    gl_FragData[0] = textureCube(cubemap, TexCoord0);
    gl_FragData[1] = vec4(1.0);
	gl_FragData[2].r = 0.0;
	gl_FragData[3] = vec4(WorldPosition,1.0);
	gl_FragDepth = 0.99999f;
}