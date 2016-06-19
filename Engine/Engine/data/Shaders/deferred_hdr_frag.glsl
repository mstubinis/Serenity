#version 120

uniform float exposure;
uniform float gamma;

uniform sampler2D lightingBuffer;

uniform vec2 gScreenSize;

void main(){
    vec2 uv = gl_FragCoord.xy / gScreenSize;
	vec3 lighting = texture2D(lightingBuffer, uv).rgb;

    vec3 mapped = vec3(1.0) - exp(-lighting * exposure); // Exposure tone mapping
    mapped = pow(mapped, vec3(1.0 / gamma));             // Gamma correction 

    gl_FragColor = vec4(mapped, 1.0);
}