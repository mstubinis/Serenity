#version 120

uniform sampler2D gDepthMap;

void main(void){
	vec2 uv = gl_TexCoord[0].st;
    gl_FragDepth = texture2D(gDepthMap,uv);
}