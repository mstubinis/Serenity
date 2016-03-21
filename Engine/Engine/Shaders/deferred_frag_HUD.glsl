#version 120

uniform sampler2D DiffuseMap;
uniform int DiffuseMapEnabled;

uniform vec4 Object_Color;

varying vec3 WorldPosition;
varying vec2 UV;

uniform float far;
uniform float C;

void main(){
	if(DiffuseMapEnabled == 1.0){
		gl_FragColor = texture2D(DiffuseMap, UV) * Object_Color;
	}
	else{
		gl_FragColor = Object_Color;
	}
}