#version 120

uniform sampler2D gDiffuseMap;
uniform sampler2D gLightMap;
uniform sampler2D gMiscMap;
uniform sampler2D gGodsRaysMap;
uniform sampler2D gBloomMap;

uniform int HasSSAO;
uniform int HasLighting;
uniform int HasHDR;

uniform float gamma;

void main(void){
    vec2 uv = gl_TexCoord[0].st;
    vec3 diffuse = texture2D(gDiffuseMap, uv).rgb;
    vec3 rays = texture2D(gGodsRaysMap,uv).rgb;
    vec3 hdr = texture2D(gMiscMap,uv).rgb;
    vec3 lighting = texture2D(gLightMap, uv).rgb;

    if(HasLighting == 0){ 
        lighting = diffuse;
    }
	else{
	    lighting = hdr;
	}
    if(HasSSAO == 1){ 
        float brightness = dot(lighting, vec3(0.2126, 0.7152, 0.0722));
        float ssao = texture2D(gBloomMap,uv).a + 0.0001;
		brightness = min(1.0,pow(brightness,0.125));
        lighting *= max(brightness, ssao);
    }
	lighting += rays;
	//lighting = pow(lighting, vec3(1.0 / gamma));
    gl_FragColor = vec4(lighting,1.0);
}
