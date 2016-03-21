#version 120

attribute vec3 position;

uniform mat4 VP;
uniform mat4 World;

uniform int NotFullscreenQuad;

void main(){
	if(NotFullscreenQuad == 0){
		gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
		gl_TexCoord[0] = gl_MultiTexCoord0;
	}
	else{
		//find out why this won't work...
		//mat4 MVP = VP * World;
		//gl_Position = MVP * vec4(position, 1.0);

		gl_Position = (VP * World) * gl_Vertex;
	}
}