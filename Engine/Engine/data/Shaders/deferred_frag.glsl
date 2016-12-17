#version 120

uniform sampler2D DiffuseTexture;
uniform sampler2D NormalTexture;
uniform sampler2D GlowTexture;
uniform sampler2D SpecularTexture;

uniform samplerCube ReflectionTexture;
uniform sampler2D   ReflectionTextureMap;
uniform float       CubemapMixFactor;
uniform samplerCube RefractionTexture;
uniform sampler2D   RefractionTextureMap;
uniform float       RefractionRatio;

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
uniform int RefractionTextureEnabled;

uniform vec4 Object_Color;
uniform vec3 Gods_Rays_Color;
uniform vec3 CameraPosition;

varying vec3 WorldPosition;
varying vec2 UV;
varying vec3 Normals; 
varying vec3 Binormals;
varying vec3 Tangents;

uniform int HasGodsRays;

vec4 PaintersAlgorithm(vec4 top, vec4 bottom){
	vec4 ret = vec4(0.0);
	float _a = top.a + bottom.a * (1.0 - top.a);
	ret.r = ((top.r * top.a + bottom.r * bottom.a * (1.0-top.a)) / _a);
    ret.g = ((top.g * top.a + bottom.g * bottom.a * (1.0-top.a)) / _a);
    ret.b = ((top.b * top.a + bottom.b * bottom.a * (1.0-top.a)) / _a);
	ret.a = _a;
	return ret;
}

vec3 CalcBumpedNormal(){
    vec3 t = (texture2D(NormalTexture, UV).xyz * 2.0) - 1.0;
    mat3 TBN = mat3(Tangents, Binormals, Normals);
    return normalize(TBN * t);
}
void main(void){
    vec3 normals = normalize(Normals);
	vec4 diffuseColor = Object_Color;
	if(DiffuseTextureEnabled == 1){
		diffuseColor *= texture2D(DiffuseTexture, UV);
	}
    if(NormalTextureEnabled == 1){
        normals = CalcBumpedNormal();
    }
	if(ReflectionTextureEnabled == 1){
	    vec4 color2 = textureCube(ReflectionTexture,normalize(reflect(normals,CameraPosition - WorldPosition))) * texture2D(ReflectionTextureMap,UV).r;

		color2.a *= CubemapMixFactor;
		color2 = PaintersAlgorithm(color2,diffuseColor);

		diffuseColor += color2;
	}
	else{
	    diffuseColor = vec4(1.0);
	}
	if(RefractionTextureEnabled == 1){
	    vec4 color1 = textureCube(RefractionTexture,normalize(refract(normals,CameraPosition - WorldPosition,RefractionRatio))) * texture2D(RefractionTextureMap,UV).r;

		color1.a *= CubemapMixFactor;
		color1 = PaintersAlgorithm(color1,diffuseColor);

		diffuseColor = color1;
	}
	gl_FragData[0] = diffuseColor;
    if(Shadeless == 0){
        gl_FragData[1].rgb = normals;
		gl_FragData[2].r = BaseGlow;
        if(GlowTextureEnabled == 1){
            gl_FragData[2].r += texture2D(GlowTexture, UV).r;
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