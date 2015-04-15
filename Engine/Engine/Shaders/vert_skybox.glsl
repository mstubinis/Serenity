#version 120

attribute vec3 Position;
uniform mat4 MVP;

varying vec3 TexCoord0;

void main(){
    gl_Position = MVP * vec4(Position, 1.0);
	gl_Position.z = gl_Position.w - 0.01;
    TexCoord0 = Position;
}