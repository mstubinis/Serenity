#version 120

uniform float exposure;
uniform float decay;
uniform float density;
uniform float weight;
uniform vec2 lightPositionOnScreen;
uniform sampler2D firstPass;
uniform int samples;
uniform vec2 gScreenSize;

void main(){
	vec2 uv = gl_FragCoord.xy / gScreenSize;
    vec2 deltaTextCoord = vec2( uv - (lightPositionOnScreen.xy / gScreenSize) );
    deltaTextCoord *= 1.0 /  float(samples) * density;
    float illuminationDecay = 1.0;
	for(int i=0; i < samples; i++){

		//uv -= deltaTextCoord;

		uv.x -= deltaTextCoord.x;
		uv.y += deltaTextCoord.y;

		vec4 sample = texture2D(firstPass,uv);	
		sample *= illuminationDecay * weight;
		gl_FragColor += sample;
		illuminationDecay *= decay;
	}
	gl_FragColor *= exposure;
}