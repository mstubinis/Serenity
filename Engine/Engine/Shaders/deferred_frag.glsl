#version 130

uniform sampler2D DiffuseMap;
uniform sampler2D NormalMap;
uniform sampler2D GlowMap;

uniform int Shadeless;

uniform int DiffuseMapEnabled;
uniform int NormalMapEnabled;
uniform int GlowMapEnabled;

uniform vec3 Object_Color;

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
	if(DiffuseMapEnabled == 1)
		gl_FragData[0] = texture(DiffuseMap, UV) * vec4(Object_Color,1);
	else
		gl_FragData[0] = vec4(Object_Color,1);

	if(Shadeless == 0){
		if(NormalMapEnabled == 1)
			gl_FragData[1] = vec4(CalcBumpedNormal().xyz,0);
		else
			gl_FragData[1] = vec4(normalize(Normals).xyz,0);

		if(GlowMapEnabled == 1)
			gl_FragData[2].r = texture(GlowMap, UV).r;
		else
			gl_FragData[2] = vec4(0);
	}
	else{
		gl_FragData[1] = vec4(1);
		gl_FragData[2] = vec4(1);
	}
}