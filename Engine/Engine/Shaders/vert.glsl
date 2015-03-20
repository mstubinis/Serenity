#version 330

layout(location=0)in vec3 position;
layout(location=1)in vec2 uv;
layout(location=2)in vec3 normal;
layout(location=3)in vec3 binormal;
layout(location=4)in vec3 tangent;
layout(location=5)in vec4 color;

uniform mat4 MVP;
uniform mat4 World;

out vec4 Color;
out vec2 UV;

out vec3 Normals;
out vec3 Binormals;
out vec3 Tangents;

void main(){
	gl_Position = MVP * vec4(position, 1.0);

	Color = color;
	UV = uv;

	Normals = (World * vec4(normal,0.0)).xyz;
	Binormals = (World * vec4(binormal,0.0)).xyz;
	Tangents = (World * vec4(tangent,0.0)).xyz;
}