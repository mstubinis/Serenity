#version 120

uniform samplerCube Texture;

varying vec3 UV;

void main(){
    gl_FragData[0] = textureCube(Texture, UV);
    gl_FragData[1].rgb = vec3(1.0);
	gl_FragData[2].r = 0.0;
	gl_FragData[2].b = 0.0;
	gl_FragData[3].r = gl_FragCoord.z;
	gl_FragDepth = 0.99999;
}