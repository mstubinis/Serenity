
USE_LOG_DEPTH_FRAGMENT

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
flat varying float HasAtmo;

const vec4 ConstantZeroVec4 = vec4(0.0,0.0,0.0,0.0);
const vec3 ConstantAlmostOneVec3 = vec3(0.9999,0.9999,0.9999);
const vec3 ConstantOneVec3 = vec3(1.0,1.0,1.0);
const vec3 ConstantZeroVec3 = vec3(0.0,0.0,0.0);
const vec2 ConstantZeroVec2 = vec2(0.0,0.0);
const vec2 ConstantOneVec2 = vec2(1.0,1.0);

float Pack2NibblesInto8BitChannel(float x,float y){
    float lowEnd = round(x / 0.0666);
    float highEnd = round(y / 0.0666) * 16.0;
    return (highEnd + lowEnd) / 255.0;
}
vec2 Unpack2NibblesFrom8BitChannel(float data){
    float d = data * 255.0;
    float y = fract(d / 16.0);
    float x = (d - (y * 16.0));
    return vec2(y, x / 255.0);
}
float Pack2FloatIntoFloat16(float x,float y){
    x = clamp(x,0.0001,0.9999);
    y = clamp(y,0.0001,0.9999);
    float _x = (x + 1.0) * 0.5;
    float _y = (y + 1.0) * 0.5;
    return floor(_x * 100.0) + _y;
}
vec2 sign_not_zero(vec2 v) {
    return vec2(v.x >= 0 ? 1.0 : -1.0,v.y >= 0 ? 1.0 : -1.0);
}
vec2 EncodeOctahedron(vec3 v) {
    if(  all(greaterThan(v,ConstantAlmostOneVec3))  )
        return ConstantOneVec2;
    v.xy /= dot(abs(v), ConstantOneVec3);
    return mix(v.xy, (1.0 - abs(v.yx)) * sign_not_zero(v.xy), step(v.z, 0.0));
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
    vec4 InDiffuse = texture2D(DiffuseTexture, UV) * Object_Color;
    vec4 OutDiffuse = ConstantZeroVec4;
    vec2 OutNormals = ConstantOneVec2;
    vec3 InGlow = texture2D(GlowTexture, UV).rgb;
    float OutGlow = 0.0;
    float OutSpecular = 1.0;
    float OutPackedMetalnessSmoothness = 1.0;
    if(HasAtmo > 0.99){
        if(FirstConditionals.x > 0.5){
            vec3 HDR = (1.0 - exp(-FragDataMisc1.w * (c0 + InDiffuse.rgb) * c1));
            OutDiffuse.rgb = max(0.05 * InDiffuse.rgb, HDR);    
            if(FirstConditionals.z > 0.5){
                vec3 lightIntensity = max(0.05 * ConstantOneVec3,(1.0 - exp( -FragDataMisc1.w * ( (c0 + ConstantOneVec3 ) * c1) )));
                OutDiffuse = vec4(max(OutDiffuse.rgb, (1.0 - lightIntensity) * InGlow),InDiffuse.a);
            }
        }
    }else{
        if(FirstConditionals.x > 0.5){
            vec3 PxlNormal = normalize(Normals);
            if(FirstConditionals.y > 0.5){
                PxlNormal = CalcBumpedNormal(UV,NormalTexture);
            }
            vec3 ViewDir = normalize(VCameraPositionReal - WorldPosition);
            vec3 LightDir = normalize(FragDataMisc1.xyz - WorldPosition);
            float NdotL = max(0.0,dot(PxlNormal,LightDir));
            float VdotN = max(0.0,dot(PxlNormal,ViewDir));
            //OutDiffuse = vec4(InDiffuse.rgb * minnaert(NdotL,VdotN),InDiffuse.a);
            OutDiffuse = vec4(max(InDiffuse.rgb * 0.038,InDiffuse.rgb * orenNayar(ViewDir,LightDir,NdotL,VdotN) * NdotL * 1.6),InDiffuse.a);
        }
        OutNormals.rg = EncodeOctahedron(ConstantOneVec3);
        OutPackedMetalnessSmoothness = Pack2FloatIntoFloat16(0.0,1.0);

        if(FirstConditionals.z > 0.5){
            OutGlow = InGlow.r + MaterialBasePropertiesOne.x;
        }else{
            OutGlow = MaterialBasePropertiesOne.x;
        }
        if(FirstConditionals.w > 0.5){
            OutSpecular = texture2D(SpecularTexture, UV).r;
        }
    }
    gl_FragData[0] = OutDiffuse;          
    gl_FragData[1] = vec4(OutNormals,0.0,OutPackedMetalnessSmoothness); //0.0 = matID + ao, which is never used
    vec4 GodRays = vec4(Gods_Rays_Color,1.0);
    float GodRaysRG = Pack2NibblesInto8BitChannel(GodRays.r,GodRays.g);
    gl_FragData[2] = vec4(OutGlow,OutSpecular,GodRaysRG,GodRays.b);
}
