#version 110

varying vec3 TexCoord0;
uniform samplerCube cubemap;

void main(){
    gl_FragColor = texture(cubemap, TexCoord0);
}