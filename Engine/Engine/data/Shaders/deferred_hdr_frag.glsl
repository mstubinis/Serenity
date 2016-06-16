#version 120

uniform float exposure;
uniform float gamma;

uniform sampler2D lightingBuffer;
uniform sampler2D bloomBuffer;

uniform vec2 gScreenSize;

uniform int HasBloom;

void main(void){
    vec2 uv = gl_FragCoord.xy / gScreenSize;
	vec3 lighting = texture2D(lightingBuffer, uv).rgb;

	if(HasBloom == 1){
		vec3 bloomColor = texture2D(bloomBuffer, uv).rgb;
		lighting += bloomColor / gamma; // additive blending. normally divided by gamma is not used but i thought it looked better
	}

    vec3 mapped = vec3(1.0) - exp(-lighting * exposure); // Exposure tone mapping
    mapped = pow(mapped, vec3(1.0 / gamma));             // Gamma correction 

    gl_FragColor = vec4(mapped, 1.0);
}