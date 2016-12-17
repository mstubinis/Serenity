#version 120

uniform sampler2D DiffuseTexture;

uniform sampler2D NormalTexture;

uniform sampler2D GlowTexture;

uniform sampler2D SpecularTexture;

uniform samplerCube ReflectionTexture; //the cubemap texture
uniform sampler2D ReflectionTextureMap;
uniform float ReflectionMixFactor;

uniform int Shadeless;
uniform float far;
uniform float C;
uniform float BaseGlow;
uniform float matID;

uniform int DiffuseTextureEnabled;
uniform int NormalTextureEnabled;
uniform int GlowTextureEnabled;
uniform int SpecularTextureEnabled;
uniform int ReflectionTextureEnabled;

uniform vec4 Object_Color;
uniform vec3 Gods_Rays_Color;
uniform vec3 CameraPosition;

varying vec3 WorldPosition;
varying vec2 UV;
varying vec3 Normals; 
varying vec3 Binormals;
varying vec3 Tangents;

uniform int HasGodsRays;

vec3 CalcBumpedNormal(){
    vec3 t = ((texture2D(NormalTexture, UV).xyz) * 2.0) - 1.0;
    mat3 TBN = mat3(Tangents, Binormals, Normals);
    return normalize(TBN * t);
}
void main(){

    vec3 normals = normalize(Normals);
    if(NormalTextureEnabled == 1){
        normals = CalcBumpedNormal();
    }


    if(DiffuseTextureEnabled == 1){
        gl_FragData[0] = texture2D(DiffuseTexture, UV) * Object_Color;
    }
    else{
        gl_FragData[0] = Object_Color;
    }
	if(ReflectionTextureEnabled == 1){
	    vec3 reflectionVector = normalize(reflect(normals,CameraPosition - WorldPosition));
	    vec3 reflectionColor = textureCube(ReflectionTexture,reflectionVector).rgb * vec3(texture2D(ReflectionTextureMap,UV).r);
		gl_FragData[0].rgb += reflectionColor * 1.0;
	}
    if(Shadeless == 0){
        gl_FragData[1].rgb = normals;
        if(GlowTextureEnabled == 1){
            gl_FragData[2].r = texture2D(GlowTexture, UV).r + BaseGlow;
        }
        else{
            gl_FragData[2].r = BaseGlow;
		}
		if(SpecularTextureEnabled == 1){
			gl_FragData[2].g = texture2D(SpecularTexture, UV).r;
		}
		else{
			gl_FragData[2].g = 1.0;
		}
    }
    else{
        gl_FragData[1].rgb = vec3(1.0);
        gl_FragData[2].r = BaseGlow;
		gl_FragData[2].g = 1.0;
    }
    gl_FragData[1].a = Object_Color.a;
    gl_FragData[2].b = matID;
    gl_FragData[3] = vec4(WorldPosition,1.0);
	if(HasGodsRays == 1){
		gl_FragData[4] = (texture2D(DiffuseTexture, UV) * vec4(Gods_Rays_Color,1.0))*0.5;
	}
    gl_FragDepth = (log(C * gl_TexCoord[6].z + 1.0) / log(C * far + 1.0));
}