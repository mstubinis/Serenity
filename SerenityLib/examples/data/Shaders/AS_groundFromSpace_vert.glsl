
USE_LOG_DEPTH_VERTEX

layout (location = 0) in vec3 position;
//layout (location = 1) in float uv;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 normal;
layout (location = 3) in vec4 binormal;
layout (location = 4) in vec4 tangent;
layout (location = 5) in vec4 BoneIDs;
layout (location = 6) in vec4 Weights;

uniform int HasAtmosphere;

uniform mat4 Model;
uniform mat4 Rot;

uniform int nSamples;
uniform int fromAtmosphere;

uniform vec4 VertDataMisc1;     //xyz = camPos,             w = lightDir.x
uniform vec4 VertDataMisc2;     //xyz = camPosReal,         w = lightDir.y
uniform vec4 VertDataMisc3;     //xyz = v3InvWaveLength,    w = lightDir.z
uniform vec4 VertDataScale;     //Scale,Scaledepth,ScaleOverScaleDepth,Samples
uniform vec4 VertDataRadius;    //CameraHeight2,OuterRadius,OuterRadius2,InnerRadius
uniform vec4 VertDatafK;        //KrESun,KmESun,Kr4PI,Km4PI

varying vec3 c0;
varying vec3 c1;
varying vec2 UV;
varying vec3 WorldPosition;
varying vec3 VCameraPositionReal;
varying vec3 Normals;
varying mat3 TBN;
flat varying float HasAtmo;

vec2 UnpackFloat32Into2Floats(float i){
    vec2 res;
    res.y = i - floor(i);
    res.x = (i - res.y) / 1000.0;
    res.x = (res.x - 0.5) * 2.0;
    res.y = (res.y - 0.5) * 2.0;
    return res;
}
float scale(float _cos) {   
    float x = 1.0 - _cos;   
    return VertDataScale.y * exp(-0.00287 + x * (0.459 + x * (3.83 + x * (-6.80 + x * 5.25)))); 
}
float getNearIntersection(vec3 _p, vec3 _r, float _d2, float _r2){
    float B = 2.0 * dot(_p, _r);
    float C = _d2 - _r2;
    float _det = max(0.0, B * B - 4.0 * C);
    return 0.5 * (-B - sqrt(_det));
}
void main(){

    mat4 ModelClone = Model;
    ModelClone[3][0] -= CameraRealPosition.x;
    ModelClone[3][1] -= CameraRealPosition.y;
    ModelClone[3][2] -= CameraRealPosition.z;

    WorldPosition = (ModelClone * vec4(position,1.0)).xyz;

          Normals   = (ModelClone * vec4(normal.zyx,0.0)).xyz;
    vec3  Binormals = (ModelClone * vec4(binormal.zyx,0.0)).xyz;
    vec3  Tangents  = (ModelClone * vec4(tangent.zyx,0.0)).xyz;
    TBN = (mat3(Tangents,Binormals,Normals));

    UV = uv;

    gl_Position = CameraViewProj * ModelClone * vec4(position, 1.0);

    //UV = UnpackFloat32Into2Floats(uv);
    VCameraPositionReal = VertDataMisc2.xyz;

    HasAtmo = HasAtmosphere;

    if(HasAtmosphere == 1){
        vec3 test = (Rot * vec4(position,1.0)).xyz;
        vec3 v3Pos = test * VertDataRadius.w;
        vec3 v3Ray = v3Pos - VertDataMisc1.xyz;
        float _far = length(v3Ray);
        v3Ray /= _far;  
        
        vec3 v3Start;
        if(fromAtmosphere == 0){
            float _near = getNearIntersection(VertDataMisc1.xyz, v3Ray, VertDataRadius.x, VertDataRadius.z);
            v3Start = VertDataMisc1.xyz + v3Ray * _near; 
            _far -= _near;
        }else{
            v3Start = VertDataMisc1.xyz;
        }
        
        vec3 normalSphere = normalize(v3Pos);
        float _depth = exp((VertDataRadius.w - VertDataRadius.y) / VertDataScale.y);    
        float _cameraAngle = dot(-v3Ray, normalSphere);
        vec3 lightDirection = vec3(VertDataMisc1.w,VertDataMisc2.w,VertDataMisc3.w);
        float _lightAngle = dot(lightDirection, normalSphere);
        float _cameraScale = scale(_cameraAngle);
        float _lightScale = scale(_lightAngle);
        float _cameraOffset = _depth * _cameraScale;
        float _temp = (_lightScale + _cameraScale);
    
        float _sampleLength = _far / VertDataScale.w;
        float _scaledLength = _sampleLength * VertDataScale.x;   
        vec3 v3SampleRay = v3Ray * _sampleLength;   
        vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;   
    
        vec3 v3FrontColor = vec3(0.0);    
        vec3 v3Attenuate = vec3(0.0);
        for(int i = 0; i < nSamples; ++i)   {   
            float _height = length(v3SamplePoint);  
            float _depth = exp(VertDataScale.z * (VertDataRadius.w - _height));    
            float _scatter = _depth * _temp - _cameraOffset;  
            v3Attenuate = exp(-_scatter * (VertDataMisc3.xyz * VertDatafK.z + VertDatafK.w)); 
            v3FrontColor += v3Attenuate * (_depth * _scaledLength); 
            v3SamplePoint += v3SampleRay;
        }
        c0 = v3FrontColor * (VertDataMisc3.xyz * VertDatafK.x + VertDatafK.y);  
        c1 = v3Attenuate;
    }
}
