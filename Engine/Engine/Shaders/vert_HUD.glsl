#version 120

attribute vec3 position;
attribute vec2 uv;
attribute vec3 normal;
attribute vec3 tangent;
attribute vec3 binormal;

uniform mat4 MVP;
uniform mat4 World;

varying vec2 UV;

varying vec3 WorldPosition;

void main(){
	UV = uv;
	gl_Position = MVP * vec4(position, 1.0);
	gl_TexCoord[6] = gl_Position;
	WorldPosition = (World * vec4(position,1.0)).xyz;
}