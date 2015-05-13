#version 120

uniform sampler2D texture;
uniform float strengthModifier;
uniform vec2 gScreenSize;

uniform float R;
uniform float G;
uniform float B;
uniform float A;

varying vec2 offset[14];
varying float Radius;

vec2 CalcTexCoord(){return gl_FragCoord.xy / gScreenSize;}
void main(void){
    vec4 sum = vec4(0.0);
    vec2 uv = CalcTexCoord();
    vec4 color = texture2D(texture, uv );

	float strength = max(1.0, Radius * strengthModifier);

	sum += texture2D(texture, uv+offset[ 0])*0.0044 * strength;
	sum += texture2D(texture, uv+offset[ 1])*0.009 * strength;
	sum += texture2D(texture, uv+offset[ 2])*0.0216 * strength;
	sum += texture2D(texture, uv+offset[ 3])*0.044 * strength;
	sum += texture2D(texture, uv+offset[ 4])*0.078 * strength;
	sum += texture2D(texture, uv+offset[ 5])*0.116 * strength;
	sum += texture2D(texture, uv+offset[ 6])*0.147 * strength;
	sum += texture2D(texture, uv )*0.16 * strength;
	sum += texture2D(texture, uv+offset[ 7])*0.147 * strength;
	sum += texture2D(texture, uv+offset[ 8])*0.116 * strength;
	sum += texture2D(texture, uv+offset[ 9])*0.078 * strength;
	sum += texture2D(texture, uv+offset[10])*0.044 * strength;
	sum += texture2D(texture, uv+offset[11])*0.0216 * strength;
	sum += texture2D(texture, uv+offset[12])*0.009 * strength;
	sum += texture2D(texture, uv+offset[13])*0.0044 * strength;

	if(R == 1.0)
		gl_FragColor.r = sum.r;
	if(G == 1.0)
		gl_FragColor.g = sum.g;
	if(B == 1.0)
		gl_FragColor.b = sum.b;
	if(A == 1.0)
		gl_FragColor.a = sum.a;
}