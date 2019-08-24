
USE_LOG_DEPTH_FRAGMENT
USE_MAX_MATERIAL_LAYERS_PER_COMPONENT
USE_MAX_MATERIAL_COMPONENTS
USE_MAX_LIGHTS_PER_PASS

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
    vec3  materialF0;
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
struct ImpactPoint {
    vec3 Position;
    vec3 Data; // x = Radius | y = CurrentTime | z = MaxTime
};
uniform ImpactPoint   impacts[60];
uniform int         numImpacts;

uniform Component   components[MAX_MATERIAL_COMPONENTS];
uniform int         numComponents;

uniform vec4        MaterialBasePropertiesOne;
uniform vec4        MaterialBasePropertiesTwo;

uniform int Shadeless;

uniform vec4 Object_Color;
uniform vec4 Material_F0AndID;

varying vec3 WorldPosition;
varying vec2 UV;
varying vec3 Normals;
varying mat3 TBN;
flat varying vec3 CamPosition;
flat varying vec3 CamRealPosition;
varying vec3 TangentCameraPos;
varying vec3 TangentFragPos;

void main(){
    InData inData;
    inData.uv = UV;
    inData.diffuse = vec4(0.0,0.0,0.0,0.0001); //this is extremely wierd, but we need some form of alpha to get painters algorithm to work...
    inData.objectColor = Object_Color;
    inData.normals = normalize(Normals);
    inData.glow = MaterialBasePropertiesOne.x;
    inData.specular = 1.0;
    inData.ao = MaterialBasePropertiesOne.y;
    inData.metalness = MaterialBasePropertiesOne.z;
    inData.smoothness = MaterialBasePropertiesOne.w;
    inData.materialF0 = Material_F0AndID.rgb;

    for (int j = 0; j < numComponents; ++j) {
        ProcessComponent(components[j], inData);
    }
    inData.diffuse.a *= MaterialBasePropertiesTwo.x;

    vec2 encodedNormals = EncodeOctahedron(inData.normals); //yes these two lines are evil and not needed, but they sync up the results with the deferred pass...
    inData.normals = DecodeOctahedron(encodedNormals);

    float totalAlpha = 0.0;
    for (int j = 0; j < numImpacts; ++j) {
        float dist = distance(impacts[j].Position - CamRealPosition, WorldPosition);
        float alpha = smoothstep(1.0, 0.0, dist / impacts[j].Data.x);
        float timeFactor = impacts[j].Data.y / impacts[j].Data.z;
        alpha *= 1.0 - timeFactor;
        totalAlpha += alpha;
    }
    inData.diffuse.a *= clamp(totalAlpha, 0.0, 1.0);
    gl_FragData[0] = inData.diffuse;
    gl_FragData[1] = vec4(encodedNormals, 0.0, 0.0);
    gl_FragData[2] = vec4(inData.glow, inData.specular, 0.0, 0.0);
    gl_FragData[3] = inData.diffuse;
}