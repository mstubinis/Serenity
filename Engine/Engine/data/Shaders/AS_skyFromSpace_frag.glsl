
USE_LOG_DEPTH_FRAGMENT

uniform vec4 FragDataGravity; //g,g*g,exposure,UNUSED

varying vec3 c0;
varying vec3 c1;
varying vec3 v3Direction;
varying vec3 v3LightPosition;
varying float Depth;
varying vec3 WorldPosition;

uniform int HasGodsRays;

const vec3 ConstVec3Zero = vec3(0.0);
const vec2 ConstVec2Zero = vec2(0.0);
const vec2 ConstVec2One = vec2(1.0);

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

    gl_FragColor = vec4(HDR.xyz,nightmult);
    /*
    gl_FragData[0] = vec4(HDR.xyz,nightmult);
    gl_FragData[1].rg = ConstVec2One;
    gl_FragData[2].rg = ConstVec2Zero;
    if(HasGodsRays == 1){
        gl_FragData[3] = vec4(ConstVec3Zero,1.0);
    }
    */
}