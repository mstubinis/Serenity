#version 120

attribute vec3 Position;
uniform mat4 VP;
uniform mat4 World;

varying vec3 TexCoord0;
varying vec3 WorldPosition;

void main(){
	mat4 MVP = VP * World;
    gl_Position = MVP * vec4(Position, 1.0);
	gl_Position.z = gl_Position.w;
    TexCoord0 = Position;
	WorldPosition = (World * vec4(Position,1.0)).xyz;
}