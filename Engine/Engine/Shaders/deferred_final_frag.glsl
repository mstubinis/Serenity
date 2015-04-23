#version 110

uniform sampler2D gColorMap;
uniform sampler2D gLightMap;
uniform sampler2D gSSAOMap;

uniform vec2 gScreenSize;
uniform vec4 gAmbientColor;

vec2 CalcTexCoord(){ return gl_FragCoord.xy / gScreenSize; }
void main(){
	vec2 texCoords = CalcTexCoord();
	vec4 image = texture2D(gColorMap, texCoords);
	vec4 lighting = texture2D(gLightMap, texCoords);
	float ssao = texture2D(gSSAOMap, texCoords);

	if(lighting.r > 0.9999 && lighting.g > 0.9999 && lighting.b > 0.9999)
		gl_FragColor = image;
	else{
		vec4 light = max(gAmbientColor,max(lighting,lighting*ssao));
		vec4 Final = image*light;
		gl_FragColor = Final;
	}
}