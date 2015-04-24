#version 120

uniform sampler2D DiffuseMap;
uniform int DiffuseMapEnabled;

uniform vec4 Object_Color;

varying vec3 WorldPosition;
varying vec2 UV;

void main(){
	if(DiffuseMapEnabled == 1.0){
		gl_FragData[0] = texture2D(DiffuseMap, UV) * Object_Color;
	}
	else{
		gl_FragData[0] = Object_Color;
	}
	gl_FragData[1] = vec4(1.0);
	gl_FragData[2] = vec4(WorldPosition,1.0);
}