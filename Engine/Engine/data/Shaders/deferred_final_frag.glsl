#version 120

uniform sampler2D gDiffuseMap;
uniform sampler2D gLightMap;
uniform sampler2D gBloomMap;
uniform sampler2D gMiscMap;
uniform sampler2D gGodsRaysMap;

uniform int HasSSAO;
uniform int HasLighting;
uniform int HasBloom;
uniform int HasHDR;

uniform vec3 gAmbientColor;

void main(void){
    vec2 uv = gl_TexCoord[0].st;
    vec4 diffuse = texture2D(gDiffuseMap, uv);
    vec4 bloom = texture2D(gBloomMap,uv);
    vec3 rays = texture2D(gGodsRaysMap,uv).rgb;
    vec4 hdr = max(vec4(gAmbientColor,1.0),texture2D(gMiscMap,uv));
    vec4 lighting = max(vec4(gAmbientColor,1.0),texture2D(gLightMap, uv));

    if(HasLighting == 0){ 
        lighting = diffuse; 
    }
	else{
	    lighting = hdr;
	}
    if(HasSSAO == 1){ 
        float brightness = dot(lighting.xyz, vec3(0.2126, 0.7152, 0.0722));
        float ssao = texture2D(gBloomMap,uv).a;
        lighting *= min(1.0,ssao*(brightness+0.65)); //this minimizes the ssao effect on very brightly light areas
    }
    if(HasBloom == 1){
        lighting += bloom; 
    }
    gl_FragColor = lighting;

    gl_FragColor += vec4(rays,1.0);
}
