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
    vec3 normalMapTexture = (texture(NormalMap, UV).xyz) * 2.0 - 1.0;
    mat3 TBN = mat3(-Tangents, Binormals, Normals);
    return normalize(TBN * normalMapTexture);
}
void main(){
	if(DiffuseMapEnabled == 1)
		gl_FragData[0] = texture(DiffuseMap, UV) * vec4(Object_Color,1);
	else
		gl_FragData[0] = vec4(Object_Color,1);

	if(Shadeless == 0){
		if(NormalMapEnabled == 1)
			gl_FragData[1].rgb = CalcBumpedNormal();
		else
			gl_FragData[1].rgb = normalize(Normals);

		if(GlowMapEnabled == 1)
			gl_FragData[1].a = texture(GlowMap, UV).r;
		else
			gl_FragData[1].a = 0;
	}
	else{
		gl_FragData[1] = vec4(1);
	}
}