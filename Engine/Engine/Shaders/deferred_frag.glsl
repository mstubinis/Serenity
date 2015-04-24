#version 120

uniform sampler2D DiffuseMap;
uniform sampler2D NormalMap;
uniform sampler2D GlowMap;

uniform int Shadeless;
uniform float far;
uniform float C;

uniform int DiffuseMapEnabled;
uniform int NormalMapEnabled;
uniform int GlowMapEnabled;

uniform vec4 Object_Color;

varying vec3 WorldPosition;
varying vec2 UV;
varying vec3 Normals; 
varying vec3 Binormals;
varying vec3 Tangents;

vec3 CalcBumpedNormal(){
    vec3 normalMapTexture = (texture2D(NormalMap, UV).xyz) * 2.0 - 1.0;
    mat3 TBN = mat3(-Tangents, Binormals, Normals);
    return normalize(TBN * normalMapTexture);
}
void main(){
	if(DiffuseMapEnabled == 1)
		gl_FragData[0] = texture2D(DiffuseMap, UV) * Object_Color;
	else
		gl_FragData[0] = Object_Color;

	if(Shadeless == 0){
		if(NormalMapEnabled == 1)
			gl_FragData[1].rgb = normalize(CalcBumpedNormal());
		else
			gl_FragData[1].rgb = normalize(Normals);

		if(GlowMapEnabled == 1)
			gl_FragData[1].a = texture2D(GlowMap, UV).r;
		else
			gl_FragData[1].a = 0.0;
	}
	else{
		gl_FragData[1] = vec4(1.0);
	}
	gl_FragData[2] = vec4(WorldPosition,1.0);
    const float offset = 1.0;
    gl_FragDepth = (log(C * gl_TexCoord[6].z + offset) / log(C * far + offset));
}