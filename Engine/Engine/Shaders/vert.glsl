#version 130

in vec3 position;
in vec2 uv;
in vec3 normal;
in vec3 binormal;
in vec3 tangent;
in vec4 color;

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
	Binormals = (World * vec4(binormal,0.0)).xyz;
	Tangents = (World * vec4(tangent,0.0)).xyz;
}