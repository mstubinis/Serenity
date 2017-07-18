#version 120

uniform float exposure;

uniform sampler2D lightingBuffer;
uniform sampler2D bloomBuffer;
uniform int HasHDR;
uniform int HasBloom;
uniform int HDRAlgorithm;
uniform float gamma;

void main(void){
    vec2 uv = gl_TexCoord[0].st;
    vec3 lighting = texture2D(lightingBuffer, uv).rgb;
    vec3 bloom = texture2D(bloomBuffer, uv).rgb;
    if(HasBloom == 1.0){
        lighting += bloom;
    }
	if(HasHDR == 1.0){
	    if(HDRAlgorithm == 0.0){
		    lighting = lighting / (lighting + vec3(1.0)); // Reinhard tone mapping
		}
		else if(HDRAlgorithm == 1.0){
		    vec3 x = max(vec3(0), lighting - vec3(0.004));
			lighting = (x * (vec3(6.2) * x + vec3(0.5))) / (x * (vec3(6.2) * x + vec3(1.7)) + vec3(0.06)); //Filmic tone mapping
		}
		else if(HDRAlgorithm == 2.0){
		    lighting = vec3(1.0) - exp(-lighting * exposure); // Exposure tone mapping
		}
	}
    gl_FragColor = vec4(lighting, 1.0);
}
