uniform sampler2D     DiffuseTexture;
uniform sampler2D     NormalTexture;
uniform sampler2D     GlowTexture;
uniform sampler2D     SpecularTexture;
uniform sampler2D     AOTexture;
uniform sampler2D     MetalnessTexture;
uniform sampler2D     SmoothnessTexture;
uniform samplerCube   ReflectionTexture;
uniform sampler2D     ReflectionTextureMap;
uniform samplerCube   RefractionTexture;
uniform sampler2D     RefractionTextureMap;
uniform sampler2D     HeightmapTexture;

uniform vec4 MaterialBasePropertiesOne;  //x = BaseGlow    y = BaseAO     z = BaseMetalness  w = BaseSmoothness
uniform float CubemapMixFactor;
uniform float RefractionRatio;

uniform vec4 FirstConditionals;          //x = diffuse     y = normals    z = glow           w = specular
uniform vec4 SecondConditionals;         //x = ao          y = metal      z = smoothness     w = reflection
uniform vec4 ThirdConditionals;          //x = refraction  y = heightmap  z = UNUSED         w = UNUSED

uniform vec4 FragDataMisc1;              //xyz = lightPos  w = exposure

uniform int HasGodsRays;

uniform vec4 Object_Color;
uniform vec3 Gods_Rays_Color;

varying vec3 c0;
varying vec3 c1;
varying vec3 WorldPosition;
varying vec3 VCameraPosition;
varying vec3 VCameraPositionReal;
varying vec4 Color;
varying vec2 UV;
varying vec3 Normals; 
varying vec3 Binormals;
varying vec3 Tangents;
flat varying float HasAtmo;
flat varying float FC;
varying float logz_f;

const vec4 ConstantZeroVec4 = vec4(0.0,0.0,0.0,0.0);
const vec3 ConstantAlmostOneVec3 = vec3(0.9999,0.9999,0.9999);
const vec3 ConstantOneVec3 = vec3(1.0,1.0,1.0);
const vec2 ConstantOneVec2 = vec2(1.0,1.0);

vec2 sign_not_zero(vec2 v) {
    return vec2(v.x >= 0 ? 1.0 : -1.0,v.y >= 0 ? 1.0 : -1.0);
}
vec2 EncodeOctahedron(vec3 v) {
    if(  all(greaterThan(v,ConstantAlmostOneVec3))  )
        return ConstantOneVec2;
	v.xy /= dot(abs(v), ConstantOneVec3);
	return mix(v.xy, (1.0 - abs(v.yx)) * sign_not_zero(v.xy), step(v.z, 0.0));
}
vec3 CalcBumpedNormal(){
    vec3 normalTexture = texture2D(NormalTexture, UV).xyz * 2.0 - 1.0;
    mat3 TBN = mat3(Tangents, Binormals, Normals);
    return TBN * normalize(normalTexture);
}
float minnaert(float _NdotL, float _VdotN){
    const float smoothness = 1.1;
    return pow(_VdotN * _NdotL,smoothness)*1.3;
}
float orenNayar(vec3 _ViewDir, vec3 _LightDir,float _NdotL,float _VdotN){
     float roughness = 0.1;
     float _alpha = roughness * roughness;
     float A = 1.0 - 0.5 * _alpha / (_alpha + 0.33);
     float B = 0.45 * _alpha / (_alpha + 0.09);
     float cosAzimuthSinPolarTanPolar = (dot(_LightDir, _ViewDir) - _VdotN * _NdotL) / max(_VdotN, _NdotL);
     return (A + B * max(0.0, cosAzimuthSinPolarTanPolar));
}
void main(){
    if(HasAtmo > 0.99){
        if(FirstConditionals.x > 0.5){
            vec4 diffuse = texture2D(DiffuseTexture, UV) * Object_Color;
            vec3 HDR = (1.0 - exp(-FragDataMisc1.w * (c0 + diffuse.rgb) * c1));
            gl_FragData[0].rgb = max( vec3(0.05) * diffuse.rgb, HDR);    
            if(FirstConditionals.z > 0.5){
                vec3 lightIntensity = max(vec3(0.05) * ConstantOneVec3,(1.0 - exp( -FragDataMisc1.w * ( (c0 + ConstantOneVec3 ) * c1) )));
                gl_FragData[0].rgb = max(gl_FragData[0].rgb, (1.0 - lightIntensity) * texture2D(GlowTexture, UV).rgb);
            }
            gl_FragData[0].a = diffuse.a;
        }
        else{
            gl_FragData[0] = ConstantZeroVec4;
        }
        gl_FragData[1].rg = ConstantOneVec2;

        gl_FragData[2].r = 0.0;
        gl_FragData[2].g = 1.0;
    }
    else{
        if(FirstConditionals.x > 0.5){
		    vec4 diffuse = texture2D(DiffuseTexture, UV) * Object_Color;
		    vec3 PxlNormal;
			if(FirstConditionals.y > 0.5){
			    PxlNormal = normalize(CalcBumpedNormal());
			}
			else{
			    PxlNormal = normalize(Normals);
			}
		    vec3 ViewDir = normalize(VCameraPositionReal - WorldPosition);
			vec3 LightDir = normalize(FragDataMisc1.xyz - WorldPosition);
			float NdotL = max(0.0,dot(PxlNormal,LightDir ));
			float VdotN = max(0.0,dot(PxlNormal,ViewDir ));
            //gl_FragData[0].rgb = diffuse.rgb * vec3(minnaert(NdotL,VdotN));
			gl_FragData[0].rgb = max(diffuse.rgb * vec3(0.038),diffuse.rgb * vec3(orenNayar(ViewDir,LightDir,NdotL,VdotN)) * NdotL * 1.6);
			gl_FragData[0].a = 1.0;
        }
        else{
            gl_FragData[0] = ConstantZeroVec4;
        }
        gl_FragData[1].rg = EncodeOctahedron(vec3(1.0));
        gl_FragData[1].a = texture2D(DiffuseTexture, UV).a;

        if(FirstConditionals.z > 0.5){
            gl_FragData[2].r = texture2D(GlowTexture, UV).r + MaterialBasePropertiesOne.x;
        }
        else{
            gl_FragData[2].r = MaterialBasePropertiesOne.x;
        }

        if(FirstConditionals.w > 0.5){
            gl_FragData[2].g = texture2D(SpecularTexture, UV).r;
        }
        else{
            gl_FragData[2].g = 1.0;
        }
    }
    gl_FragData[2].b = 0.0;
    if(HasGodsRays == 1){
        gl_FragData[3] = vec4(Gods_Rays_Color.r,Gods_Rays_Color.g,Gods_Rays_Color.b,1.0);
    }
    gl_FragDepth = log2(logz_f) * FC;
}
