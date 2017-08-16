#version 120

uniform float fExposure;
uniform float BaseGlow;

uniform sampler2D DiffuseTexture;
uniform sampler2D NormalTexture;
uniform sampler2D GlowTexture;
uniform sampler2D SpecularTexture;

uniform sampler2D AOTexture;
uniform sampler2D MetalnessTexture;
uniform sampler2D SmoothnessTexture;
uniform float     BaseAO;
uniform float     BaseMetalness;
uniform float     BaseSmoothness;

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

float Round(float x){
    return x < 0.0 ? int(x - 0.5) : int(x + 0.5);
}
vec2 sign_not_zero(vec2 v) {
    return vec2(v.x >= 0 ? 1.0 : -1.0,v.y >= 0 ? 1.0 : -1.0);
}
vec2 EncodeOctahedron(vec3 v) {
    if(v.r > 0.9999 && v.g > 0.9999 && v.b > 0.9999)
        return vec2(1.0);
	v.xy /= dot(abs(v), vec3(1.0));
	// Branch-Less version
	return mix(v.xy, (1.0 - abs(v.yx)) * sign_not_zero(v.xy), step(v.z, 0.0));
}
vec3 DecodeOctahedron(vec2 n) {
    if(n.r > 0.9999 && n.g > 0.9999)
        return vec3(1.0);
	vec3 v = vec3(n.xy, 1.0 - abs(n.x) - abs(n.y));
	if (v.z < 0.0) v.xy = (1.0 - abs(v.yx)) * sign_not_zero(v.xy);
	return normalize(v);
}
vec2 EncodeSpherical(vec3 n){
    if(n.r > 0.9999 && n.g > 0.9999 && n.b > 0.9999)
        return vec2(1.0);
    vec2 encN;
    encN.x = atan( n.x, n.y ) * 1.0 / 3.1415926535898;
    encN.y = n.z;
    encN = encN * 0.5 + 0.5;
    return encN;
}
vec3 DecodeSpherical(vec2 encN){
    if(encN.r > 0.9999 && encN.g > 0.9999)
        return vec3(1.0);
     vec2 ang = encN * 2.0 - 1.0;
     vec2 scth;
     float ang2 = ang.x * 3.1415926535898;
     scth.x = sin(ang2);
     scth.y = cos(ang2);
     vec2 scphi = vec2( sqrt( 1.0 - ang.y * ang.y ), ang.y );
     vec3 n;
     n.x = scth.y * scphi.x;
     n.y = scth.x * scphi.x;
     n.z = scphi.y;
     return n;
}
vec2 EncodeStereographic(vec3 n){
    if(n.r > 0.9999 && n.g > 0.9999 && n.b > 0.9999)
        return vec2(1.0);
	float scale = 1.7777777;
	vec2 enc = n.xy / (n.z+1.0);
	enc /= scale;
	enc = enc*0.5+0.5;
	return enc;
}
vec3 DecodeStereographic(vec2 enc){
    if(enc.r > 0.9999 && enc.g > 0.9999)
        return vec3(1.0);
	float scale = 1.7777777;
	vec3 nn = vec3(enc.xy,1.0)*vec3(2.0*scale,2.0*scale,0.0) + vec3(-scale,-scale,1.0);
	float g = 2.0 / dot(nn.xyz,nn.xyz);
	vec3 n;
	n.xy = g*nn.xy;
	n.z = g-1.0;
	return normalize(n);
}

vec3 CalcBumpedNormal(void){
    vec3 normalTexture = texture2D(NormalTexture, UV).xyz * 2.0 - 1.0;
    mat3 TBN = mat3(Tangents, Binormals, Normals);
    return TBN * normalize(normalTexture);
}
void main(void){
    if(HasAtmosphere == 1){
        if(FirstConditionals.x > 0.5){
            vec4 diffuse = texture2D(DiffuseTexture, UV) * Object_Color;
			vec3 HDR = (1.0-exp(-fExposure*(c0+diffuse.rgb)*c1));
            gl_FragData[0].rgb = max(vec3(0.05,0.05,0.05)*diffuse.rgb,HDR);    
            if(FirstConditionals.z > 0.5){
                vec3 lightIntensity = max(vec3(0.05,0.05,0.05)*vec3(1.0),(1.0 - exp( -fExposure * ((c0+vec3(1.0)) * c1) )));
                gl_FragData[0].rgb = max(gl_FragData[0].rgb, (1.0-lightIntensity)*texture2D(GlowTexture, UV).rgb);
            }
            gl_FragData[0].a = diffuse.a;
        }
        else{
            gl_FragData[0] = vec4(0.0);
        }
        gl_FragData[1].rg = vec2(1.0);
        
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
            gl_FragData[1].rg = EncodeOctahedron(CalcBumpedNormal());
            gl_FragData[1].a = texture2D(DiffuseTexture, UV).a;
        }
        else{
            gl_FragData[1].rg = EncodeOctahedron(normalize(Normals));
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
