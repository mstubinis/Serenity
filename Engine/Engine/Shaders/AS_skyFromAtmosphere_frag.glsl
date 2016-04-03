#version 110

uniform float g;
uniform float g2;
uniform float fExposure;
uniform float far;
uniform float C;

varying vec3 c0;
varying vec3 c1;
varying vec3 v3Direction;
varying vec3 v3LightPosition;
varying float Depth;
varying float outerRadius;
varying float cameraHeight;
varying float planetRadius;

void main(){
	float fCos = dot(v3LightPosition, v3Direction) / length(v3Direction);
	float fRayleighPhase = 0.75 * (1.0 + (fCos*fCos));
	float fMiePhase = 1.5 * ((1.0 - g2) / (2.0 + g2)) * (1.0 + fCos*fCos) / pow(1.0 + g2 - 2.0*g*fCos, 1.5);
	
	float sun = 2.0*((1.0 - 0.2) / (2.0 + 0.2)) * (1.0 + fCos*fCos) / pow(1.0 + 0.2 - 2.0*(-0.2)*fCos, 1.0);
	
	vec4 f4Ambient = (sun * Depth )*vec4(0.05, 0.05, 0.1,1.0);
	
	vec4 f4Color = (fRayleighPhase * vec4(c0,1) + fMiePhase * vec4(c1,1))+f4Ambient;
	vec4 HDR = 1.0 - exp(f4Color * -fExposure);
	float nightmult = clamp(max(HDR.x, max(HDR.y, HDR.z))*1.5,0.0,1.0);

	float camHeightOffGround = cameraHeight - planetRadius;
	float range = outerRadius - planetRadius;
	float alpha = (range-camHeightOffGround) / (range*0.5);

	gl_FragData[0] = vec4(HDR.xyz,nightmult);
	gl_FragData[0].a = clamp(alpha * (gl_FragData[0].rgb * 5.5),0.01,0.99);
	gl_FragData[1].rgb = vec3(1.0);
	gl_FragData[2].r = 0.0;
	gl_FragData[2].b = 0.0;

    const float offset = 1.0;
	gl_FragData[3].r = gl_FragCoord.z;
    gl_FragDepth = (log(C * gl_TexCoord[6].z + offset) / log(C * far + offset));
}