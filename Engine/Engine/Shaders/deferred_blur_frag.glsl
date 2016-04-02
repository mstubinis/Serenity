#version 120

uniform sampler2D texture;
uniform float strengthModifier;
uniform vec2 gScreenSize;

uniform int R;
uniform int G;
uniform int B;
uniform int A;

uniform float H;
uniform float V;

vec2 offset[14];

uniform float radius;

float weights[7] = float[](
	0.028,
	0.024,
	0.020,
	0.016,
	0.012,
	0.008,
	0.004
);

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


	for(int i = 0; i < 7; i++){
		offset[i] = vec2(-weights[i] * radius * H, -weights[i] * radius * V);
		offset[13-i] = vec2(weights[i] * radius * H, weights[i] * radius * V);
	}



    vec4 sum = vec4(0.0);
    vec2 uv = gl_FragCoord.xy / gScreenSize;
    vec4 color = texture2D(texture, uv );

	float strength = max(1.0, radius * strengthModifier);

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