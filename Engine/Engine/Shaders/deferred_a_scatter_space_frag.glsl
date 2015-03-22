#version 330

uniform float g;
uniform float g2;

in vec3 c0;
in vec3 c1;
in vec3 v3Direction;
in vec3 v3LightDirection;

// Calculates the Mie phase function
float getMiePhase(float fCos, float fCos2, float g, float g2){
	return 1.5 * ((1.0 - g2) / (2.0 + g2)) * (1.0 + fCos2) / pow(abs(1.0 + g2 - 2.0*g*fCos), 1.5);
}
// Calculates the Rayleigh phase function
float getRayleighPhase(float fCos2){
	//return 1.0;
	return 0.75 + 0.75*fCos2;
}

void main(){

	float fCos = dot(v3LightDirection, v3Direction) / length(v3Direction);
	float fCos2 = fCos*fCos;
	vec3 color = getRayleighPhase(fCos2) * c0 + getMiePhase(fCos, fCos2, g, g2) * c1;
	vec4 AtmoColor = vec4(color.r,color.g,color.b, color.b);
	vec4 PlanetColor = vec4(1, 0, 0, 1);

	//gl_FragColor = AtmoColor + PlanetColor;
	gl_FragColor = vec4(1, 0, 0, 1);

}