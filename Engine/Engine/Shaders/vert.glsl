#version 120

attribute vec3 position;
attribute vec2 uv;
attribute vec3 normal;
attribute vec3 tangent;
attribute vec4 color;

uniform mat4 MVP;
uniform mat4 World;

varying vec4 Color;
varying vec2 UV;

varying vec3 Normals;
varying vec3 Binormals;
varying vec3 Tangents;

void main(){
	gl_Position = MVP * vec4(position, 1.0);

	Color = color;
	UV = uv;

	Normals = (World * vec4(normal,0.0)).xyz;
	Tangents = (World * vec4(tangent,0.0)).xyz;
	Binormals = cross(Normals,Tangents);
}