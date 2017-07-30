#version 120

uniform sampler2D DiffuseTexture;
uniform sampler2D NormalTexture;
uniform sampler2D GlowTexture;
uniform sampler2D SpecularTexture;

uniform sampler2D AOTexture;
uniform sampler2D MetalnessTexture;
uniform sampler2D SmoothnessTexture;
uniform float BaseAO;
uniform float BaseMetalness;
uniform float BaseSmoothness;

uniform samplerCube ReflectionTexture;
uniform sampler2D   ReflectionTextureMap;
uniform samplerCube RefractionTexture;
uniform sampler2D   RefractionTextureMap;

uniform float       CubemapMixFactor;
uniform float       RefractiveIndex;
uniform vec3        CameraPosition;

uniform float BaseGlow;
uniform float matID;

uniform int Shadeless;
uniform int HasGodsRays;

uniform vec3 FirstConditionals;  //x = diffuse  y = normals    z = glow
uniform vec3 SecondConditionals; //x = specular y = reflection z = refraction
uniform vec3 ThirdConditionals; //x = ao y = metalness z = smoothness

uniform vec4 Object_Color;
uniform vec3 Gods_Rays_Color;

varying vec3 WorldPosition;
varying vec2 UV;
varying vec3 Normals; 
varying vec3 Binormals;
varying vec3 Tangents;

varying float FC_2_f;
varying float logz_f;

vec4 PaintersAlgorithm(vec4 paint, vec4 canvas){
    vec4 r = vec4(0.0);
    float Alpha = paint.a + canvas.a * (1.0 - paint.a);
    r.r = (paint.r * paint.a + canvas.r * canvas.a * (1.0-paint.a)) / Alpha;
    r.g = (paint.g * paint.a + canvas.g * canvas.a * (1.0-paint.a)) / Alpha;
    r.b = (paint.b * paint.a + canvas.b * canvas.a * (1.0-paint.a)) / Alpha;
    r.a = Alpha;
    return r;
}
vec4 Reflection(vec4 d, vec3 cpos, vec3 n, vec3 wpos){
    vec4 r = vec4(0.0);
    r = textureCube(ReflectionTexture,reflect(n,normalize(cpos - wpos))) * texture2D(ReflectionTextureMap,UV).r;
    r.a *= CubemapMixFactor;
    r = PaintersAlgorithm(r,d);
    return r;
}
vec4 Refraction(vec4 d, vec3 cpos, vec3 n, vec3 wpos){
    vec4 r = vec4(0.0);
    r = textureCube(RefractionTexture,refract(n,normalize(cpos - wpos),1.0 / RefractiveIndex)) * texture2D(RefractionTextureMap,UV).r;
    r.a *= CubemapMixFactor;
    r = PaintersAlgorithm(r,d);
    return r;
}

vec3 CalcBumpedNormal(void){
    vec3 normTexture = texture2D(NormalTexture, UV).xyz;
    vec3 t = (normTexture * 2.0) - 1.0;
    mat3 TBN = mat3(Tangents, Binormals, Normals);
    return normalize(TBN * t);
}
void main(void){
    gl_FragData[0] = Object_Color;
    gl_FragData[1].rgb = normalize(Normals);
    gl_FragData[2].r = BaseGlow;
    gl_FragData[2].g = 1.0;
	gl_FragData[2].b = BaseMetalness;
	gl_FragData[1].a = matID;

    if(FirstConditionals.x > 0.5){ gl_FragData[0] *= texture2D(DiffuseTexture, UV); }
    if(FirstConditionals.y > 0.5){ gl_FragData[1].rgb = CalcBumpedNormal(); }

    if(SecondConditionals.y > 0.5){
        gl_FragData[0] = Reflection(gl_FragData[0],CameraPosition,gl_FragData[1].rgb,WorldPosition);
    }
    
    if(SecondConditionals.z > 0.5){
        gl_FragData[0] = Refraction(gl_FragData[0],CameraPosition,gl_FragData[1].rgb,WorldPosition);
    }
	if(ThirdConditionals.z > 0.5){
	    gl_FragData[2].b = BaseSmoothness * texture2D(MetalnessTexture, UV).r;
	}
    
    if(Shadeless == 0){
        if(FirstConditionals.z > 0.5){ 
            gl_FragData[2].r += texture2D(GlowTexture, UV).r; 
        }
        if(SecondConditionals.x > 0.5){ 
            gl_FragData[2].g = texture2D(SpecularTexture, UV).r; 
        }
    }
    else{ 
        gl_FragData[1].rgb = vec3(1.0); 
    }

    if(HasGodsRays == 1){
        gl_FragData[3] = (texture2D(DiffuseTexture, UV) * vec4(Gods_Rays_Color,1.0))*0.5;
    }
    gl_FragDepth = log2(logz_f) * FC_2_f;
}
