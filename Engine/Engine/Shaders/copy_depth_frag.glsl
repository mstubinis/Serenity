#version 120

uniform sampler2D gDepthMap;
uniform vec2 gScreenSize;

void main(){
	vec2 uv = gl_FragCoord.xy / gScreenSize;
    gl_FragDepth = texture2D(gDepthMap,uv);
}