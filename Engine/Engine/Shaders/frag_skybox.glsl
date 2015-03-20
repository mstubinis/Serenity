#version 330

in vec3 TexCoord0;

uniform samplerCube cubemap;

void main(){
    gl_FragColor = texture(cubemap, TexCoord0);
}