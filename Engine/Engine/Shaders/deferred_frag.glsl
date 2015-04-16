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

uniform vec3 Object_Color;

varying vec4 Color;
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
		gl_FragData[0] = texture2D(DiffuseMap, UV) * vec4(Object_Color,1.0);
	else
		gl_FragData[0] = vec4(Object_Color,1.0);

	if(Shadeless == 0){
		if(NormalMapEnabled == 1)
			gl_FragData[1].rgb = CalcBumpedNormal();
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
    const float offset = 1.0;
    gl_FragDepth = (log(C * gl_TexCoord[6].z + offset) / log(C * far + offset));
}