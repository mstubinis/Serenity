#version 120

varying vec2 offset[28];
varying float Radius;
uniform float radius;

uniform float H;
uniform float V;

float weights[7] = float[](
	0.028,
	0.024,
	0.020,
	0.016,
	0.012,
	0.008,
	0.004
);
void main(){
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	for(int i = 0; i < 7; i++){
		offset[i] = vec2(-weights[i] * radius * H, -weights[i] * radius * V);
		offset[13-i] = vec2(weights[i] * radius * H, weights[i] * radius * V);
	}
	Radius = radius;
}