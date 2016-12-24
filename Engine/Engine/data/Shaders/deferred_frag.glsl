#version 120

uniform sampler2D DiffuseTexture;
uniform sampler2D NormalTexture;
uniform sampler2D GlowTexture;
uniform sampler2D SpecularTexture;

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

varying vec3 WorldPosition;
varying vec2 UV;
varying vec3 Normals; 
varying vec3 Binormals;
varying vec3 Tangents;

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