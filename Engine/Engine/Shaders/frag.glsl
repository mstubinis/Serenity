#version 330

in vec4 Color;
in vec2 UV;
in vec3 Normal;
in vec3 Binormal;
in vec3 Tangent;

uniform sampler2D DiffuseMap;
uniform sampler2D NormalMap;
uniform int DiffuseMapEnabled;
uniform int NormalMapEnabled;

uniform vec3 Object_Color;

void main(){
	vec4 diffuse = texture(DiffuseMap,UV);
	vec4 normal = texture(NormalMap,UV);

	gl_FragColor = diffuse;
	gl_FragColor *= vec4(Object_Color,1);
}