
USE_LOG_DEPTH_FRAGMENT

uniform vec4 FragDataGravity; //g,g*g,exposure,UNUSED

varying vec3 c0;
varying vec3 c1;
varying vec3 v3Direction;
varying vec3 v3LightPosition;
varying float Depth;
varying float outerRadius;
varying float cameraHeight;
varying float planetRadius;
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
    
    vec4 f4Color = (fRayleighPhase * vec4(c0, 1.0) + fMiePhase * vec4(c1, 1.0)) + f4Ambient;
    vec4 HDR = 1.0 - exp(f4Color * -FragDataGravity.z);
    float nightmult = clamp(max(HDR.x, max(HDR.y, HDR.z)) * 1.5, 0.0, 1.0);

    float camHeightOffGround = cameraHeight - planetRadius;
    float range = outerRadius - planetRadius;
    float alpha = (range - camHeightOffGround) / (range * 0.5);

    vec4 OutDiffuse;
    OutDiffuse.rgb = vec3(clamp(vec4(HDR.xyz, nightmult), 0.01, 0.99));
    gl_FragData[0] = vec4(OutDiffuse.rgb,clamp(alpha * (OutDiffuse.rgb * 5.5), 0.01, 0.99));
    gl_FragData[1] = vec4(ConstantOneVec2,0.0,1.0); //out normals, out ao, out packed metalness and smoothness

    vec4 GodRays = clamp(vec4(HDR.xyz,nightmult), 0.01, 0.99);
    GodRays = pow(GodRays, vec4(11.0));
    GodRays = clamp(GodRays, 0.01, 0.99);
    GodRays.rgb = max(GodRays.rgb, vec3(0.125, 0.116, 0.25)) * 0.3;
    float GodRaysRG = Pack2NibblesInto8BitChannel(GodRays.r,GodRays.g);
    gl_FragData[2] = vec4(0.0,0.0,GodRaysRG,GodRays.b);
}