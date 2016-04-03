#version 120

attribute vec3 position;
attribute vec2 uv;
attribute vec3 normal;
attribute vec3 tangent;
attribute vec3 binormal;

uniform mat4 VP;
uniform mat4 World;

varying vec2 UV;

void main(){
	mat4 MVP = VP * World;
	UV = uv;
	gl_Position = MVP * vec4(position, 1.0);
	gl_TexCoord[6] = gl_Position;
}