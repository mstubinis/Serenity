#version 120

uniform float exposure;

uniform sampler2D lightingBuffer;
uniform sampler2D bloomBuffer;
uniform sampler2D gDiffuseMap;
uniform int HasHDR;
uniform int HasBloom;
uniform int HDRAlgorithm;
uniform float gamma;

vec3 uncharted(vec3 x,float a,float b,float c,float d,float e,float f){
    return vec3(((x*(a*x+c*b)+d*e)/(x*(a*x+b)+d*f))-e/f);
}

void main(void){
    vec2 uv = gl_TexCoord[0].st;
	vec3 diffuse = texture2D(gDiffuseMap,uv).rgb;
    vec3 lighting = texture2D(lightingBuffer, uv).rgb;
    vec3 bloom = texture2D(bloomBuffer, uv).rgb;

	lighting *= diffuse;

    if(HasBloom == 1.0){
        lighting += bloom;
    }
    if(HasHDR == 1.0){
        if(HDRAlgorithm == 0.0){ // Reinhard tone mapping
            lighting = lighting / (lighting + vec3(1.0));
        }
        else if(HDRAlgorithm == 1.0){ //Filmic tone mapping
            vec3 x = max(vec3(0), lighting - vec3(0.004));
            lighting = (x * (vec3(6.2) * x + vec3(0.5))) / (x * (vec3(6.2) * x + vec3(1.7)) + vec3(0.06));
        }
        else if(HDRAlgorithm == 2.0){ // Exposure tone mapping
            lighting = vec3(1.0) - exp(-lighting * exposure);
        }
        else if(HDRAlgorithm == 3.0){ // Uncharted tone mapping
            float A = 0.15; float B = 0.5; float C = 0.1; float D = 0.2; float E = 0.02; float F = 0.3; float W = 11.2;
	        lighting = exposure * uncharted(lighting,A,B,C,D,E,F);
	        vec3 white = 1.0 / uncharted( vec3(W),A,B,C,D,E,F );
	        lighting *= white;
        }
    }
    gl_FragColor = vec4(lighting, 1.0);
}
