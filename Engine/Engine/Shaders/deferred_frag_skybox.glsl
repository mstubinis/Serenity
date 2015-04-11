#version 130

uniform samplerCube cubemap;

varying vec3 TexCoord0;

void main(){
    gl_FragData[0] = texture(cubemap, TexCoord0);
    gl_FragData[1] = vec4(1);
	gl_FragData[2] = vec4(1);
}