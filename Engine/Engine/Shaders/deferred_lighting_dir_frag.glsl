#version 330

struct BaseLight{
    vec3 Color;
    float AmbientIntensity;
    float DiffuseIntensity;
};
struct DirectionalLight{
    BaseLight Base;
    vec3 Direction;
};

uniform sampler2D gNormalMap;
uniform sampler2D gPositionMap;

uniform DirectionalLight gDirectionalLight;

uniform vec3 gEyeWorldPos;
uniform vec2 gScreenSize;
uniform float gMatSpecularIntensity;
uniform float gSpecularPower;
uniform mat4 VPInverse;

layout(location=0)out vec4 LightOut;

vec2 CalcTexCoord(){return gl_FragCoord.xy / gScreenSize;}
vec4 CalcLightInternal(BaseLight Light,vec3 LightDirection,vec3 WorldPos,vec3 Normal){
    vec4 AmbientColor = vec4(Light.Color, 1.0) * Light.AmbientIntensity;
    float DiffuseFactor = dot(Normal, -LightDirection);

    vec4 DiffuseColor  = vec4(0, 0, 0, 0);
    vec4 SpecularColor = vec4(0, 0, 0, 0);

    if (DiffuseFactor > 0.0) {
        DiffuseColor = vec4(Light.Color, 1.0) * Light.DiffuseIntensity * DiffuseFactor;

        vec3 VertexToEye = normalize(gEyeWorldPos - WorldPos);
        vec3 LightReflect = normalize(reflect(LightDirection, Normal));
        float SpecularFactor = dot(VertexToEye, LightReflect);
        SpecularFactor = pow(SpecularFactor, gSpecularPower);
        if (SpecularFactor > 0.0) {
            SpecularColor = vec4(Light.Color, 1.0) * gMatSpecularIntensity * SpecularFactor;
        }
    }

    return (AmbientColor + DiffuseColor + SpecularColor);
}
vec4 CalcDirectionalLight(vec3 WorldPos, vec3 Normal){ return CalcLightInternal(gDirectionalLight.Base,gDirectionalLight.Direction,WorldPos,Normal); }
vec3 decodeLocation(vec2 texCoord){
	vec4 clipSpaceLocation;
    clipSpaceLocation.xy = texCoord * 2.0 - 1.0;
    clipSpaceLocation.z = texture(gPositionMap, texCoord).r * 2.0 - 1.0;
    clipSpaceLocation.w = 1.0;
    vec4 homogenousLocation = VPInverse * clipSpaceLocation;
    return homogenousLocation.xyz / homogenousLocation.w;
}
void main(){
	vec2 texCoord = CalcTexCoord();
	vec3 position = decodeLocation(texCoord);
    vec3 normal = texture2D( gNormalMap, texCoord).xyz * 2 - 1;

	if(normal.r > 0.99 && normal.g > 0.99 && normal.b > 0.99) //find out why normal is not near or equal to white for skybox
		LightOut = vec4(0,0,0,0);
	else
		LightOut = CalcDirectionalLight(position,normalize(normal));
}
