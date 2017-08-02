#version 120

uniform float fExposure;
uniform float BaseGlow;

uniform sampler2D DiffuseTexture;
uniform sampler2D NormalTexture;
uniform sampler2D GlowTexture;
uniform sampler2D SpecularTexture;

uniform samplerCube ReflectionTexture;
uniform sampler2D   ReflectionTextureMap;
uniform float       CubemapMixFactor;
uniform samplerCube RefractionTexture;
uniform sampler2D   RefractionTextureMap;
uniform float       RefractionRatio;

uniform vec3 FirstConditionals;  //x = diffuse  y = normals    z = glow
uniform vec3 SecondConditionals; //x = specular y = ao z = metalness
uniform vec3 ThirdConditionals; //x = smoothness y = reflection z = refraction

uniform int HasGodsRays;

uniform vec4 Object_Color;
uniform vec3 Gods_Rays_Color;

varying vec3 c0;
varying vec3 c1;

uniform int HasAtmosphere;

varying vec3 WorldPosition;
varying vec3 CameraPosition;
varying vec4 Color;
varying vec2 UV;
varying vec3 Normals; 
varying vec3 Binormals;
varying vec3 Tangents;

varying float FC_2_f;
varying float logz_f;

vec3 CalcBumpedNormal(void){
    vec3 t = (texture2D(NormalTexture, UV).xyz * 2.0) - 1.0;
    mat3 TBN = mat3(Tangents, Binormals, Normals);
    return normalize(TBN * t);
}
void main(void){
    if(HasAtmosphere == 1){
        if(FirstConditionals.x > 0.5){
            vec4 diffuse = texture2D(DiffuseTexture, UV) * Object_Color;
            gl_FragData[0].rgb = max(vec3(0.05,0.05,0.05)*diffuse.rgb,(1.0-exp(-fExposure*((c0+diffuse.rgb)*c1))));    
            if(FirstConditionals.z > 0.5){
                vec3 lightIntensity = max(vec3(0.05,0.05,0.05)*vec3(1.0),(1.0 - exp( -fExposure * ((c0+vec3(1.0)) * c1) )));
                gl_FragData[0].rgb = max(gl_FragData[0].rgb, (1.0-lightIntensity)*texture2D(GlowTexture, UV).rgb);
            }
            gl_FragData[0].a = diffuse.a;
        }
        else{
            gl_FragData[0] = vec4(0.0);
        }
        gl_FragData[1] = vec4(1.0);
        
        gl_FragData[2].r = 0.0;
        gl_FragData[2].g = 1.0;
    }
    else{
        if(FirstConditionals.x > 0.5){
            gl_FragData[0] = texture2D(DiffuseTexture, UV) * Object_Color;
        }
        else{
            gl_FragData[0] = vec4(0.0);
        }

        if(FirstConditionals.y > 0.5){
            gl_FragData[1].rgb = normalize(CalcBumpedNormal());
            gl_FragData[1].a = texture2D(DiffuseTexture, UV).a;
        }
        else{
            gl_FragData[1].rgb = normalize(Normals);
            gl_FragData[1].a = texture2D(DiffuseTexture, UV).a;
        }

        if(FirstConditionals.z > 0.5){
            gl_FragData[2].r = texture2D(GlowTexture, UV).r + BaseGlow;
        }
        else{
            gl_FragData[2].r = BaseGlow;
        }

        if(SecondConditionals.x > 0.5){
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
    gl_FragDepth = log2(logz_f) * FC_2_f;
}
