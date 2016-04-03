#version 120

uniform vec2 gScreenSize;
uniform sampler2D texture;
uniform sampler2D texture1;

void main(){
	vec2 uv = gl_FragCoord.xy / gScreenSize;
	float glow = texture2D(texture,uv).r;
	vec3 diffuse = texture2D(texture1,uv).rgb;
	gl_FragColor.rgb = vec3(glow) * diffuse;
}