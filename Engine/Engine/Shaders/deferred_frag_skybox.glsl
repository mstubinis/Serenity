#version 120

uniform samplerCube cubemap;

varying vec3 TexCoord0;

void main(){
    gl_FragData[0] = textureCube(cubemap, TexCoord0);
    gl_FragData[1] = vec4(1.0);
}