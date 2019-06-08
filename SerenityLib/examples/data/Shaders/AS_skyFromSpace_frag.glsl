
USE_LOG_DEPTH_FRAGMENT

uniform vec4 FragDataGravity; //g,g*g,exposure,UNUSED

varying vec3 c0;
varying vec3 c1;
varying vec3 v3Direction;
varying vec3 v3LightPosition;
varying float Depth;
varying vec3 WorldPosition;

const vec4 ConstantZeroVec4 = vec4(0.0,0.0,0.0,0.0);
const vec3 ConstantAlmostOneVec3 = vec3(0.9999,0.9999,0.9999);
const vec3 ConstantOneVec3 = vec3(1.0,1.0,1.0);
const vec2 ConstantOneVec2 = vec2(1.0,1.0);
const vec2 ConstantZeroVec2 = vec2(0.0,0.0);
const vec3 ConstantZeroVec3 = vec3(0.0,0.0,0.0);

void main(){
    float fCos = dot(v3LightPosition, v3Direction) / length(v3Direction);
    float fCos2 = fCos * fCos;
    float fRayleighPhase = 0.75 * (1.0 + fCos2);
    float fMiePhase = 1.5 * ((1.0 - FragDataGravity.y) / (2.0 + FragDataGravity.y)) * (1.0 + fCos2) / pow(1.0 + FragDataGravity.y - 2.0 * FragDataGravity.x * fCos, 1.5);
    
    float sun = 0.727272 * (1.0 + fCos2) / pow(1.2 + 0.4 * fCos, 1.0);
    
    vec4 f4Ambient = (sun * Depth) * vec4(0.05, 0.05, 0.1,1.0);
    
    vec4 f4Color = (fRayleighPhase * vec4(c0,1.0) + fMiePhase * vec4(c1,1.0)) + f4Ambient;
    vec4 HDR = 1.0 - exp(f4Color * -FragDataGravity.z);
    float nightmult = clamp(max(HDR.x, max(HDR.y, HDR.z))*1.5,0.0,1.0);

    vec4 GodRays = vec4(ConstantZeroVec3,1.0);
    float GodRaysRG = Pack2NibblesInto8BitChannel(GodRays.r,GodRays.g);
    gl_FragData[0] = vec4(HDR.xyz,nightmult);
    gl_FragData[1] = vec4(ConstantOneVec2,0.0,1.0); //out normals, out matIDandAO, out packed metalness and smoothness
    gl_FragData[2] = vec4(0.0,0.0,GodRaysRG,GodRays.b);
}