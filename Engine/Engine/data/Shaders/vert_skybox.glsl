#version 120

attribute vec3 position;
uniform mat4 VP;
uniform mat4 Model;

varying vec3 UV;

void main(void){
    mat4 MVP = VP * Model;
    UV = position;
    gl_Position = MVP * vec4(position, 1.0);
    gl_Position.z = gl_Position.w;
}