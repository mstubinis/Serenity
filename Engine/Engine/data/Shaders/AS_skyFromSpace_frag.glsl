
USE_LOG_DEPTH_FRAGMENT

uniform vec4 FragDataGravity; //g,g*g,exposure,UNUSED

varying vec3 c0;
varying vec3 c1;
varying vec3 v3Direction;
varying vec3 v3LightPosition;
varying float Depth;
varying vec3 WorldPosition;

uniform int HasGodsRays;

const vec4 ConstantZeroVec4 = vec4(0.0,0.0,0.0,0.0);
const vec3 ConstantAlmostOneVec3 = vec3(0.9999,0.9999,0.9999);
const vec3 ConstantOneVec3 = vec3(1.0,1.0,1.0);
const vec2 ConstantOneVec2 = vec2(1.0,1.0);
const vec2 ConstantZeroVec2 = vec2(0.0,0.0);
const vec3 ConstantZeroVec3 = vec3(0.0,0.0,0.0);

float Pack2FloatIntoFloat16(float x,float y){
    x = clamp(x,0.0001,0.9999);
    y = clamp(y,0.0001,0.9999);
    float _x = (x + 1.0) * 0.5;
    float _y = (y + 1.0) * 0.5;
    return floor(_x * 100.0) + _y;
}
vec2 sign_not_zero(vec2 v) {
    return vec2(v.x >= 0 ? 1.0 : -1.0,v.y >= 0 ? 1.0 : -1.0);
}
vec2 EncodeOctahedron(vec3 v) {
    if(  all(greaterThan(v,ConstantAlmostOneVec3))  )
        return ConstantOneVec2;
    v.xy /= dot(abs(v), ConstantOneVec3);
    return mix(v.xy, (1.0 - abs(v.yx)) * sign_not_zero(v.xy), step(v.z, 0.0));
}
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

    //gl_FragColor = vec4(HDR.xyz,nightmult);
    
    gl_FragData[0] = vec4(HDR.xyz,nightmult);
    gl_FragData[1] = vec4(ConstantOneVec2,0.0,1.0); //out normals, out ao, out packed metalness and smoothness
    gl_FragData[2].rg = ConstantZeroVec2;
    if(HasGodsRays == 1){
        gl_FragData[3] = vec4(ConstantZeroVec3,1.0);
    }
}