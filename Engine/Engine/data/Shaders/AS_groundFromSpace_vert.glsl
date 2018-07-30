attribute vec3 position;
//attribute float uv;
attribute vec2 uv;
attribute vec4 normal;
attribute vec4 binormal;
attribute vec4 tangent;

uniform int HasAtmosphere;

uniform mat4 VP;
uniform mat4 Model;
uniform mat4 Rot;

uniform int nSamples;
uniform int fromAtmosphere;

uniform float fcoeff;

uniform vec4 VertDataMisc1;     //xyz = camPos,             w = lightDir.x
uniform vec4 VertDataMisc2;     //xyz = camPosReal,         w = lightDir.y
uniform vec4 VertDataMisc3;     //xyz = v3InvWaveLength,    w = lightDir.z
uniform vec4 VertDataScale;     //fScale,fScaledepth,fScaleOverScaleDepth,fSamples
uniform vec4 VertDataRadius;    //fCameraHeight2,fOuterRadius,fOuterRadius2,fInnerRadius
uniform vec4 VertDatafK;        //fKrESun,fKmESun,fKr4PI,fKm4PI

varying vec3 c0;
varying vec3 c1;
varying vec2 UV;
varying vec3 WorldPosition;
varying vec3 VCameraPositionReal;
varying vec3 Normals;
varying vec3 Binormals;
varying vec3 Tangents;
flat varying float HasAtmo;
varying float logz_f;
flat varying float FC;

vec2 UnpackFloat32Into2Floats(float i){
    vec2 res;
    res.y = i - floor(i);
    res.x = (i - res.y) / 1000.0;
    res.x = (res.x - 0.5) * 2.0;
    res.y = (res.y - 0.5) * 2.0;
    return res;
}
float scale(float fCos) {   
    float x = 1.0 - fCos;   
    return VertDataScale.y * exp(-0.00287 + x * (0.459 + x * (3.83 + x * (-6.80 + x * 5.25)))); 
}
float getNearIntersection(vec3 _p, vec3 _r, float _d2, float _r2){
    float B = 2.0 * dot(_p, _r);
    float C = _d2 - _r2;
    float fDet = max(0.0, B * B - 4.0 * C);
    return 0.5 * (-B - sqrt(fDet));
}
void main(){
    mat4 MVP = VP * Model;
	WorldPosition = (Model * vec4(position,1.0)).xyz;
    Normals = (Model * vec4(normal.zyx,0.0)).xyz; //Order is ZYXW so to bring it to XYZ we need to use ZYX
    Binormals = (Model * vec4(binormal.zyx,0.0)).xyz; //Order is ZYXW so to bring it to XYZ we need to use ZYX
    Tangents = (Model * vec4(tangent.zyx,0.0)).xyz; //Order is ZYXW so to bring it to XYZ we need to use ZYX
	UV = uv;

    gl_Position = MVP * vec4(position, 1.0);

    //UV = UnpackFloat32Into2Floats(uv);
	VCameraPositionReal = VertDataMisc2.xyz;

	HasAtmo = HasAtmosphere;

    logz_f = 1.0 + gl_Position.w;
    gl_Position.z = (log2(max(1e-6, logz_f)) * fcoeff - 1.0) * gl_Position.w;
    FC = fcoeff;

    if(HasAtmosphere == 1){
		vec3 test = (Rot * vec4(position,1.0)).xyz;
		vec3 v3Pos = test * VertDataRadius.w;
        vec3 v3Ray = v3Pos - VertDataMisc1.xyz;
        float fFar = length(v3Ray);
        v3Ray /= fFar;  
        
        vec3 v3Start;
        if(fromAtmosphere == 0){
            float fNear = getNearIntersection(VertDataMisc1.xyz, v3Ray, VertDataRadius.x, VertDataRadius.z);
            v3Start = VertDataMisc1.xyz + v3Ray * fNear; 
            fFar -= fNear;
        }
        else{
            v3Start = VertDataMisc1.xyz;
        }
        
        vec3 normalSphere = normalize(v3Pos);
        float fDepth = exp((VertDataRadius.w - VertDataRadius.y) / VertDataScale.y);    
        float fCameraAngle = dot(-v3Ray, normalSphere);
		vec3 lightDirection = vec3(VertDataMisc1.w,VertDataMisc2.w,VertDataMisc3.w);
        float fLightAngle = dot(lightDirection, normalSphere);
        float fCameraScale = scale(fCameraAngle);
        float fLightScale = scale(fLightAngle);
        float fCameraOffset = fDepth*fCameraScale;
        float fTemp = (fLightScale + fCameraScale);
    
        float fSampleLength = fFar / VertDataScale.w;
        float fScaledLength = fSampleLength * VertDataScale.x;   
        vec3 v3SampleRay = v3Ray * fSampleLength;   
        vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;   
    
        vec3 v3FrontColor = vec3(0.0);    
        vec3 v3Attenuate = vec3(0.0);
        for(int i = 0; i < nSamples; ++i)   {   
            float fHeight = length(v3SamplePoint);  
            float fDepth = exp(VertDataScale.z * (VertDataRadius.w - fHeight));    
            float fScatter = fDepth*fTemp - fCameraOffset;  
            v3Attenuate = exp(-fScatter * (VertDataMisc3.xyz * VertDatafK.z + VertDatafK.w)); 
            v3FrontColor += v3Attenuate * (fDepth * fScaledLength); 
            v3SamplePoint += v3SampleRay;
        }
        c0 = v3FrontColor * (VertDataMisc3.xyz * VertDatafK.x + VertDatafK.y);  
        c1 = v3Attenuate;
    }
}
