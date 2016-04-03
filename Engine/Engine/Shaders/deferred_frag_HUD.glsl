#version 120

uniform sampler2D DiffuseMap;
uniform int DiffuseMapEnabled;

uniform vec4 Object_Color;

varying vec2 UV;

uniform float far;
uniform float C;

void main(){
	if(DiffuseMapEnabled == 1.0){
		gl_FragData[0] = texture2D(DiffuseMap, UV) * Object_Color;
	}
	else{
		gl_FragData[0] = Object_Color;
	}
    gl_FragData[1].rgb = vec3(1.0);
	gl_FragData[2].r = 0.0;
	gl_FragData[2].b = 0.0;
	gl_FragData[3].r = gl_FragCoord.z;
}