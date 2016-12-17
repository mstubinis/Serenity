#version 120

uniform sampler2D DiffuseTexture;

uniform int DiffuseTextureEnabled;

uniform vec4 Object_Color;

varying vec3 WorldPosition;
varying vec2 UV;

uniform float far;
uniform float C;

void main(void){
	gl_FragData[0] = Object_Color;
    if(DiffuseTextureEnabled == 1){
        gl_FragData[0] *= texture2D(DiffuseTexture, UV);
    }
    gl_FragData[1] = vec4(1.0);
    gl_FragData[2].r = 0.0;
    gl_FragData[2].b = 0.0;
}