#version 120

uniform sampler2D texture;
uniform float strengthModifier;
uniform vec2 gScreenSize;

uniform int R;
uniform int G;
uniform int B;
uniform int A;

varying vec2 offset[28];
varying float Radius;

float g[7] = float[](
	0.0044299121055113265,
	0.00895781211794,
	0.0215963866053,
	0.0443683338718,
	0.0776744219933,
	0.115876621105,
	0.147308056121
);
void main(void){
    vec4 sum = vec4(0.0);
    vec2 uv = gl_FragCoord.xy / gScreenSize;
    vec4 color = texture2D(texture, uv );

	float strength = max(1.0, Radius * strengthModifier);

	for(int i = 0; i < 7; i++){
		sum += texture2D(texture, uv + offset[i])    *g[i] * strength;
		sum += texture2D(texture, uv + offset[13-i]) *g[i] * strength;
	}
	sum += texture2D(texture, uv ) * 0.159576912161 * strength;

	if(R == 1) gl_FragColor.r = sum.r;
	if(G == 1) gl_FragColor.g = sum.g;
	if(B == 1) gl_FragColor.b = sum.b;
	if(A == 1) gl_FragColor.a = sum.a;
}