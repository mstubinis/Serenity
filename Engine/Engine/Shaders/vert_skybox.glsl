#version 120

attribute vec3 Position;
uniform mat4 MVP;

varying vec3 TexCoord0;

void main(){
    gl_Position = MVP * vec4(Position, 1);
	gl_Position.z = gl_Position.w - 0.01f;
    TexCoord0 = Position;
}