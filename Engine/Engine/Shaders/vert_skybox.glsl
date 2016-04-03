#version 120

attribute vec3 Position;
uniform mat4 VP;
uniform mat4 World;

varying vec3 UV;

void main(){
	mat4 MVP = VP * World;
	UV = Position;
	gl_Position = MVP * vec4(Position, 1.0);
    gl_Position.z = gl_Position.w;
}