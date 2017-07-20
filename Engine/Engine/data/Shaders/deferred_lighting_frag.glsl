#version 120
#define MATERIAL_COUNT_LIMIT 255

uniform int LightType;

uniform float gamma;

uniform vec3 LightColor;

uniform vec3 LightIntensities; //x = ambient, y = diffuse, z = specular
uniform vec3 LightData;        //x = constant, y = linear z = exponent

uniform vec3 LightDirection;
uniform vec3 LightPosition;

uniform sampler2D gNormalMap;
uniform sampler2D gMiscMap;
uniform sampler2D gDepthMap;
uniform vec2 gScreenSize;

uniform vec3 gCameraPosition;

uniform vec4 materials[MATERIAL_COUNT_LIMIT];

uniform mat4 invVP;
uniform float nearz;
uniform float farz;

float linearize_depth(float depth){
    float a = farz / (farz - nearz);
    float b = farz * nearz / (nearz - farz);
    return (a + b / depth);
}
float invertLogDepth(float log_depth){
    return linearize_depth(pow(farz + 1.0, log_depth) - 1.0);
}
vec3 reconstruct_world_pos(vec2 _uv){
    float depth = texture2D(gDepthMap, _uv).r;
    vec4 wpos = invVP * (vec4(_uv, invertLogDepth(depth), 1.0) * 2.0 - 1.0);
    return wpos.xyz / wpos.w;
}
float BeckmannDist(float theta, float roughness, float pi){
    float cos2a = theta * theta;
    float b = (1.0 - cos2a) / (cos2a * roughness);
    return exp(-b) / (pi * roughness * (cos2a * cos2a));
}
float SchlickFrensel(float f0, float theta){
    return f0 + (1.0-f0) * pow( 1.0 - theta, 5.0);
}
vec3 CookTorr(float frensel,float vdoth, float vdotn, float ldotn, float ndoth,float _alpha, float pi){
    float Beck = BeckmannDist(ndoth,_alpha,pi);

    float a = (2.0 * (ndoth) * (vdotn)) / (vdoth);
    float b = (2.0 * (ndoth) * (ldotn)) / (vdoth);
    float G = min(1.0,min(a,b));

    float fin = (Beck * frensel * G) / (max((4.0 * vdotn * ldotn),0.0) + 0.0001);

    return vec3(fin);
}
vec3 CalcLightInternal(vec3 LightDir,vec3 PxlWorldPos,vec3 PxlNormal,vec2 uv){
    float Glow = texture2D(gMiscMap,uv).r;
    if((PxlNormal.r > 0.9999 && PxlNormal.g > 0.9999 && PxlNormal.b > 0.9999) || Glow > 0.99 ){
        return vec3(1.0);
    }
    vec3 AmbientColor  = LightColor * LightIntensities.x;
    vec3 DiffuseColor  = vec3(0.0);
    vec3 SpecularColor = vec3(0.0);
    vec3 TotalLight    = vec3(0.0);

    highp int index = int(texture2D(gMiscMap,uv).b * float(MATERIAL_COUNT_LIMIT));
    float Lambertian = max(dot(LightDir,PxlNormal), 0.0);
    DiffuseColor = (Lambertian * LightColor) * LightIntensities.y;
    vec3 ViewDir = normalize(gCameraPosition - PxlWorldPos);
    
    float SpecularAngle = 0.0;

    float kPi = 3.1415926535898;
    float smoothness = materials[index].g; //note: this value is automatically clamped 0 to 1 with the phyiscally based models below
    float roughness = 1.0 - smoothness; //only valid for physical lighting models
    float alpha = roughness * roughness;
    
    vec3 Half = normalize(LightDir + ViewDir);
    float NdotH = max(dot(PxlNormal, Half), 0.0);
    float NdotL = max(dot(PxlNormal, LightDir), 0.0);
    
    if(materials[index].b == 0.0){ // this is blinn phong (non-physical)

        float conserv = (8.0 + smoothness ) / (8.0 * kPi);
        SpecularAngle = conserv * pow(NdotH), smoothness);
    }		
    else if(materials[index].b == 1.0){ //this is phong (non-physical)

        float conserv = (2.0 + smoothness ) / (2.0 * kPi);
        vec3 Reflect = reflect(-LightDir, PxlNormal);
        SpecularAngle = conserv * pow(max(dot(ViewDir, Reflect), 0.0), smoothness);
    }
    else if(materials[index].b == 2.0){ //this is GGX (physical)

        float F0 = 0.8; //fresnel term, 0 to 1
        float LdotH = max(0.0, dot(LightDir,Half));
        float alphaSqr = alpha * alpha;
        float denom = NdotH * NdotH * (alphaSqr - 1.0) + 1.0;
        float D = alphaSqr / (kPi * denom * denom);
        float Fresnel = SchlickFrensel(F0,LdotH);
        float k = 0.5 * alpha;
        float k2 = k * k;
        SpecularAngle = max(0.0, (NdotL * D * Fresnel / (LdotH*LdotH*(1.0-k2)+k2)) );
    }
    else if(materials[index].b == 3.0){ //this is Cook-Torrance (physical)
        float F0 = 0.8; //fresnel term, 0 to 1
        float VdotH = max(0.0, dot(ViewDir,Half));
        float VdotN = max(0.0, dot(ViewDir,PxlNormal));
        float Frensel = SchlickFrensel(F0,VdotH).r;
        SpecularAngle = CookTorr(Frensel,VdotH,VdotN,NdotL,NdotH,alpha,kPi).r;
    }
    else if(materials[index].b == 4.0){ //this is gaussian (physical)

        float b = acos(NdotH); //this might also be cos. find out
        float fin = b / smoothness;
        SpecularAngle = exp(-fin*fin);
    }
    else if(materials[index].b == 5.0){ //this is beckmann (physical)

        SpecularAngle = BeckmannDist(NdotH,alpha,kPi);
    }
    else if(materials[index].b == 6.0){ //this is PBR (physical)
        /*
        vec3 F0 = mix(vec3(0.04), albedo, metallic);
        vec3 Lo = vec3(0.0);
        float attenuation = 1.0;
        float Distance = length(LightDir);
        if(LightType != 0 && LightType != 2){
            attenuation = 1.0 / (max(1.0 , LightData.x + (LightData.y * Distance) + (LightData.z * Distance * Distance)));
        }
        
        vec3 radiance = LightColor * attenuation; // calculate per-light radiance 

        vec3 Frensel = vec3(SchlickFrensel(F0,vdoth));
        vec3 kD = (vec3(1.0) - Frensel) * (1.0 - metallic);

        vec3 n  =  CookTorr(Frensel.r,VdotH,VdotN,LdotN,NdotH,alpha,kPi);
        float d = 4.0 * max(dot(PxlNormal, ViewDir), 0.0) * NdotL + 0.001; 

        // add to outgoing radiance Lo           
        Lo += (kD * albedo / vec3(kPi) + (n / vec3(d))) * radiance * NdotL; 

        //vec3 ambient = AmbientColor * albedo * ao;
        vec3 ambient = AmbientColor * ao;
        vec3 color = ambient + Lo;
        
        //here he tone mapped and gammad, but prob can skip this to HDR pass

        TotalLight = vec4(color, 1.0);
        return max( vec3(Glow), TotalLight);
        */
    }

    SpecularColor = (LightColor * LightIntensities.z * SpecularAngle) * texture2D(gMiscMap,uv).g; //texture2D is specular map

    TotalLight = AmbientColor + DiffuseColor + SpecularColor;
    return max( vec3(Glow), TotalLight);
}
vec3 CalcPointLight(vec3 PxlWorldPos, vec3 PxlNormal, vec2 uv){
    vec3 LightDir = normalize(LightPosition - PxlWorldPos);
    float Distance = length(LightDir);

    vec3 c = CalcLightInternal(LightDir, PxlWorldPos, PxlNormal, uv);

    float attenuation =  1.0 / (max(1.0 , LightData.x + (LightData.y * Distance) + (LightData.z * Distance * Distance)));
    return c * attenuation;
}
vec3 CalcSpotLight(vec3 PxlWorldPos, vec3 PxlNormal, vec2 uv){
    return vec3(0);
}
void main(void){
    //vec2 uv = gl_TexCoord[0].st; //this cannot be used for point light mesh
    vec2 uv = gl_FragCoord.xy / gScreenSize;

    vec3 PxlPosition = reconstruct_world_pos(uv);
    vec3 PxlNormal = texture2D(gNormalMap, uv).rgb;

    vec3 lightCalculation = vec3(0.0);

    if(LightType == 0){
        lightCalculation = CalcLightInternal(normalize(LightPosition - PxlPosition),PxlPosition,PxlNormal,uv);
    }
    else if(LightType == 1){
        lightCalculation = CalcPointLight(PxlPosition,PxlNormal,uv);
    }
    else if(LightType == 2){
        lightCalculation = CalcLightInternal(LightDirection,PxlPosition,PxlNormal,uv);
    }
    else if(LightType == 3){
        lightCalculation = CalcSpotLight(PxlPosition,PxlNormal,uv);
    }
    gl_FragData[0].rgb = lightCalculation;
}
