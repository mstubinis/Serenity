#version 120

uniform sampler2D DiffuseTexture;
uniform sampler2D NormalTexture;
uniform sampler2D GlowTexture;
uniform sampler2D SpecularTexture;

uniform samplerCube ReflectionTexture;
uniform sampler2D   ReflectionTextureMap;
uniform samplerCube RefractionTexture;
uniform sampler2D   RefractionTextureMap;

uniform float       CubemapMixFactor;
uniform float       RefractionRatio;

uniform float far;
uniform float C;
uniform float BaseGlow;
uniform float matID;

uniform int Shadeless;
uniform int HasGodsRays;

uniform vec3 FirstConditionals;  //x = diffuse  y = normals    z = glow
uniform vec3 SecondConditionals; //x = specular y = reflection z = refraction

uniform vec4 Object_Color;
uniform vec3 Gods_Rays_Color;
uniform vec3 CameraPosition;

varying vec3 WorldPosition;
varying vec2 UV;
varying vec3 Normals; 
varying vec3 Binormals;
varying vec3 Tangents;

vec4 PaintersAlgorithm(vec4 paint, vec4 canvas){
    vec4 r = vec4(0.0);
    float Alp = paint.a + canvas.a * (1.0 - paint.a);
    r.r = (paint.r * paint.a + canvas.r * canvas.a * (1.0-paint.a)) / Alp;
    r.g = (paint.g * paint.a + canvas.g * canvas.a * (1.0-paint.a)) / Alp;
    r.b = (paint.b * paint.a + canvas.b * canvas.a * (1.0-paint.a)) / Alp;
    r.a = Alp;
    return r;
}
vec4 Reflection(vec4 d, vec3 cpos, vec3 n, vec3 wpos){
    vec4 r = vec4(0.0);
    r = textureCube(ReflectionTexture,reflect(n,cpos - wpos)) * texture2D(ReflectionTextureMap,UV).r;
    r.a *= CubemapMixFactor;
    r = PaintersAlgorithm(r,d);
    return r;
}
vec4 Refraction(vec4 d, vec3 cpos, vec3 n, vec3 wpos){
    vec4 r = vec4(0.0);
    r = textureCube(RefractionTexture,refract(n,cpos - wpos,RefractionRatio)) * texture2D(RefractionTextureMap,UV).r;
    r.a *= CubemapMixFactor;
    r = PaintersAlgorithm(r,d);
    return r;
}

vec3 CalcBumpedNormal(){
    vec3 t = (texture2D(NormalTexture, UV).xyz * 2.0) - 1.0;
    mat3 TBN = mat3(Tangents, Binormals, Normals);
    return normalize(TBN * t);
}
void main(void){
    gl_FragData[0] = Object_Color;
    gl_FragData[1].rgb = normalize(Normals);
    gl_FragData[2].r = BaseGlow;
    gl_FragData[2].g = 1.0;
    if(FirstConditionals.x > 0.5){ gl_FragData[0] *= texture2D(DiffuseTexture, UV); }
    if(FirstConditionals.y > 0.5){ gl_FragData[1].rgb = CalcBumpedNormal(); }

    if(SecondConditionals.y > 0.5){
        gl_FragData[0] = Reflection(gl_FragData[0],CameraPosition,gl_FragData[1].rgb,WorldPosition);
    }
    if(SecondConditionals.z > 0.5){
        gl_FragData[0] = Refraction(gl_FragData[0],CameraPosition,gl_FragData[1].rgb,WorldPosition);
    }

    if(Shadeless == 0){
        if(FirstConditionals.z > 0.5){ gl_FragData[2].r += texture2D(GlowTexture, UV).r; }
        if(SecondConditionals.x > 0.5){ gl_FragData[2].g = texture2D(SpecularTexture, UV).r; }
    }
    else{ gl_FragData[1].rgb = vec3(1.0); }

    gl_FragData[1].a = Object_Color.a;
    gl_FragData[2].b = matID;
    gl_FragData[3] = vec4(WorldPosition,1.0);
    if(HasGodsRays == 1){
        gl_FragData[4] = (texture2D(DiffuseTexture, UV) * vec4(Gods_Rays_Color,1.0))*0.5;
    }
    gl_FragDepth = (log(C * gl_TexCoord[6].z + 1.0) / log(C * far + 1.0));
}