#version 120

uniform sampler2D texture;

uniform int R;
uniform int G;
uniform int B;
uniform int A;

uniform vec2 HV;

vec2 offset[14];

uniform float radius;
uniform vec4 strengthModifier;

float weights[7] = float[](0.028,0.024,0.020,0.016,0.012,0.008,0.004);

float gauss[7] = float[](
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
        offset[i] = vec2(-weights[i] * radius * HV.x, -weights[i] * radius * HV.y);
        offset[13-i] = vec2(weights[i] * radius * HV.x, weights[i] * radius * HV.y);
    }
    vec4 sum = vec4(0.0); vec2 uv = gl_TexCoord[0].st * 2.0;

    float strengthR = max(1.0, radius * strengthModifier.r);
	float strengthG = max(1.0, radius * strengthModifier.g);
	float strengthB = max(1.0, radius * strengthModifier.b);
	float strengthA = max(1.0, radius * strengthModifier.a);
	vec4 strength = vec4(strengthR,strengthG,strengthB,strengthA);
    for(int i = 0; i < 7; i++){
        sum += texture2D(texture, uv + offset[i])    * gauss[i] * strength;
        sum += texture2D(texture, uv + offset[13-i]) * gauss[i] * strength;
    }
    sum.r += texture2D(texture, uv ).r * 0.159576912161 * strengthR;
	sum.g += texture2D(texture, uv ).g * 0.159576912161 * strengthG;
	sum.b += texture2D(texture, uv ).b * 0.159576912161 * strengthB;
	sum.a += texture2D(texture, uv ).a * 0.159576912161 * strengthA;

    if(R == 1) gl_FragColor.r = sum.r;
    if(G == 1) gl_FragColor.g = sum.g;
    if(B == 1) gl_FragColor.b = sum.b;
    if(A == 1) gl_FragColor.a = sum.a;
}