
#include <serenity/renderer/opengl/glsl/Materials.h>

#include <serenity/renderer/opengl/Extensions.h>
#include <serenity/resources/shader/ShaderHelper.h>
#include <serenity/resources/material/MaterialEnums.h>

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/lexical_cast.hpp>

using namespace Engine;
using namespace Engine::priv;

void opengl::glsl::Materials::convert(std::string& code, uint32_t versionNumber, ShaderType shaderType) {
#pragma region OutputSubmissions
    if (ShaderHelper::lacksDefinition(code, "SUBMIT_DIFFUSE(", "void SUBMIT_DIFFUSE(")) {
        ShaderHelper::insertStringRightBeforeMainFunc(code, R"(
void SUBMIT_DIFFUSE(float r, float g, float b, float a){ gl_FragData[0] = vec4(r, g, b, a); }
void SUBMIT_DIFFUSE(vec4 color){ gl_FragData[0] = color; }
)");
    }
    if (ShaderHelper::lacksDefinition(code, "SUBMIT_NORMALS(", "void SUBMIT_NORMALS(")) {
        ShaderHelper::insertStringRightBeforeMainFunc(code, R"(
void SUBMIT_NORMALS(float x, float y, float z){ gl_FragData[1].xy = EncodeOctahedron(vec3(x, y, z)); }
void SUBMIT_NORMALS(vec3 normals){ gl_FragData[1].xy = EncodeOctahedron(normals); }
void SUBMIT_NORMALS(float xCompressed, float yCompressed){ gl_FragData[1].xy = vec2(xCompressed, yCompressed); }
void SUBMIT_NORMALS(vec2 compressedNormals){ gl_FragData[1].xy = compressedNormals; }
)");
    }
    if (ShaderHelper::lacksDefinition(code, "SUBMIT_GLOW(", "void SUBMIT_GLOW(")) {
        ShaderHelper::insertStringRightBeforeMainFunc(code, R"(
void SUBMIT_GLOW(float glow){ gl_FragData[1].a = glow; }
)");
    }
    if (ShaderHelper::lacksDefinition(code, "SUBMIT_SPECULAR(", "void SUBMIT_SPECULAR(")) {
        ShaderHelper::insertStringRightBeforeMainFunc(code, R"(
void SUBMIT_SPECULAR(float specular){ gl_FragData[2].g = specular; }
)");
    }
    if (ShaderHelper::lacksDefinition(code, "SUBMIT_MATERIAL_ID_AND_AO(", "void SUBMIT_MATERIAL_ID_AND_AO(")) {
        ShaderHelper::insertStringRightBeforeMainFunc(code, R"(
void SUBMIT_MATERIAL_ID_AND_AO(float matID, float ao){ gl_FragData[1].b = matID + ao; }
void SUBMIT_MATERIAL_ID_AND_AO(float matIDAndAO){ gl_FragData[1].b = matIDAndAO; }
)");
    }
    if (ShaderHelper::lacksDefinition(code, "SUBMIT_GOD_RAYS_COLOR(", "void SUBMIT_GOD_RAYS_COLOR(")) {
        ShaderHelper::insertStringRightBeforeMainFunc(code, R"(
void SUBMIT_GOD_RAYS_COLOR(float godRaysR, float godRaysG, float godRaysB){
    float GodRaysRG = Pack2NibblesInto8BitChannel(godRaysR, godRaysG);
    gl_FragData[2].ba = vec2(GodRaysRG, godRaysB);
}
void SUBMIT_GOD_RAYS_COLOR(float godRaysPackedRG, float godRaysB){ gl_FragData[2].ba = vec2(godRaysPackedRG, godRaysB); }
)");
    }
    if (ShaderHelper::lacksDefinition(code, "SUBMIT_METALNESS_AND_SMOOTHNESS(", "void SUBMIT_METALNESS_AND_SMOOTHNESS(")) {
        ShaderHelper::insertStringRightBeforeMainFunc(code, R"(
void SUBMIT_METALNESS_AND_SMOOTHNESS(float inMetal, float inSmooth){
    float PackedMetalSmooth = Pack2NibblesInto8BitChannel(inMetal, inSmooth);
    gl_FragData[2].r = PackedMetalSmooth;
}
void SUBMIT_METALNESS_AND_SMOOTHNESS(float inPackedMetalSmooth){ gl_FragData[2].r = inPackedMetalSmooth; }
)");
    }
#pragma endregion

#pragma region process component

    if (ShaderHelper::sfind(code, "ProcessComponentDiffuse(") || ShaderHelper::sfind(code, "ProcessComponentDiffuseLOD(")) {
        if (!ShaderHelper::sfind(code, "void ProcessComponentDiffuse(")) {
            std::string newCode = R"(
void ProcessComponentDiffuse(Component inComponent, inout InData data) {
    vec4 calculatedDiffuse = data.diffuse;
)";
for (int i = 0; i < MAX_MATERIAL_LAYERS_PER_COMPONENT; ++i) {
    const std::string idx = std::to_string(i);
    newCode += "calculatedDiffuse = inComponent.layers[" + idx + "].data0.x == 1.0 ? CalculateDiffuse(inComponent.layers[" + idx + "], calculatedDiffuse, data.objectColor, data.uv) : calculatedDiffuse;\n";
}
newCode += R"(
    data.diffuse = inComponent.componentData.y == 1.0 ? calculatedDiffuse : data.diffuse;
}
void ProcessComponentDiffuseLOD(Component inComponent, inout InData data, float lod) {
    vec4 calculatedDiffuse = data.diffuse;
)";
for (int i = 0; i < MAX_MATERIAL_LAYERS_PER_COMPONENT; ++i) {
    const std::string idx = std::to_string(i);
    newCode += "calculatedDiffuse = inComponent.layers[" + idx + "].data0.x == 1.0 ? CalculateDiffuseLOD(inComponent.layers[" + idx + "], calculatedDiffuse, data.objectColor, data.uv, lod) : calculatedDiffuse;\n";
}
newCode += R"(
    data.diffuse = inComponent.componentData.y == 1.0 ? calculatedDiffuse: data.diffuse;
}
)";
            ShaderHelper::insertStringRightBeforeLineContent(code, std::move(newCode), " main(");
        }
    }

    if (ShaderHelper::sfind(code, "ProcessComponentNormal(") || ShaderHelper::sfind(code, "ProcessComponentNormalLOD(")) {
        if (!ShaderHelper::sfind(code, "void ProcessComponentNormal(")) {
            std::string newCode = R"(
void ProcessComponentNormal(Component inComponent, inout InData data) {
    vec3 calculatedNormals = data.normals;
)";
for (int i = 0; i < MAX_MATERIAL_LAYERS_PER_COMPONENT; ++i) {
    const std::string idx = std::to_string(i);
    newCode += "calculatedNormals = inComponent.layers[" + idx + "].data0.x == 1.0 ? CalculateNormals(inComponent.layers[" + idx + "], calculatedNormals, data.uv) : calculatedNormals;\n";
}
newCode += R"(
    data.normals = inComponent.componentData.y == 1.0 ? calculatedNormals: data.normals;
}
void ProcessComponentNormalLOD(Component inComponent, inout InData data, float lod) {
    vec3 calculatedNormals = data.normals;

)";
for (int i = 0; i < MAX_MATERIAL_LAYERS_PER_COMPONENT; ++i) {
    const std::string idx = std::to_string(i);
    newCode += "calculatedNormals = inComponent.layers[" + idx + "].data0.x == 1.0 ? CalculateNormalsLOD(inComponent.layers[" + idx + "], calculatedNormals, data.uv, lod) : calculatedNormals;\n";
}
newCode += R"(
    data.normals = inComponent.componentData.y == 1.0 ? calculatedNormals : data.normals;
}
)";
            ShaderHelper::insertStringRightBeforeLineContent(code, std::move(newCode), " main(");
        }
    }

    if (ShaderHelper::sfind(code, "ProcessComponentGlow(") || ShaderHelper::sfind(code, "ProcessComponentGlowLOD(")) {
        if (!ShaderHelper::sfind(code, "void ProcessComponentGlow(")) {

            std::string newCode = R"(
void ProcessComponentGlow(Component inComponent, inout InData data) {
    float calculatedGlow = data.glow;
)";
for (int i = 0; i < MAX_MATERIAL_LAYERS_PER_COMPONENT; ++i) {
    const std::string idx = std::to_string(i);
    newCode += "calculatedGlow = inComponent.layers[" + idx + "].data0.x == 1.0 ? CalculateGlow(inComponent.layers[" + idx + "], calculatedGlow, data.uv) : calculatedGlow;\n";
}
newCode += R"(
    data.glow = inComponent.componentData.y == 1.0 ? calculatedGlow : data.glow;
}
void ProcessComponentGlowLOD(Component inComponent, inout InData data, float lod) {
    float calculatedGlow = data.glow;

)";
for (int i = 0; i < MAX_MATERIAL_LAYERS_PER_COMPONENT; ++i) {
    const std::string idx = std::to_string(i);
    newCode += "calculatedGlow = inComponent.layers[" + idx + "].data0.x == 1.0 ? CalculateGlowLOD(inComponent.layers[" + idx + "], calculatedGlow, data.uv, lod) : calculatedGlow;\n";
}
newCode += R"(
    data.glow = inComponent.componentData.y == 1.0 ? calculatedGlow : data.glow;
}
)";
            ShaderHelper::insertStringRightBeforeLineContent(code, std::move(newCode), " main(");
        }
    }

    if (ShaderHelper::sfind(code, "ProcessComponentSpecular(") || ShaderHelper::sfind(code, "ProcessComponentSpecularLOD(")) {
        if (!ShaderHelper::sfind(code, "void ProcessComponentSpecular(")) {
            std::string newCode = R"(
void ProcessComponentSpecular(Component inComponent, inout InData data) {
    float calculatedSpecular = data.specular;

)";
for (int i = 0; i < MAX_MATERIAL_LAYERS_PER_COMPONENT; ++i) {
    const std::string idx = std::to_string(i);
    newCode += "calculatedSpecular = inComponent.layers[" + idx + "].data0.x == 1.0 ? CalculateSpecular(inComponent.layers[" + idx + "], calculatedSpecular, data.uv) : calculatedSpecular;\n";
}
newCode += R"(
    data.specular = inComponent.componentData.y == 1.0 ? calculatedSpecular : data.specular;
}
void ProcessComponentSpecularLOD(Component inComponent, inout InData data, float lod) {
    float calculatedSpecular = data.specular;

)";
for (int i = 0; i < MAX_MATERIAL_LAYERS_PER_COMPONENT; ++i) {
    const std::string idx = std::to_string(i);
    newCode += "calculatedSpecular = inComponent.layers[" + idx + "].data0.x == 1.0 ? CalculateSpecularLOD(inComponent.layers[" + idx + "], calculatedSpecular, data.uv, lod) : calculatedSpecular;\n";
}
newCode += R"(
    data.specular = inComponent.componentData.y == 1.0 ? calculatedSpecular : data.specular;
}
)";
            ShaderHelper::insertStringRightBeforeLineContent(code, std::move(newCode), " main(");
        }
    }

    if (ShaderHelper::sfind(code, "ProcessComponentMetalness(") || ShaderHelper::sfind(code, "ProcessComponentMetalnessLOD(")) {
        if (!ShaderHelper::sfind(code, "void ProcessComponentMetalness(")) {
            std::string newCode = R"(
void ProcessComponentMetalness(Component inComponent, inout InData data) {
    float calculatedMetalness = data.metalness;
)";
for (int i = 0; i < MAX_MATERIAL_LAYERS_PER_COMPONENT; ++i) {
    const std::string idx = std::to_string(i);
    newCode += "calculatedMetalness = inComponent.layers[" + idx + "].data0.x == 1.0 ? CalculateMetalness(inComponent.layers[" + idx + "], calculatedMetalness, data.uv) : calculatedMetalness;\n";
}
newCode += R"(
    data.metalness = inComponent.componentData.y == 1.0 ? calculatedMetalness : data.metalness;
}
void ProcessComponentMetalnessLOD(Component inComponent, inout InData data, float lod) {
    float calculatedMetalness = data.metalness;
)";
for (int i = 0; i < MAX_MATERIAL_LAYERS_PER_COMPONENT; ++i) {
    const std::string idx = std::to_string(i);
    newCode += "calculatedMetalness = inComponent.layers[" + idx + "].data0.x == 1.0 ? CalculateMetalnessLOD(inComponent.layers[" + idx + "], calculatedMetalness, data.uv, lod) : calculatedMetalness;\n";
}
newCode += R"(
    data.metalness = inComponent.componentData.y == 1.0 ? calculatedMetalness : data.metalness;
}
)";
            ShaderHelper::insertStringRightBeforeLineContent(code, std::move(newCode), " main(");
        }
    }

    if (ShaderHelper::sfind(code, "ProcessComponentSmoothness(") || ShaderHelper::sfind(code, "ProcessComponentSmoothnessLOD(")) {
        if (!ShaderHelper::sfind(code, "void ProcessComponentSmoothness(")) {
            std::string newCode = R"(
void ProcessComponentSmoothness(Component inComponent, inout InData data) {
    float calculatedSmoothness = data.smoothness;
)";
for (int i = 0; i < MAX_MATERIAL_LAYERS_PER_COMPONENT; ++i) {
    const std::string idx = std::to_string(i);
    newCode += "calculatedSmoothness = inComponent.layers[" + idx + "].data0.x == 1.0 ? CalculateSmoothness(inComponent.layers[" + idx + "], calculatedSmoothness, data.uv) : calculatedSmoothness;\n";
}
newCode += R"(
    data.smoothness = inComponent.componentData.y == 1.0 ? calculatedSmoothness : data.smoothness;
}
void ProcessComponentSmoothnessLOD(Component inComponent, inout InData data, float lod) {
    float calculatedSmoothness = data.smoothness;
)";
for (int i = 0; i < MAX_MATERIAL_LAYERS_PER_COMPONENT; ++i) {
    const std::string idx = std::to_string(i);
    newCode += "calculatedSmoothness = inComponent.layers[" + idx + "].data0.x == 1.0 ? CalculateSmoothnessLOD(inComponent.layers[" + idx + "], calculatedSmoothness, data.uv, lod) : calculatedSmoothness;\n";
}
newCode += R"(
    data.smoothness = inComponent.componentData.y == 1.0 ? calculatedSmoothness : data.smoothness;
}
)";
            ShaderHelper::insertStringRightBeforeLineContent(code, std::move(newCode), " main(");
        }
    }

    if (ShaderHelper::sfind(code, "ProcessComponentAO(") || ShaderHelper::sfind(code, "ProcessComponentAOLOD(")) {
        if (!ShaderHelper::sfind(code, "void ProcessComponentAO(")) {
            std::string newCode = R"(
void ProcessComponentAO(Component inComponent, inout InData data) {
    float calculatedAO = data.ao;
)";
for (int i = 0; i < MAX_MATERIAL_LAYERS_PER_COMPONENT; ++i) {
    const std::string idx = std::to_string(i);
    newCode += "calculatedAO = inComponent.layers[" + idx + "].data0.x == 1.0 ? CalculateAO(inComponent.layers[" + idx + "], calculatedAO, data.uv) : calculatedAO;\n";
}
newCode += R"(
    data.ao = inComponent.componentData.y == 1.0 ? calculatedAO : data.ao;
}
void ProcessComponentAOLOD(Component inComponent, inout InData data, float lod) {
    float calculatedAO = data.ao;
)";
for (int i = 0; i < MAX_MATERIAL_LAYERS_PER_COMPONENT; ++i) {
    const std::string idx = std::to_string(i);
    newCode += "calculatedAO = inComponent.layers[" + idx + "].data0.x == 1.0 ? CalculateAOLOD(inComponent.layers[" + idx + "], calculatedAO, data.uv, lod) : calculatedAO;\n";
}
newCode += R"(
    data.ao = inComponent.componentData.y == 1.0 ? calculatedAO : data.ao;
}
)";
            ShaderHelper::insertStringRightBeforeLineContent(code, std::move(newCode), " main(");
        }
    }

    if (ShaderHelper::sfind(code, "ProcessComponentReflection(") || ShaderHelper::sfind(code, "ProcessComponentReflectionLOD(")) {
        if (!ShaderHelper::sfind(code, "void ProcessComponentReflection(")) {
            std::string newCode = R"(
void ProcessComponentReflection(Component inComponent, inout InData data) {
    vec4 calculatedReflection = data.diffuse;
)";
for (int i = 0; i < MAX_MATERIAL_LAYERS_PER_COMPONENT; ++i) {
    const std::string idx = std::to_string(i);
    newCode += "calculatedReflection = inComponent.layers[" + idx + "].data0.x == 1.0 ? CalculateReflection(inComponent.layers[" + idx + "], data.uv, calculatedReflection, CamPosition, data.normals, data.worldPosition) : calculatedReflection;\n";
}
newCode += R"(
    data.diffuse = inComponent.componentData.y == 1.0 ? calculatedReflection : data.diffuse;
}
void ProcessComponentReflectionLOD(Component inComponent, inout InData data, float lod) {
    vec4 calculatedReflection = data.diffuse;
)";
for (int i = 0; i < MAX_MATERIAL_LAYERS_PER_COMPONENT; ++i) {
    const std::string idx = std::to_string(i);
    newCode += "calculatedReflection = inComponent.layers[" + idx + "].data0.x == 1.0 ? CalculateReflectionLOD(inComponent.layers[" + idx + "], data.uv, calculatedReflection, CamPosition, data.normals, data.worldPosition, lod) : calculatedReflection;\n";
}
newCode += R"(
    data.diffuse = inComponent.componentData.y == 1.0 ? calculatedReflection : data.diffuse;
}
)";
            ShaderHelper::insertStringRightBeforeLineContent(code, std::move(newCode), " main(");
        }
    }

    if (ShaderHelper::sfind(code, "ProcessComponentRefraction(") || ShaderHelper::sfind(code, "ProcessComponentRefractionLOD(")) {
        if (!ShaderHelper::sfind(code, "void ProcessComponentRefraction(")) {
            std::string newCode = R"(
void ProcessComponentRefraction(Component inComponent, inout InData data) {
    vec4 calculatedRefraction = data.diffuse;
)";
for (int i = 0; i < MAX_MATERIAL_LAYERS_PER_COMPONENT; ++i) {
    const std::string idx = std::to_string(i);
    newCode += "calculatedRefraction = inComponent.layers[" + idx + "].data0.x == 1.0 ? CalculateRefraction(inComponent.layers[" + idx + "], data.uv, calculatedRefraction, CamPosition, data.normals, data.worldPosition) : calculatedRefraction;\n";
}
newCode += R"(
    data.diffuse = inComponent.componentData.y == 1.0 ? calculatedRefraction : data.diffuse;
}
void ProcessComponentRefractionLOD(Component inComponent, inout InData data, float lod) {
    vec4 calculatedRefraction = data.diffuse;
)";
for (int i = 0; i < MAX_MATERIAL_LAYERS_PER_COMPONENT; ++i) {
    const std::string idx = std::to_string(i);
    newCode += "calculatedRefraction = inComponent.layers[" + idx + "].data0.x == 1.0 ? CalculateRefractionLOD(inComponent.layers[" + idx + "], data.uv, calculatedRefraction, CamPosition, data.normals, data.worldPosition, lod) : calculatedRefraction;\n";
}
newCode += R"(
    data.diffuse = inComponent.componentData.y == 1.0 ? calculatedRefraction : data.diffuse;
}
)";
            ShaderHelper::insertStringRightBeforeLineContent(code, std::move(newCode), " main(");
        }
    }

    if (ShaderHelper::sfind(code, "ProcessComponentParallax(") || ShaderHelper::sfind(code, "ProcessComponentParallaxLOD(")) {
        if (!ShaderHelper::sfind(code, "void ProcessComponentParallax(")) {
            std::string newCode = R"(
void ProcessComponentParallax(Component inComponent, inout InData data) {
    vec2 calculatedUV = data.uv;
    vec3 ViewDir = normalize(TangentCameraPos - TangentFragPos);
)";
for (int i = 0; i < MAX_MATERIAL_LAYERS_PER_COMPONENT; ++i) {
    const std::string idx = std::to_string(i);
    newCode += "calculatedUV = inComponent.layers[" + idx + "].data0.x == 1.0 ? CalculateParallaxMap(inComponent.layers[" + idx + "], ViewDir, calculatedUV) : calculatedUV;\n";
}
newCode += R"(
    data.uv = inComponent.componentData.y == 1.0 ? calculatedUV : data.uv;
}
void ProcessComponentParallaxLOD(Component inComponent, inout InData data, float lod) {
    vec2 calculatedUV = data.uv;
    vec3 ViewDir = normalize(TangentCameraPos - TangentFragPos);
)";
for (int i = 0; i < MAX_MATERIAL_LAYERS_PER_COMPONENT; ++i) {
    const std::string idx = std::to_string(i);
    newCode += "calculatedUV = inComponent.layers[" + idx + "].data0.x == 1.0 ? CalculateParallaxMapLOD(inComponent.layers[" + idx + "], ViewDir, calculatedUV, lod) : calculatedUV;\n";
}
newCode += R"(
    data.uv = inComponent.componentData.y == 1.0 ? calculatedUV : data.uv;
}
)";
            ShaderHelper::insertStringRightBeforeLineContent(code, std::move(newCode), " main(");
        }
    }

#pragma endregion

#pragma region calculate diffuse

    if (ShaderHelper::sfind(code, "CalculateDiffuse(") || ShaderHelper::sfind(code, "CalculateDiffuseLOD(")) {
        if (!ShaderHelper::sfind(code, "vec4 CalculateDiffuse(")) {
            ShaderHelper::insertStringRightBeforeLineContent(code, R"(
vec4 CalculateDiffuse(Layer inLayer, vec4 canvas, vec4 objectColor, vec2 inUVs) {
    vec4 paint = objectColor;
    if (inLayer.data1.y >= 0.5) {
        paint *= texture2D(inLayer.texture, inUVs * inLayer.uvModifications.zw + inLayer.uvModifications.xy);
    }else{
        paint.rgb *= Material_F0AndID.rgb;
    }
    if (inLayer.data1.z >= 0.5) {
        paint *= texture2D(inLayer.mask, inUVs).r;
    }
    paint *= inLayer.data2;
    return DoBlend(paint, canvas, inLayer);
}
vec4 CalculateDiffuseLOD(Layer inLayer, vec4 canvas, vec4 objectColor, vec2 inUVs, float lod) {
    vec4 paint = objectColor;
    if (inLayer.data1.y >= 0.5) {
        paint *= texture2DLod(inLayer.texture, inUVs * inLayer.uvModifications.zw + inLayer.uvModifications.xy, lod);
    }else{
        paint.rgb *= Material_F0AndID.rgb;
    }
    if (inLayer.data1.z >= 0.5) {
        paint *= texture2DLod(inLayer.mask, inUVs, lod).r;
    }
    paint *= inLayer.data2;
    return DoBlend(paint, canvas, inLayer);
}
)", "void ProcessComponentDiffuse(");
        }
    }
#pragma endregion

#pragma region calculate normals
    if (ShaderHelper::sfind(code, "CalculateNormals(") || ShaderHelper::sfind(code, "CalculateNormalsLOD(")) {
        if (!ShaderHelper::sfind(code, "vec3 CalculateNormals(")) {
            ShaderHelper::insertStringRightBeforeLineContent(code, R"(
vec3 CalculateNormals(Layer inLayer, vec3 objectNormals, vec2 inUVs) {
    vec3 outNormals = objectNormals;
    if (inLayer.data1.y > 0.9) {
        outNormals = CalcBumpedNormal(inUVs * inLayer.uvModifications.zw + inLayer.uvModifications.xy, inLayer.texture);
    }else if (inLayer.data1.y > 0.4) {
        outNormals = CalcBumpedNormalCompressed(inUVs * inLayer.uvModifications.zw + inLayer.uvModifications.xy, inLayer.texture);
    }
    if (inLayer.data1.z >= 0.5) {
        outNormals *= texture2D(inLayer.mask, inUVs).r;
    }
    outNormals *= inLayer.data2.xyz;
    outNormals *= inLayer.data2.w;
    return outNormals;
}
vec3 CalculateNormalsLOD(Layer inLayer, vec3 objectNormals, vec2 inUVs, float lod) {
    vec3 outNormals = objectNormals;
    if (inLayer.data1.y > 0.9) {
        outNormals = CalcBumpedNormalLOD(inUVs * inLayer.uvModifications.zw + inLayer.uvModifications.xy, inLayer.texture, lod);
    }else if (inLayer.data1.y > 0.4) {
        outNormals = CalcBumpedNormalCompressedLOD(inUVs * inLayer.uvModifications.zw + inLayer.uvModifications.xy, inLayer.texture, lod);
    }
    if (inLayer.data1.z >= 0.5) {
        outNormals *= texture2D(inLayer.mask, inUVs).r;
    }
    outNormals *= inLayer.data2.xyz;
    outNormals *= inLayer.data2.w;
    return outNormals;
}
)", "void ProcessComponentNormal(");
        }
    }
#pragma endregion

#pragma region calculate glow
    if (ShaderHelper::sfind(code, "CalculateGlow(") || ShaderHelper::sfind(code, "CalculateGlowLOD(")) {
        if (!ShaderHelper::sfind(code, "float CalculateGlow(")) {
            ShaderHelper::insertStringRightBeforeLineContent(code, R"(
float CalculateGlow(Layer inLayer, float objectGlow, vec2 inUVs) {
    float outGlow = objectGlow;
    if (inLayer.data1.y >= 0.5) {
        outGlow += texture2D(inLayer.texture, inUVs * inLayer.uvModifications.zw + inLayer.uvModifications.xy).r;
    }
    if (inLayer.data1.z >= 0.5) {
        outGlow *= texture2D(inLayer.mask, inUVs).r;
    }
    outGlow = clamp(outGlow, inLayer.data2.x, inLayer.data2.y);
    outGlow *= inLayer.data2.z;
    return outGlow;
}
float CalculateGlowLOD(Layer inLayer, float objectGlow, vec2 inUVs, float lod) {
    float outGlow = objectGlow;
    if (inLayer.data1.y >= 0.5) {
        outGlow += texture2DLod(inLayer.texture, inUVs * inLayer.uvModifications.zw + inLayer.uvModifications.xy, lod).r;
    }
    if (inLayer.data1.z >= 0.5) {
        outGlow *= texture2DLod(inLayer.mask, inUVs, lod).r;
    }
    outGlow = clamp(outGlow, inLayer.data2.x, inLayer.data2.y);
    outGlow *= inLayer.data2.z;
    return outGlow;
}
)", "void ProcessComponentGlow(");
        }
    }
#pragma endregion

#pragma region calculate specular
    if (ShaderHelper::sfind(code, "CalculateSpecular(") || ShaderHelper::sfind(code, "CalculateSpecularLOD(")) {
        if (!ShaderHelper::sfind(code, "float CalculateSpecular(")) {
            ShaderHelper::insertStringRightBeforeLineContent(code, R"(
float CalculateSpecular(Layer inLayer, float objectSpecular, vec2 inUVs) {
    float outSpecular = objectSpecular;
    if (inLayer.data1.y >= 0.5) {
        outSpecular *= texture2D(inLayer.texture, inUVs * inLayer.uvModifications.zw + inLayer.uvModifications.xy).r;
    }
    if (inLayer.data1.z >= 0.5) {
        outSpecular *= texture2D(inLayer.mask, inUVs).r;
    }
    outSpecular = clamp(outSpecular, inLayer.data2.x, inLayer.data2.y);
    outSpecular *= inLayer.data2.z;
    return outSpecular;
}
float CalculateSpecularLOD(Layer inLayer, float objectSpecular, vec2 inUVs, float lod) {
    float outSpecular = objectSpecular;
    if (inLayer.data1.y >= 0.5) {
        outSpecular *= texture2DLod(inLayer.texture, inUVs * inLayer.uvModifications.zw + inLayer.uvModifications.xy, lod).r;
    }
    if (inLayer.data1.z >= 0.5) {
        outSpecular *= texture2DLod(inLayer.mask, inUVs, lod).r;
    }
    outSpecular = clamp(outSpecular, inLayer.data2.x, inLayer.data2.y);
    outSpecular *= inLayer.data2.z;
    return outSpecular;
}
)", "void ProcessComponentSpecular(");
        }
    }
#pragma endregion

#pragma region calculate ao
    if (ShaderHelper::sfind(code, "CalculateAO(") || ShaderHelper::sfind(code, "CalculateAOLOD(")) {
        if (!ShaderHelper::sfind(code, "float CalculateAO(")) {
            ShaderHelper::insertStringRightBeforeLineContent(code, R"(
float CalculateAO(Layer inLayer, float objectAO, vec2 inUVs) {
    float outAO = objectAO;
    if (inLayer.data1.y >= 0.5) {
        outAO *= texture2D(inLayer.texture, inUVs * inLayer.uvModifications.zw + inLayer.uvModifications.xy).r;
    }
    if (inLayer.data1.z >= 0.5) {
        outAO *= texture2D(inLayer.mask, inUVs).r;
    }
    outAO = clamp(outAO, inLayer.data2.x, inLayer.data2.y);
    outAO *= inLayer.data2.z;
    return outAO;
}
float CalculateAOLOD(Layer inLayer, float objectAO, vec2 inUVs, float lod) {
    float outAO = objectAO;
    if (inLayer.data1.y >= 0.5) {
        outAO *= texture2DLod(inLayer.texture, inUVs * inLayer.uvModifications.zw + inLayer.uvModifications.xy, lod).r;
    }
    if (inLayer.data1.z >= 0.5) {
        outAO *= texture2DLod(inLayer.mask, inUVs, lod).r;
    }
    outAO = clamp(outAO, inLayer.data2.x, inLayer.data2.y);
    outAO *= inLayer.data2.z;
    return outAO;
}
)", "void ProcessComponentAO(");
        }
    }
#pragma endregion

#pragma region calculate metalness
    if (ShaderHelper::sfind(code, "CalculateMetalness(") || ShaderHelper::sfind(code, "CalculateMetalnessLOD(")) {
        if (!ShaderHelper::sfind(code, "float CalculateMetalness(")) {
            ShaderHelper::insertStringRightBeforeLineContent(code, R"(
float CalculateMetalness(Layer inLayer, float objectMetalness, vec2 inUVs) {
    float outMetalness = objectMetalness;
    if (inLayer.data1.y >= 0.5) {
        outMetalness = texture2D(inLayer.texture, inUVs * inLayer.uvModifications.zw + inLayer.uvModifications.xy).r;
    }
    if (inLayer.data1.z >= 0.5) {
        outMetalness *= texture2D(inLayer.mask, inUVs).r;
    }
    outMetalness = clamp(outMetalness, inLayer.data2.x, inLayer.data2.y);
    outMetalness *= inLayer.data2.z;
    return outMetalness;
}
float CalculateMetalnessLOD(Layer inLayer, float objectMetalness, vec2 inUVs, float lod) {
    float outMetalness = objectMetalness;
    if (inLayer.data1.y >= 0.5) {
        outMetalness = texture2DLod(inLayer.texture, inUVs * inLayer.uvModifications.zw + inLayer.uvModifications.xy, lod).r;
    }
    if (inLayer.data1.z >= 0.5) {
        outMetalness *= texture2DLod(inLayer.mask, inUVs, lod).r;
    }
    outMetalness = clamp(outMetalness, inLayer.data2.x, inLayer.data2.y);
    outMetalness *= inLayer.data2.z;
    return outMetalness;
}
)", "void ProcessComponentMetalness(");
        }
    }
#pragma endregion

#pragma region calculate smoothness
    if (ShaderHelper::sfind(code, "CalculateSmoothness(") || ShaderHelper::sfind(code, "CalculateSmoothnessLOD(")) {
        if (!ShaderHelper::sfind(code, "float CalculateSmoothness(")) {
            ShaderHelper::insertStringRightBeforeLineContent(code, R"(
float CalculateSmoothness(Layer inLayer, float objectSmoothness, vec2 inUVs) {
    float outSmoothness = objectSmoothness;
    if (inLayer.data1.y >= 0.5) {
        outSmoothness = texture2D(inLayer.texture, inUVs * inLayer.uvModifications.zw + inLayer.uvModifications.xy).r;
    }
    if (inLayer.data1.z >= 0.5) {
        outSmoothness *= texture2D(inLayer.mask, inUVs).r;
    }
    outSmoothness = clamp(outSmoothness, inLayer.data2.x, inLayer.data2.y);
    outSmoothness *= inLayer.data2.z;
    return outSmoothness;
}
float CalculateSmoothnessLOD(Layer inLayer, float objectSmoothness, vec2 inUVs, float lod) {
    float outSmoothness = objectSmoothness;
    if (inLayer.data1.y >= 0.5) {
        outSmoothness = texture2DLod(inLayer.texture, inUVs * inLayer.uvModifications.zw + inLayer.uvModifications.xy, lod).r;
    }
    if (inLayer.data1.z >= 0.5) {
        outSmoothness *= texture2DLod(inLayer.mask, inUVs, lod).r;
    }
    outSmoothness = clamp(outSmoothness, inLayer.data2.x, inLayer.data2.y);
    outSmoothness *= inLayer.data2.z;
    return outSmoothness;
}
)", "void ProcessComponentSmoothness(");
        }
    }
#pragma endregion

#pragma region calculate reflection
    if (ShaderHelper::sfind(code, "CalculateReflection(") || ShaderHelper::sfind(code, "CalculateReflectionLOD(")) {
        if (!ShaderHelper::sfind(code, "vec4 CalculateReflection(")) {
            ShaderHelper::insertStringRightBeforeLineContent(code, R"(
vec4 CalculateReflection(Layer inLayer, vec2 inUVs, vec4 inDiffuse, vec3 inCameraPosition, vec3 inNormals, vec3 inWorldPosition){
//    inUVs *= inLayer.uvModifications.zw;
//    inUVs += inLayer.uvModifications.xy;
    vec4 r = vec4(0.0);
    r = textureCube(inLayer.cubemap, reflect(inNormals,normalize(inCameraPosition - inWorldPosition))) * texture2D(inLayer.mask, inUVs).r;
    r.a *= inLayer.data2.x;
    r = PaintersAlgorithm(r, inDiffuse);
    return r;
}
vec4 CalculateReflectionLOD(Layer inLayer, vec2 inUVs, vec4 inDiffuse, vec3 inCameraPosition, vec3 inNormals, vec3 inWorldPosition, float lod){
//    inUVs *= inLayer.uvModifications.zw;
//    inUVs += inLayer.uvModifications.xy;
    vec4 r = vec4(0.0);
    r = textureCubeLod(inLayer.cubemap, reflect(inNormals,normalize(inCameraPosition - inWorldPosition)), lod) * texture2D(inLayer.mask, inUVs).r;
    r.a *= inLayer.data2.x;
    r = PaintersAlgorithm(r, inDiffuse);
    return r;
}
)", "void ProcessComponentReflection(");
        }
    }
#pragma endregion

#pragma region calculate refraction
    if (ShaderHelper::sfind(code, "CalculateRefraction(") || ShaderHelper::sfind(code, "CalculateRefractionLOD(")) {
        if (!ShaderHelper::sfind(code, "vec4 CalculateRefraction(")) {
            ShaderHelper::insertStringRightBeforeLineContent(code, R"(
vec4 CalculateRefraction(Layer inLayer, vec2 inUVs, vec4 inDiffuse, vec3 inCameraPosition, vec3 inNormals, vec3 inWorldPosition){
//    inUVs *= inLayer.uvModifications.zw;
//    inUVs += inLayer.uvModifications.xy;
    vec4 r = vec4(0.0);
    r = textureCube(inLayer.cubemap,refract(inNormals,normalize(inCameraPosition - inWorldPosition),1.0 / inLayer.data2.y)) * texture2D(inLayer.mask,inUVs).r;
    r.a *= inLayer.data2.x;
    r = PaintersAlgorithm(r, inDiffuse);
    return r;
}
vec4 CalculateRefractionLOD(Layer inLayer, vec2 inUVs, vec4 inDiffuse, vec3 inCameraPosition, vec3 inNormals, vec3 inWorldPosition, float lod){
//    inUVs *= inLayer.uvModifications.zw;
//    inUVs += inLayer.uvModifications.xy;
    vec4 r = vec4(0.0);
    r = textureCubeLod(inLayer.cubemap,refract(inNormals, normalize(inCameraPosition - inWorldPosition), 1.0 / inLayer.data2.y), lod) * texture2D(inLayer.mask, inUVs).r;
    r.a *= inLayer.data2.x;
    r = PaintersAlgorithm(r, inDiffuse);
    return r;
}
)", "void ProcessComponentRefraction(");
        }
    }
#pragma endregion

#pragma region calculate parallax 
    if (ShaderHelper::sfind(code, "CalculateParallaxMap(") || ShaderHelper::sfind(code, "CalculateParallaxMapLOD(")) {
        if (!ShaderHelper::sfind(code, "vec2 CalculateParallaxMap(")) {
            ShaderHelper::insertStringRightBeforeLineContent(code, 
                "vec2 CalculateParallaxMap(Layer inLayer, vec3 _ViewDir, vec2 inUVs){\n"
                //"    inUVs *= inLayer.uvModifications.zw;\n"
                //"    inUVs += inLayer.uvModifications.xy;\n"
                "    float minLayers = min(5.0,(5.0 * inLayer.data2.x) + 1.0);\n"
                "    float maxLayers = min(30.0,(30.0 * inLayer.data2.x) + 1.0);\n"
                "    float num_Layers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), _ViewDir)));\n"
                "    float layerDepth = 1.0 / num_Layers;\n"// calculate the size of each layer 
                "    float currentLayerDepth = 0.0;\n"// depth of current layer  
                "    vec2 P = _ViewDir.xy * inLayer.data2.x;\n"// the amount to shift the texture coordinates per layer (from vector P)
                "    vec2 deltaUV = P / num_Layers;\n"
                "    vec2  currentUV = inUVs;\n"
                "    float currentDepth = texture2D(inLayer.texture, currentUV).r;\n"
                "    while(currentLayerDepth < currentDepth){\n"
                "    	currentUV -= deltaUV;\n"// shift texture coordinates along direction of P
                "    	currentDepth = texture2D(inLayer.texture, currentUV).r;\n"
                "    	currentLayerDepth += layerDepth;\n"
                "    }\n"
                "    vec2 prevUV = currentUV + deltaUV;\n" // get texture coordinates before collision (reverse operations)
                "    float afterDepth  = currentDepth - currentLayerDepth;\n" // get depth after and before collision for linear interpolation
                "    float beforeDepth = texture2D(inLayer.texture, prevUV).r - currentLayerDepth + layerDepth;\n"
                "    float weight = afterDepth / (afterDepth - beforeDepth);\n" // interpolation of texture coordinates
                "    return prevUV * weight + currentUV * (1.0 - weight);\n"
                "}\n"
                "vec2 CalculateParallaxMapLOD(Layer inLayer, vec3 _ViewDir, vec2 inUVs, float lod){\n"
                //"    inUVs *= inLayer.uvModifications.zw;\n"
                //"    inUVs += inLayer.uvModifications.xy;\n"
                "    float minLayers = min(5.0,(5.0 * inLayer.data2.x) + 1.0);\n"
                "    float maxLayers = min(30.0,(30.0 * inLayer.data2.x) + 1.0);\n"
                "    float num_Layers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), _ViewDir)));\n"
                "    float layerDepth = 1.0 / num_Layers;\n"// calculate the size of each layer 
                "    float currentLayerDepth = 0.0;\n"// depth of current layer  
                "    vec2 P = _ViewDir.xy * inLayer.data2.x;\n"// the amount to shift the texture coordinates per layer (from vector P)
                "    vec2 deltaUV = P / num_Layers;\n"
                "    vec2  currentUV = inUVs;\n"
                "    float currentDepth = texture2DLod(inLayer.texture, currentUV, lod).r;\n"
                "    while(currentLayerDepth < currentDepth){\n"
                "    	currentUV -= deltaUV;\n"// shift texture coordinates along direction of P
                "    	currentDepth = texture2DLod(inLayer.texture, currentUV, lod).r;\n"
                "    	currentLayerDepth += layerDepth;\n"
                "    }\n"
                "    vec2 prevUV = currentUV + deltaUV;\n" // get texture coordinates before collision (reverse operations)
                "    float afterDepth  = currentDepth - currentLayerDepth;\n" // get depth after and before collision for linear interpolation
                "    float beforeDepth = texture2DLod(inLayer.texture, prevUV, lod).r - currentLayerDepth + layerDepth;\n"
                "    float weight = afterDepth / (afterDepth - beforeDepth);\n" // interpolation of texture coordinates
                "    return prevUV * weight + currentUV * (1.0 - weight);\n"
                "}\n"
            , "void ProcessComponentParallax(");
        }
    }
#pragma endregion

#pragma region do blend
    //https://docs.gimp.org/2.4/en/gimp-concepts-layer-modes.html
    if (ShaderHelper::sfind(code, "DoBlend(")) {
        if (!ShaderHelper::sfind(code, "vec4 DoBlend(")) {
            ShaderHelper::insertStringRightBeforeLineContent(code, 
"vec4 DoBlend(vec4 paint, vec4 canvas, Layer inLayer) {//generated\n"//TODO: complete this
"    if (inLayer.data1.x       == 0.0) {\n"//default
"        return PaintersAlgorithm(paint, canvas);\n"
"    }else if (inLayer.data1.x == 1.0) {\n"//mix
"        return mix(paint, canvas, 0.5);\n"
"    }else if (inLayer.data1.x == 2.0) {\n"//add
"        return canvas + paint;\n"
"    }else if (inLayer.data1.x == 3.0) {\n"//subtract
"        return max(vec4(0.00001), canvas - paint);\n"
"    }else if (inLayer.data1.x == 4.0) {\n"//Multiply mode multiplies the pixel values of the upper layer with those of the layer below it and then divides the result by 255
"        return canvas * paint;\n"
"    }else if (inLayer.data1.x == 5.0) {\n"//divide
"        vec4 a = RangeTo255(canvas);\n"
"        vec4 b = RangeTo255(paint);\n"
"        a *= 256.0;\n"
"        a /= (paint + 0.0039215686);\n" //0.0039215686 is 1 / 255, to avoid divide by zero
"        return max(vec4(0.00001), RangeTo1(a));\n"
"    }else if (inLayer.data1.x == 6.0) {\n"//screen
"        vec4 a = RangeTo255(canvas);\n"
"        vec4 b = RangeTo255(paint);\n"
"        a = InvertColor255(a);\n"
"        b = InvertColor255(b);\n"
"        vec4 c = a * b;\n"
"        c /= 255.0;\n"
"        return RangeTo1(InvertColor255(c));\n"
"    }else if (inLayer.data1.x == 7.0) {\n"//Overlay - double check this
"        vec4 a = RangeTo255(canvas);\n"
"        vec4 aCopy = a;\n"
"        vec4 aCopy1 = a;\n"
"        vec4 b = RangeTo255(paint);\n"
"        a = InvertColor255(a);\n"
"        a *= (b * 2.0);\n"
"        aCopy += a;\n"
"        aCopy /= 255.0;\n"
"        aCopy *= aCopy1;\n"
"        aCopy /= 255.0;\n"
"        return RangeTo1(aCopy);\n"
"    }else if (inLayer.data1.x == 8.0) {\n"//TODO: dissolve (Dissolve mode dissolves the upper layer into the layer beneath it by drawing a random pattern of pixels in areas of partial transparency)
"        return canvas + paint;\n"
"    }else if (inLayer.data1.x == 9.0) {\n"//dodge
"        vec4 a = RangeTo255(canvas);\n"
"        vec4 b = RangeTo255(paint);\n"
"        a *= 256.0;\n"
"        a /= InvertColor255(b);\n"
"        a = RangeTo1(a);\n"
"        return a;\n"
"    }else if (inLayer.data1.x == 10.0) {\n"//burn
"        vec4 a = RangeTo255(canvas);\n"
"        vec4 b = RangeTo255(paint);\n"
"        a = InvertColor255(a);\n"
"        a *= 256.0;\n"
"        a /= (b + vec4(1.0));\n"
"        a = InvertColor255(a);\n"
"        a = RangeTo1(a);\n"
"        return a;\n"
"    }else if (inLayer.data1.x == 11.0) {\n" //TODO: Hard light mode is rather complicated because the equation consists of two parts, one for darker colors and one for brighter colors. If the pixel color of the upper layer is greater than 128, the layers are combined according to the first formula shown below. Otherwise, the pixel values of the upper and lower layers are multiplied together and multiplied by two, then divided by 256
"        return canvas + paint;\n"
"    }else if (inLayer.data1.x == 12.0) {\n" //TODO: soft light
"        return canvas + paint;\n"
"    }else if (inLayer.data1.x == 13.0) {\n" //GrainExtract
"        return (canvas - paint) + vec4(0.5);\n"
"    }else if (inLayer.data1.x == 14.0) {\n" //GrainMerge
"        return (canvas + paint) - vec4(0.5);\n"
"    }else if (inLayer.data1.x == 15.0) {\n" //TODO: Difference mode subtracts the pixel value of the upper layer from that of the lower layer and then takes the absolute value of the result.
"        return abs(canvas - paint);\n"
"    }else if (inLayer.data1.x == 16.0) {\n" //Darken
"        return min(canvas, paint);\n"
"    }else if (inLayer.data1.x == 17.0) {\n" //Lighten
"        return max(canvas, paint);\n"
"    }else if (inLayer.data1.x == 18.0) {\n" //TODO: Hue mode uses the hue of the upper layer and the saturation and value of the lower layer to form the resulting image. However, if the saturation of the upper layer is zero, the hue is taken from the lower layer, too.
"        return canvas + paint;\n"
"    }else if (inLayer.data1.x == 19.0) {\n" //TODO: Saturation mode uses the saturation of the upper layer and the hue and value of the lower layer to form the resulting image.
"        return canvas + paint;\n"
"    }else if (inLayer.data1.x == 20.0) {\n" //TODO: Color mode uses the hue and saturation of the upper layer and the value of the lower layer to form the resulting image.
"        return canvas + paint;\n"
"    }else if (inLayer.data1.x == 21.0) {\n" //TODO: Value mode uses the value of the upper layer and the saturation and hue of the lower layer to form the resulting image
"        return canvas + paint;\n"
"    }\n"
"}\n"
, "vec4 CalculateDiffuse(");
        }
    }
#pragma endregion

}
