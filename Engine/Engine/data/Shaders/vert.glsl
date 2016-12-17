#version 120

attribute vec3 position;
attribute vec2 uv;
attribute vec3 normal;
attribute vec3 tangent;
attribute vec3 binormal;

uniform mat4 VP;
uniform mat4 Model;

varying vec2 UV;

varying vec3 WorldPosition;
varying vec3 Normals;
varying vec3 Binormals;
varying vec3 Tangents;

void main(void){
    mat4 MVP = VP * Model;
    gl_Position = MVP * vec4(position, 1.0);
    gl_TexCoord[6] = gl_Position;

    UV = uv;

    Normals = (Model * vec4(normal,0.0)).xyz; 
    Binormals = (Model * vec4(binormal,0.0)).xyz;
	Tangents = (Model * vec4(tangent,0.0)).xyz;

    WorldPosition = (Model * vec4(position,1.0)).xyz;
}