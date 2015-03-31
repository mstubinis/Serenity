#version 330

uniform float fExposure;

uniform sampler2D DiffuseMap;
uniform sampler2D NormalMap;
uniform sampler2D GlowMap;

uniform int DiffuseMapEnabled;
uniform int NormalMapEnabled;
uniform int GlowMapEnabled;

uniform int HasAtmosphere;

uniform vec3 Object_Color;

in vec3 c0;
in vec3 c1;

in vec4 Color;
in vec2 UV;
in vec3 Normals; 
in vec3 Binormals;
in vec3 Tangents;

layout(location=0)out vec4 DiffuseOut; 
layout(location=1)out vec4 NormalOut; 
layout(location=2)out vec4 GlowOut; 

vec3 CalcBumpedNormal(){
    vec3 Norm = normalize(Normals);
    vec3 Tang = normalize(Tangents);
	vec3 Binorm = normalize(Binormals);
    Tang = normalize(Tang - dot(Tang, Norm) * Norm);

    vec3 BumpMapNormal = texture(NormalMap, UV).xyz;
    BumpMapNormal = BumpMapNormal * 2.0 - 1.0;
    mat3 TBN = mat3(Tang, Binorm, Norm);

    vec3 NewNormal = TBN * BumpMapNormal;
    NewNormal = normalize(NewNormal);
    return NewNormal;
}
void main(){
	if(HasAtmosphere == 1){
		if(DiffuseMapEnabled == 1){
			vec4 diffuse = texture(DiffuseMap, UV) * vec4(Object_Color.xyz,1);
			DiffuseOut.rgb = 1.0 - exp( -fExposure * ((c0+diffuse.xyz) * c1) );
			DiffuseOut.a = 1.0;
		}
		else{
			DiffuseOut = vec4(0,0,0,0);
		}
		NormalOut = vec4(1,1,1,1);
		GlowOut = vec4(1,1,1,1);
	}
	else{
		if(DiffuseMapEnabled == 1)
			DiffuseOut = texture(DiffuseMap, UV) * vec4(Object_Color.xyz,1);
		else
			DiffuseOut = vec4(0,0,0,0);

		if(NormalMapEnabled == 1)
			NormalOut = vec4(CalcBumpedNormal().xyz,0);
		else
			NormalOut = vec4(normalize(Normals).xyz,0);

		if(GlowMapEnabled == 1)
			GlowOut.r = texture(GlowMap, UV).r;
		else
			GlowOut.r = 0;
	}
}