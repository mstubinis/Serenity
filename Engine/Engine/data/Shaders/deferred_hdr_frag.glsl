#version 120

uniform float exposure;
uniform float gamma;
uniform sampler2D hdrBuffer;
uniform sampler2D bloomBuffer;
uniform vec2 gScreenSize;

void main(void){
    vec2 uv = gl_FragCoord.xy / gScreenSize;
	vec3 hdrColor = texture2D(hdrBuffer, uv).rgb;
    vec3 bloomColor = texture2D(bloomBuffer, uv).rgb;
    hdrColor += bloomColor; // additive blending

    // Exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
    // Gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));
    gl_FragColor = vec4(mapped, 1.0);
}