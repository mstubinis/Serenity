#version 120

uniform mat4 VP;
uniform mat4 Model;

void main(){
    mat4 MVP = VP * Model;
    gl_Position = MVP * gl_Vertex;
}