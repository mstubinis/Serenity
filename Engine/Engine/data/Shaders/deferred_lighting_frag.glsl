#version 120
#define MATERIAL_COUNT_LIMIT 255


uniform vec4 LightDataA; //x = ambient, y = diffuse, z = specular, w = LightDirection.x
uniform vec4 LightDataB; //x = LightDirection.y, y = LightDirection.z, z = const, w = linear
uniform vec4 LightDataC; //x = exp, y = LightPosition.x, z = LightPosition.y, w = LightPosition.z
uniform vec4 LightDataD; //x = LightColor.r, y = LightColor.g, z = LightColor.b, w = LightType
uniform vec4 LightDataE; //x = cutoff, y = outerCutoff, z = UNUSED, w = UNUSED

uniform sampler2D gDiffuseMap;
uniform sampler2D gNormalMap;
uniform sampler2D gMiscMap;
uniform sampler2D gDepthMap;

uniform vec4 ScreenData; //x = near, y = far, z = winSize.x, w = winSize.y
uniform vec4 CamPosGamma; //x = camX, y = camY, z = camZ, w = monitorGamma
uniform vec4 materials[MATERIAL_COUNT_LIMIT]; //r = frensel, g = specPower, b = specularModel, a = diffuseModel 

uniform mat4 VP;
uniform mat4 invVP;

vec3 reconstruct_world_pos(vec2 _uv){
    float log_depth = texture2D(gDepthMap, _uv).r;
    float regularDepth = pow(ScreenData.y + 1.0, log_depth) - 1.0; //log to regular depth

    //linearize regular depth
    float a = ScreenData.y / (ScreenData.y - ScreenData.x);
    float b = ScreenData.y * ScreenData.x / (ScreenData.x - ScreenData.y);
    float depth = (a + b / regularDepth);
    
    //vec4 screenSpace = vec4(_uv * 2.0 - 1.0,depth,1.0);
    vec4 screenSpace = (vec4(_uv,depth, 1.0) * 2.0 - 1.0);

    //world space it!
    vec4 wpos = invVP * screenSpace;
    return wpos.xyz / wpos.w;
}
float BeckmannDist(float theta, float _alpha, float pi){
    float cos2a = theta * theta;
    float b = (1.0 - cos2a) / (cos2a * _alpha);
    return exp(-b) / (pi * _alpha * (cos2a * cos2a));
}
vec3 SchlickFrensel(vec3 f0, float theta){
    vec3 ret = f0 + (vec3(1.0)-f0) * pow( 1.0 - theta, 5.0);
    return ret;
}
vec3 CookTorr(vec3 frensel,float vdoth, float vdotn, float ldotn, float ndoth,float _alpha, float pi){
    float Beck = BeckmannDist(ndoth,_alpha,pi);

    float a = (2.0 * ndoth * vdotn) / vdoth;
    float b = (2.0 * ndoth * ldotn) / vdoth;
    float G = min(1.0, min(a,b));

    return vec3(Beck * frensel * G) / max(vec3(4.0 * vdotn * ldotn),vec3(0.0 + 0.0001));
}
vec3 CalcLightInternal(vec3 LightDir,vec3 PxlWorldPos,vec3 PxlNormal,vec2 uv){
    float Glow = texture2D(gMiscMap,uv).r;
    float SpecularStrength = texture2D(gMiscMap,uv).g;
    vec3 DiffuseTexture = texture2D(gDiffuseMap,uv).rgb;
    if((PxlNormal.r > 0.9999 && PxlNormal.g > 0.9999 && PxlNormal.b > 0.9999)){
        return vec3(0.0);
    }
    vec3 AmbientColor  = LightDataD.xyz * LightDataA.x;
    vec3 DiffuseColor  = vec3(0.0);
    vec3 SpecularColor = vec3(0.0);
    vec3 TotalLight    = vec3(0.0);

    highp int index = int(texture2D(gMiscMap,uv).b * float(MATERIAL_COUNT_LIMIT));

    float SpecularFactor = 0.0;

    float kPi = 3.1415926535898;
    float smoothness = materials[index].g; //UNIFORM

    float F0 = materials[index].r; //UNIFORM
	//vec3 f0 = mix(vec3(F0), DiffuseTexture, vec3(metallic));
	vec3 f0 = mix(vec3(F0), DiffuseTexture, vec3(0.0));

    float roughness = 1.0 - smoothness;
    float alpha = roughness * roughness;
    
    vec3 ViewDir = normalize(CamPosGamma.xyz - PxlWorldPos);
    vec3 Half = normalize(LightDir + ViewDir);
    float NdotL = max(0.0, dot(PxlNormal, LightDir));
    float NdotH = max(0.0, dot(PxlNormal, Half));
    float VdotN = max(0.0, dot(ViewDir,PxlNormal));
    float VdotH = max(0.0, dot(ViewDir,Half));
    
    if(materials[index].a == 0.0){//this is lambert
        DiffuseColor = (NdotL * LightDataD.xyz);
    }
    else if(materials[index].a == 1.0){//this is oren-nayar
        float thetaR = acos(VdotN);
        float thetaI = acos(NdotL);
    
        float A = 1.0 - 0.5 * (alpha / (alpha + 0.33));
        float B = 0.45 * (alpha / (alpha + 0.09));
    
        float a = max(thetaI,thetaR);
        float b = min(thetaI,thetaR);
    
        float gamma = dot(ViewDir - PxlNormal * VdotN, LightDir - PxlNormal * NdotL);
    
        //DiffuseColor = (LightDataD.xyz / kPi) * (cos(thetaI)) * (A + (B * max(0.0,cos(gamma)) * sin(a) * tan(b))) * LightDataA.y;
		DiffuseColor = (LightDataD.xyz) * (cos(thetaI)) * (A + (B * max(0.0,cos(gamma)) * sin(a) * tan(b)));
    }
    else if(materials[index].a == 2.0){//this is ashikhmin-shirley
	    float s = clamp(smoothness,0.01,0.76); //this lighting model has to have some form of roughness in it to look good. cant be 1.0
	    //vec3 A = (28.0 * LightDataD.xyz * LightDataA.y) / vec3(23.0 * kPi);
		vec3 A = (28.0 * LightDataD.xyz) / vec3(23.0);
		float B = (1.0 - (s * LightDataA.z));
		float C = (1.0 - pow((1.0 - (NdotL / 2.0)),5.0));
		float D = (1.0 - pow((1.0 - (VdotN / 2.0)),5.0));
		DiffuseColor = A * B * C * D;
    }
    else if(materials[index].a == 3.0){//this is minneart
        //DiffuseColor = kPi * LightDataD.xyz * pow(VdotN*NdotL,smoothness) * NdotL; //some of these dont use kPi. but it looks nicer with it
		DiffuseColor = LightDataD.xyz * pow(VdotN*NdotL,smoothness) * NdotL; //some of these dont use kPi. but it looks nicer with it
    }

    if(materials[index].b == 0.0){ // this is blinn phong (non-physical)
        smoothness *= 32;
        float kS = (8.0 + smoothness ) / (8.0 * kPi);
        SpecularFactor = kS * pow(NdotH, smoothness);
    }		
    else if(materials[index].b == 1.0){ //this is phong (non-physical)
        smoothness *= 32;
        float kS = (2.0 + smoothness ) / (2.0 * kPi);
        vec3 Reflect = reflect(-LightDir, PxlNormal);
        float VdotR = max(0.0, dot(ViewDir,Reflect));
        SpecularFactor = kS * pow(VdotR, smoothness);
    }
    else if(materials[index].b == 2.0){ //this is GGX (physical)
        float LdotH = max(0.0, dot(LightDir,Half));
        float alphaSqr = alpha * alpha;
        float denom = NdotH * NdotH * (alphaSqr - 1.0) + 1.0;
        float D = alphaSqr / (kPi * denom * denom);
        f0 = SchlickFrensel(f0,LdotH);
        float k = 0.5 * alpha;
        float k2 = k * k;
        SpecularFactor = max(0.0, (NdotL * D * f0.r / (LdotH*LdotH*(1.0-k2)+k2)) );
    }
    else if(materials[index].b == 3.0){ //this is Cook-Torrance (physical)
        f0 = SchlickFrensel(f0,VdotH);
        vec3 res = CookTorr(f0,VdotH,VdotN,NdotL,NdotH,alpha,kPi);
        SpecularFactor = res.x;
    }
    else if(materials[index].b == 4.0){ //this is gaussian (physical)
        float b = acos(NdotH); //this might also be cos. find out
        float fin = b / smoothness;
        SpecularFactor = exp(-fin*fin);
    }
    else if(materials[index].b == 5.0){ //this is beckmann (physical)
        SpecularFactor = BeckmannDist(NdotH,alpha,kPi);
    }
    else if(materials[index].b == 6.0){ //this is ashikhmin-shirley (physical)
	    //make these controllable uniforms
	    const float Nu = 1000.0;
		const float Nv = 1000.0;

		vec3 epsilon = vec3(1.0,0.0,0.0);
		vec3 tangent = normalize(cross(PxlNormal,epsilon));
		vec3 bitangent = normalize(cross(PxlNormal,tangent));
		float hdotT = dot(Half,tangent);
		float hDotB = dot(Half,bitangent);
	    float A = sqrt( (Nu + 1.0) * (Nv + 1.0) );
		float B = pow(NdotH,((Nu * hdotT * hdotT + Nv * hDotB * hDotB) / (1.0 - (NdotH * NdotH)) ));
		float HdotL = max(0.0, dot(Half, LightDir));
		float C = 8.0 * kPi * HdotL * max(NdotL,VdotN);
		SpecularFactor = (A * B) / C;
    }

    SpecularColor = (LightDataD.xyz * LightDataA.z * SpecularFactor) * SpecularStrength;

	vec3 kD = (vec3(1.0) - f0);
	kD *= 1.0 - 0.0; //second 1.0 is metallic value

	TotalLight = (AmbientColor * DiffuseTexture) + (kD * DiffuseTexture  / vec3(kPi) + SpecularColor) * (DiffuseColor * LightDataA.y);

    return max( vec3(Glow)*DiffuseTexture,TotalLight);
}
vec3 CalcPointLight(vec3 LightPos,vec3 PxlWorldPos, vec3 PxlNormal, vec2 uv){
    vec3 LightDir = normalize(LightPos - PxlWorldPos);
    float Dist = length(LightPos - PxlWorldPos);

    vec3 c = CalcLightInternal(LightDir, PxlWorldPos, PxlNormal, uv);

    float attenuation =  1.0 / (max(1.0 , LightDataB.z + (LightDataB.w * Dist) + (LightDataC.x * Dist * Dist)));
    return c * attenuation;
}
vec3 CalcSpotLight(vec3 SpotLightDir, vec3 LightPos,vec3 PxlWorldPos, vec3 PxlNormal, vec2 uv){
    vec3 LightDir = normalize(LightPos - PxlWorldPos);
    vec3 c = CalcPointLight(LightPos, PxlWorldPos, PxlNormal, uv);
    float cosAngle = dot(LightDir, -SpotLightDir);
    float spotEffect = smoothstep(LightDataE.y, LightDataE.x, cosAngle);   
    return c * spotEffect;
}
void main(void){
    //vec2 uv = gl_TexCoord[0].st; //this cannot be used for point light mesh
    vec2 uv = gl_FragCoord.xy / vec2(ScreenData.z,ScreenData.w);

    vec3 PxlPosition = reconstruct_world_pos(uv);
    vec3 PxlNormal = normalize(texture2D(gNormalMap, uv).rgb);

    vec3 lightCalculation = vec3(0.0);
    vec3 LightPosition = vec3(LightDataC.yzw);
    vec3 LightDirection = normalize(vec3(LightDataA.w,LightDataB.x,LightDataB.y));

    if(LightDataD.w == 0.0){
        lightCalculation = CalcLightInternal(normalize(LightPosition - PxlPosition),PxlPosition,PxlNormal,uv);
    }
    else if(LightDataD.w == 1.0){
        lightCalculation = CalcPointLight(LightPosition,PxlPosition,PxlNormal,uv);
    }
    else if(LightDataD.w == 2.0){
        lightCalculation = CalcLightInternal(LightDirection,PxlPosition,PxlNormal,uv);
    }
    else if(LightDataD.w == 3.0){
        lightCalculation = CalcSpotLight(LightDirection,LightPosition,PxlPosition,PxlNormal,uv);
    }
    gl_FragData[0].rgb = lightCalculation;
}
