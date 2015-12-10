#version 120

uniform float fExposure;
uniform float far;
uniform float C;
uniform float BaseGlow;

uniform sampler2D DiffuseMap;
uniform sampler2D NormalMap;
uniform sampler2D GlowMap;

uniform int DiffuseMapEnabled;
uniform int NormalMapEnabled;
uniform int GlowMapEnabled;

uniform int HasAtmosphere;

uniform vec4 Object_Color;
uniform vec3 gAmbientColor;

varying vec3 c0;
varying vec3 c1;

varying vec3 WorldPosition;
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
	if(HasAtmosphere == 1){
		if(DiffuseMapEnabled == 1){
			vec4 diffuse = texture2D(DiffuseMap, UV) * Object_Color;
			gl_FragData[0].rgb = max(gAmbientColor*diffuse.xyz,(1.0 - exp( -fExposure * ((c0+diffuse.xyz) * c1) )));
			gl_FragData[0].a = 1.0;
			if(GlowMapEnabled == 1){
				gl_FragData[0].rgb = max(gl_FragData[0].rgb, (vec3(1.0)-gl_FragData[0].rgb)*texture2D(GlowMap, UV).rgb );
			}
		}
		else{
			gl_FragData[0] = vec4(0.0);
		}
		gl_FragData[1] = vec4(1.0);
		gl_FragData[2].r = 0.0;
	}
	else{
		if(DiffuseMapEnabled == 1)
			gl_FragData[0] = texture2D(DiffuseMap, UV) * Object_Color;
		else
			gl_FragData[0] = vec4(0.0);

		if(NormalMapEnabled == 1){
			gl_FragData[1].rgb = normalize(CalcBumpedNormal());
			gl_FragData[1].a = texture2D(DiffuseMap, UV).a;
		}
		else{
			gl_FragData[1].rgb = normalize(Normals);
			gl_FragData[1].a = texture2D(DiffuseMap, UV).a;
		}

		if(GlowMapEnabled == 1)
			gl_FragData[2].r = texture2D(GlowMap, UV).r + BaseGlow;
		else
			gl_FragData[2].r = BaseGlow;
	}
	gl_FragData[3] = vec4(WorldPosition,1.0);
    const float offset = 1.0;
    gl_FragDepth = (log(C * gl_TexCoord[6].z + offset) / log(C * far + offset));
}