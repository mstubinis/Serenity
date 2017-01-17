#version 120

attribute vec3 position;
attribute vec2 uv;
attribute vec3 normal;
attribute vec3 binormal;
attribute vec3 tangent;

uniform mat4 VP;
uniform mat4 Model;

varying vec2 UV;

varying vec3 Normals;
varying vec3 Binormals;
varying vec3 Tangents;
varying vec3 WorldPosition;

varying float logz_f;
varying float FC_2_f;
uniform float fcoeff;

void main(void){
    mat4 MVP = VP * Model;
    gl_Position = MVP * vec4(position, 1.0);
    
    UV = uv;

    Normals = (Model * vec4(normal,0.0)).xyz; 
    Binormals = (Model * vec4(binormal,0.0)).xyz;
    Tangents = (Model * vec4(tangent,0.0)).xyz;

	WorldPosition = (Model * vec4(position, 1.0)).xyz;

    logz_f = 1.0 + gl_Position.w;
	gl_Position.z = (log2(max(1e-6, logz_f)) * fcoeff - 1.0) * gl_Position.w;
    FC_2_f = fcoeff * 0.5;
}