#version 120

uniform mat4 VP;
uniform mat4 World;

void main(){
    mat4 MVP = VP * World;
    gl_Position = MVP * gl_Vertex;
}