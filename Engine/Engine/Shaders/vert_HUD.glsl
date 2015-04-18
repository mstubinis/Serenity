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

varying vec3 WorldPosition;


void main(){
	gl_Position = MVP * vec4(position, 1.0);

	Color = color;
	UV = uv;

	WorldPosition = (World * vec4(position,1.0)).xyz;
}