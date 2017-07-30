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
float BeckmannDist(float cos2a, float _alpha, float pi){
    float b = (1.0 - cos2a) / (cos2a * _alpha);
    return (exp(-b)) / (pi * _alpha * cos2a * cos2a);
}
float GGXDist(float NdotHSquared, float alphaSquared,float pi){
    float nom   = alphaSquared;
    float denom = (NdotHSquared * (alphaSquared - 1.0) + 1.0);
    denom = pi * denom * denom;
    return nom / denom;
}
float GeometrySchlickGGX(float NdotV, float roughness){
    float k = (roughness*roughness) / 8.0;
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return nom / denom;
}
vec3 SchlickFrensel(float cosTheta, vec3 frenselFactor){
    vec3 ret = frenselFactor + (vec3(1.0) - frenselFactor) * pow( 1.0 - cosTheta, 5.0);
    return ret;
}
vec3 CalcLightInternal(vec3 LightDir,vec3 PxlWorldPos,vec3 PxlNormal,vec2 uv){
    float Glow = texture2D(gMiscMap,uv).r;
    float SpecularStrength = texture2D(gMiscMap,uv).g;
    vec3 MaterialAlbedoTexture = texture2D(gDiffuseMap,uv).rgb;
    if((PxlNormal.r > 0.9999 && PxlNormal.g > 0.9999 && PxlNormal.b > 0.9999)){
        return vec3(0.0);
    }
    vec3 AmbientColor       = LightDataD.xyz * LightDataA.x;
    vec3 LightDiffuseColor  = LightDataD.xyz;
    vec3 LightSpecularColor = LightDataD.xyz * SpecularStrength;
    vec3 TotalLight         = vec3(0.0);

    highp int index = int(texture2D(gNormalMap,uv).a);

    vec3 SpecularFactor = vec3(0.0);

    float kPi = 3.1415926535898;
    float smoothness = materials[index].g; //UNIFORM

    vec3 F0 = vec3(materials[index].r); //UNIFORM
	float metalness = texture2D(gMiscMap,uv).b;
	F0 = mix(F0, MaterialAlbedoTexture, vec3(metalness));
	vec3 Frensel = F0;

    float roughness = 1.0 - smoothness;
    float alpha = roughness * roughness;
    
    vec3 ViewDir = normalize(CamPosGamma.xyz - PxlWorldPos);
    vec3 Half = normalize(LightDir + ViewDir);
    float NdotL = max(0.0, dot(PxlNormal, LightDir));
    float NdotH = max(0.0, dot(PxlNormal, Half));
    float VdotN = max(0.0, dot(ViewDir,PxlNormal));
    float VdotH = max(0.0, dot(ViewDir,Half));
    
    if(materials[index].a == 0.0){//this is lambert
        LightDiffuseColor *= LightDataA.y;
    }
    else if(materials[index].a == 1.0){//this is oren-nayar
        float thetaR = acos(VdotN);
        float thetaI = acos(NdotL);
        float A = 1.0 - 0.5 * (alpha / (alpha + 0.33));
        float B = 0.45 * (alpha / (alpha + 0.09));
        float gamma = dot(ViewDir - PxlNormal * VdotN, LightDir - PxlNormal * NdotL);
        LightDiffuseColor *= (cos(thetaI)) * (A + (B * max(0.0,cos(gamma)) * sin(max(thetaI,thetaR)) * tan(min(thetaI,thetaR)))) * LightDataA.y;
    }
    else if(materials[index].a == 2.0){//this is ashikhmin-shirley
	    float s = clamp(smoothness,0.01,0.76); //this lighting model has to have some form of roughness in it to look good. cant be 1.0
	    vec3 A = (28.0 * LightDataA.y) / vec3(23.0 * kPi);
		float B = (1.0 - (s * LightDataA.z));
		float C = (1.0 - pow((1.0 - (NdotL / 2.0)),5.0));
		float D = (1.0 - pow((1.0 - (VdotN / 2.0)),5.0));
		LightDiffuseColor *= A * B * C * D;
		LightDiffuseColor *= kPi; //i know this isnt proper, but the diffuse component is *way* too dark otherwise...
    }
    else if(materials[index].a == 3.0){//this is minneart
		LightDiffuseColor *= pow(VdotN*NdotL,smoothness) * LightDataA.y;
    }

    if(materials[index].b == 0.0){ // this is blinn phong (non-physical)
        smoothness *= 32;
        float kS = (8.0 + smoothness ) / (8.0 * kPi);
		kS /= kPi;
        SpecularFactor = vec3(kS * pow(NdotH, smoothness));
    }		
    else if(materials[index].b == 1.0){ //this is phong (non-physical)
        smoothness *= 32;
        float kS = (2.0 + smoothness ) / (2.0 * kPi);
		kS /= kPi;
        vec3 Reflect = reflect(-LightDir, PxlNormal);
        float VdotR = max(0.0, dot(ViewDir,Reflect));
        SpecularFactor = vec3(kS * pow(VdotR, smoothness));
    }
    else if(materials[index].b == 2.0){ //this is GGX (physical)
        float LdotH = max(0.0, dot(LightDir,Half));
        float alphaSqr = alpha * alpha;
        float denom = NdotH * NdotH * (alphaSqr - 1.0) + 1.0;
        float D = alphaSqr / (kPi * denom * denom);
        Frensel = SchlickFrensel(LdotH,F0);
        float k = 0.5 * alpha;
        float k2 = k * k;
        SpecularFactor = max(vec3(0.0), (NdotL * D * Frensel / (LdotH*LdotH*(1.0-k2)+k2)) );
    }
    else if(materials[index].b == 3.0){ //this is Cook-Torrance (physical)
        Frensel = SchlickFrensel(VdotH,F0);
		float NDF = GGXDist(NdotH*NdotH,alpha*alpha,kPi);
		//float G = min(1.0, min(  (2.0 * NdotH * VdotN) / VdotH  ,  (2.0 * NdotH * NdotL) / VdotH  ));
		float G = GeometrySchlickGGX(VdotN,roughness + 1.0) * GeometrySchlickGGX(NdotL,roughness + 1.0);
		vec3 Top = NDF * Frensel * G;
		float Bottom = max(4.0 * VdotN * NdotL,0.0 + 0.0001);
        SpecularFactor = Top / Bottom;
    }
    else if(materials[index].b == 4.0){ //this is gaussian (physical)
        float b = acos(NdotH); //this might also be cos. find out
        float fin = b / smoothness;
        SpecularFactor = vec3(exp(-fin*fin));
    }
    else if(materials[index].b == 5.0){ //this is beckmann (physical)
        SpecularFactor = vec3(BeckmannDist(NdotH,alpha,kPi));
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
		SpecularFactor = vec3((A * B) / C);
    }
	LightDiffuseColor *= LightDataA.y;
	LightSpecularColor *= SpecularFactor;
    LightSpecularColor *= LightDataA.z;

	vec3 kD = (vec3(1.0) - Frensel);
	kD *= 1.0 - metalness;
	TotalLight = (kD * MaterialAlbedoTexture  /  kPi + LightSpecularColor) * LightDiffuseColor * NdotL;

	TotalLight += (AmbientColor * MaterialAlbedoTexture);
    return max( vec3(Glow) * MaterialAlbedoTexture,TotalLight);
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
