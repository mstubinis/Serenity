#version 110

varying vec2 offset[14];
varying float Radius;
uniform float radius;

void main(){
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_TexCoord[0] = gl_MultiTexCoord0;

    offset[ 0] = vec2(0.0,-0.028 * radius);
    offset[ 1] = vec2(0.0,-0.024 * radius);
    offset[ 2] = vec2(0.0,-0.020 * radius);
    offset[ 3] = vec2(0.0,-0.016 * radius);
    offset[ 4] = vec2(0.0,-0.012 * radius);
    offset[ 5] = vec2(0.0,-0.008 * radius);
    offset[ 6] = vec2(0.0,-0.004 * radius);
    offset[ 7] = vec2(0.0, 0.004 * radius);
    offset[ 8] = vec2(0.0, 0.008 * radius);
    offset[ 9] = vec2(0.0, 0.012 * radius);
    offset[10] = vec2(0.0, 0.016 * radius);
    offset[11] = vec2(0.0, 0.020 * radius);
    offset[12] = vec2(0.0, 0.024 * radius);
    offset[13] = vec2(0.0, 0.028 * radius);

	Radius = radius;
}