
USE_LOG_DEPTH_FRAGMENT
USE_MAX_MATERIAL_LAYERS_PER_COMPONENT
USE_MAX_MATERIAL_COMPONENTS

struct InData {
    vec2  uv;
    vec4  diffuse;
    vec4  objectColor;
    vec3  normals;
    float glow;
    float specular;
    float ao;
    float metalness;
    float smoothness;
    vec3  worldPosition;
};
struct Layer {
    vec4 data1;
    vec4 data2;
    sampler2D texture;
    sampler2D mask;
    samplerCube cubemap;
    vec2 uvModifications;
};
struct Component {
    int numLayers;
    int componentType;
    Layer layers[MAX_MATERIAL_LAYERS_PER_COMPONENT];
};
uniform Component   components[MAX_MATERIAL_COMPONENTS];
uniform int         numComponents;

uniform vec4        MaterialBasePropertiesOne;
uniform vec4        MaterialBasePropertiesTwo;

uniform int Shadeless;

uniform vec4 Object_Color;
uniform vec4 Material_F0AndID;
uniform vec3 Gods_Rays_Color;

varying vec3 WorldPosition;
varying vec2 UV;
varying vec3 Normals;
varying mat3 TBN;
flat varying vec3 CamPosition;
varying vec3 TangentCameraPos;
varying vec3 TangentFragPos;

void main(){
    InData inData;
    inData.uv = UV;
    inData.diffuse = vec4(0.0,0.0,0.0,0.0001);
    inData.objectColor = Object_Color;
    inData.normals = normalize(Normals);
    inData.glow = MaterialBasePropertiesOne.x;
    inData.specular = 1.0;
    inData.ao = MaterialBasePropertiesOne.y;
    inData.metalness = MaterialBasePropertiesOne.z;
    inData.smoothness = MaterialBasePropertiesOne.w;
    inData.worldPosition = WorldPosition;

    for (int j = 0; j < numComponents; ++j) {
        //ProcessComponentLOD(components[j], inData, 5.0);
        ProcessComponent(components[j], inData);
    }

    float OutMatIDAndAO = Material_F0AndID.w + inData.ao;
    vec2 OutNormals = EncodeOctahedron(inData.normals);
    if(Shadeless == 1){
        OutNormals = ConstantOneVec2;
        inData.diffuse *= (1.0 + inData.glow);
    }
	float OutPackedMetalnessAndSmoothness = Pack2FloatIntoFloat16(inData.metalness,inData.smoothness);
    vec4 GodRays = vec4(Gods_Rays_Color,1.0);
    float GodRaysRG = Pack2NibblesInto8BitChannel(GodRays.r,GodRays.g);
    inData.diffuse.a *= MaterialBasePropertiesTwo.x;
    gl_FragData[0] = inData.diffuse;
    gl_FragData[1] = vec4(OutNormals, OutMatIDAndAO, OutPackedMetalnessAndSmoothness);
    gl_FragData[2] = vec4(inData.glow, inData.specular, GodRaysRG, GodRays.b);
}