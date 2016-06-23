#version 120

uniform sampler2D DiffuseMap;
uniform sampler2D NormalMap;
uniform sampler2D GlowMap;

uniform int Shadeless;
uniform float far;
uniform float C;
uniform float BaseGlow;
uniform float matID;

uniform int DiffuseMapEnabled;
uniform int NormalMapEnabled;
uniform int GlowMapEnabled;

uniform vec4 Object_Color;
uniform vec3 Gods_Rays_Color;

varying vec3 WorldPosition;
varying vec2 UV;
varying vec3 Normals; 
varying vec3 Binormals;
varying vec3 Tangents;

uniform int HasGodsRays;

vec3 CalcBumpedNormal(){
    vec3 normalMapTexture = ((texture2D(NormalMap, UV).xyz) * 2.0) - 1.0;
    mat3 TBN = mat3(Tangents, Binormals, Normals);
    return normalize(TBN * normalMapTexture);
}
void main(){
    if(DiffuseMapEnabled == 1){
        gl_FragData[0] = texture2D(DiffuseMap, UV) * Object_Color;
    }
    else{
        gl_FragData[0] = Object_Color;
    }
    if(Shadeless == 0){
        if(NormalMapEnabled == 1){
            gl_FragData[1].rgb = CalcBumpedNormal();
        }
        else{
            gl_FragData[1].rgb = normalize(Normals);
        }
        if(GlowMapEnabled == 1){
            gl_FragData[2].r = texture2D(GlowMap, UV).r + BaseGlow;
        }
        else
            gl_FragData[2].r = BaseGlow;
    }
    else{
        gl_FragData[1].rgb = vec3(1.0);
        gl_FragData[2].r = BaseGlow;
    }
    gl_FragData[1].a = Object_Color.a;
    gl_FragData[2].b = matID;
    gl_FragData[3] = vec4(WorldPosition,1.0);
	if(HasGodsRays == 1){
		gl_FragData[4] = vec4(Gods_Rays_Color.r,Gods_Rays_Color.g,Gods_Rays_Color.b,1.0);
	}
    gl_FragDepth = (log(C * gl_TexCoord[6].z + 1.0) / log(C * far + 1.0));
}