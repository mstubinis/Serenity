#version 120

attribute vec3 position;
attribute vec2 uv;
attribute vec3 normal;
attribute vec3 tangent;
attribute vec3 binormal;

uniform mat4 VP;
uniform mat4 World;

varying vec2 UV;

varying vec3 WorldPosition;
varying vec3 Normals;
varying vec3 Binormals;
varying vec3 Tangents;

void main(){
	mat4 MVP = VP * World;
	gl_Position = MVP * vec4(position, 1.0);
	gl_TexCoord[6] = MVP * vec4(position, 1.0);

	UV = uv;

	//normalizing these solved a problem, but might cause more. Be careful here
	Normals = normalize((World * vec4(normal,0.0)).xyz);
	Tangents = normalize((World * vec4(tangent,0.0)).xyz);
	Binormals = normalize((World * vec4(binormal,0.0)).xyz);

	WorldPosition = (World * vec4(position,1.0)).xyz;
}