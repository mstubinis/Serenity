#version 130

uniform float fExposure;

uniform sampler2D DiffuseMap;
uniform sampler2D NormalMap;
uniform sampler2D GlowMap;

uniform int DiffuseMapEnabled;
uniform int NormalMapEnabled;
uniform int GlowMapEnabled;

uniform int HasAtmosphere;

uniform vec3 Object_Color;

varying vec3 c0;
varying vec3 c1;

varying vec4 Color;
varying vec2 UV;
varying vec3 Normals; 
varying vec3 Binormals;
varying vec3 Tangents;

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
			gl_FragData[0].rgb = 1.0 - exp( -fExposure * ((c0+diffuse.xyz) * c1) );
			gl_FragData[0].a = 1.0;
		}
		else{
			gl_FragData[0] = vec4(0);
		}
		gl_FragData[1] = vec4(1);
		gl_FragData[2] = vec4(1);
	}
	else{
		if(DiffuseMapEnabled == 1)
			gl_FragData[0] = texture(DiffuseMap, UV) * vec4(Object_Color.xyz,1);
		else
			gl_FragData[0] = vec4(0);

		if(NormalMapEnabled == 1)
			gl_FragData[1] = vec4(CalcBumpedNormal().xyz,0);
		else
			gl_FragData[1] = vec4(normalize(Normals).xyz,0);

		if(GlowMapEnabled == 1)
			gl_FragData[2].r = texture(GlowMap, UV).r;
		else
			gl_FragData[2].r = 0;
	}
}