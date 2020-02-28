#include <core/engine/renderer/opengl/glsl/Materials.h>

#include <core/engine/renderer/opengl/Extensions.h>
#include <core/engine/shaders/ShaderHelper.h>
#include <core/engine/materials/MaterialEnums.h>

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/lexical_cast.hpp>

using namespace Engine;
using namespace Engine::priv;
using namespace std;


void opengl::glsl::Materials::convert(string& code, const unsigned int versionNumber, const ShaderType::Type shaderType) {

#pragma region process component
    if (ShaderHelper::sfind(code, "ProcessComponent(") || ShaderHelper::sfind(code, "ProcessComponentLOD(")) {
        if (!ShaderHelper::sfind(code, "void ProcessComponent(")) {
            const string process_component =
                "void ProcessComponent(in Component inComponent, inout InData data) {//generated\n"
                "    if (inComponent.componentType == 0) {\n"
                "        for (int i = 0; i < inComponent.numLayers; ++i) {\n"
                "            data.diffuse = CalculateDiffuse(inComponent.layers[i], data.diffuse, data.objectColor, data.uv);\n"
                "       }\n"
                "    }else if (inComponent.componentType == 1) {\n"
                "        for (int i = 0; i < inComponent.numLayers; ++i) {\n"
                "            data.normals = CalculateNormals(inComponent.layers[i], data.normals, data.uv);\n"
                "        }\n"
                "    }else if (inComponent.componentType == 2) {\n"
                "        for (int i = 0; i < inComponent.numLayers; ++i) {\n"
                "            data.glow = CalculateGlow(inComponent.layers[i], data.glow, data.uv);\n"
                "        }\n"
                "    }else if (inComponent.componentType == 3) {\n"
                "        for (int i = 0; i < inComponent.numLayers; ++i) {\n"
                "            data.specular = CalculateSpecular(inComponent.layers[i], data.specular, data.uv);\n"
                "        }\n"
                "    }else if (inComponent.componentType == 4) {\n"
                "        for (int i = 0; i < inComponent.numLayers; ++i) {\n"
                "            data.ao = CalculateAO(inComponent.layers[i], data.ao, data.uv);\n"
                "        }\n"
                "    }else if (inComponent.componentType == 5) {\n"
                "        for (int i = 0; i < inComponent.numLayers; ++i) {\n"
                "            data.metalness = CalculateMetalness(inComponent.layers[i], data.metalness, data.uv);\n"
                "        }\n"
                "    }else if (inComponent.componentType == 6) {\n"
                "        for (int i = 0; i < inComponent.numLayers; ++i) {\n"
                "            data.smoothness = CalculateSmoothness(inComponent.layers[i], data.smoothness, data.uv);\n"
                "        }\n"
                "    }else if (inComponent.componentType == 7) {\n"
                "        for (int i = 0; i < inComponent.numLayers; ++i) {\n"
                "            data.diffuse = CalculateReflection(inComponent.layers[i], data.uv, data.diffuse, CamPosition, data.normals, data.worldPosition);\n"
                "        }\n"
                "    }else if (inComponent.componentType == 8) {\n"
                "        for (int i = 0; i < inComponent.numLayers; ++i) {\n"
                "            data.diffuse = CalculateRefraction(inComponent.layers[i], data.uv, data.diffuse, CamPosition, data.normals, data.worldPosition);\n"
                "        }\n"
                "    }else if (inComponent.componentType == 9) {\n"
                "        for (int i = 0; i < inComponent.numLayers; ++i) {\n"
                "            vec3 ViewDir = normalize(TangentCameraPos - TangentFragPos);\n"
                "            data.uv = CalculateParallaxMap(inComponent.layers[i], ViewDir, data.uv);\n"
                "        }\n"
                "    }\n"
                "}\n"
                "void ProcessComponentLOD(in Component inComponent, inout InData data, in float lod) {//generated\n"
                "    if (inComponent.componentType == 0) {\n"
                "        for (int i = 0; i < inComponent.numLayers; ++i) {\n"
                "            data.diffuse = CalculateDiffuseLOD(inComponent.layers[i], data.diffuse, data.objectColor, data.uv, lod);\n"
                "       }\n"
                "    }else if (inComponent.componentType == 1) {\n"
                "        for (int i = 0; i < inComponent.numLayers; ++i) {\n"
                "            data.normals = CalculateNormalsLOD(inComponent.layers[i], data.normals, data.uv, lod);\n"
                "        }\n"
                "    }else if (inComponent.componentType == 2) {\n"
                "        for (int i = 0; i < inComponent.numLayers; ++i) {\n"
                "            data.glow = CalculateGlowLOD(inComponent.layers[i], data.glow, data.uv, lod);\n"
                "        }\n"
                "    }else if (inComponent.componentType == 3) {\n"
                "        for (int i = 0; i < inComponent.numLayers; ++i) {\n"
                "            data.specular = CalculateSpecularLOD(inComponent.layers[i], data.specular, data.uv, lod);\n"
                "        }\n"
                "    }else if (inComponent.componentType == 4) {\n"
                "        for (int i = 0; i < inComponent.numLayers; ++i) {\n"
                "            data.ao = CalculateAOLOD(inComponent.layers[i], data.ao, data.uv, lod);\n"
                "        }\n"
                "    }else if (inComponent.componentType == 5) {\n"
                "        for (int i = 0; i < inComponent.numLayers; ++i) {\n"
                "            data.metalness = CalculateMetalnessLOD(inComponent.layers[i], data.metalness, data.uv, lod);\n"
                "        }\n"
                "    }else if (inComponent.componentType == 6) {\n"
                "        for (int i = 0; i < inComponent.numLayers; ++i) {\n"
                "            data.smoothness = CalculateSmoothnessLOD(inComponent.layers[i], data.smoothness, data.uv, lod);\n"
                "        }\n"
                "    }else if (inComponent.componentType == 7) {\n"
                "        for (int i = 0; i < inComponent.numLayers; ++i) {\n"
                "            data.diffuse = CalculateReflectionLOD(inComponent.layers[i], data.uv, data.diffuse, CamPosition, data.normals, data.worldPosition, lod);\n"
                "        }\n"
                "    }else if (inComponent.componentType == 8) {\n"
                "        for (int i = 0; i < inComponent.numLayers; ++i) {\n"
                "            data.diffuse = CalculateRefractionLOD(inComponent.layers[i], data.uv, data.diffuse, CamPosition, data.normals, data.worldPosition, lod);\n"
                "        }\n"
                "    }else if (inComponent.componentType == 9) {\n"
                "        for (int i = 0; i < inComponent.numLayers; ++i) {\n"
                "            vec3 ViewDir = normalize(TangentCameraPos - TangentFragPos);\n"
                "            data.uv = CalculateParallaxMapLOD(inComponent.layers[i], ViewDir, data.uv, lod);\n"
                "        }\n"
                "    }\n"
                "}\n";
            ShaderHelper::insertStringRightBeforeLineContent(code, process_component, " main(");
        }
    }
#pragma endregion

#pragma region calculate diffuse

    if (ShaderHelper::sfind(code, "CalculateDiffuse(") || ShaderHelper::sfind(code, "CalculateDiffuseLOD(")) {
        if (!ShaderHelper::sfind(code, "vec4 CalculateDiffuse(")) {
            const string calculate_diffuse =
                "vec4 CalculateDiffuse(in Layer inLayer, in vec4 canvas, in vec4 objectColor, in vec2 inUVs) {//generated\n"
                "    vec4 paint = objectColor;\n"
                "    if (inLayer.data1.y >= 0.5) {\n"
                "        paint *= texture2D(inLayer.texture, inUVs + inLayer.uvModifications);\n"
                "    }else{\n"
                "        paint *= vec4(Material_F0AndID.rgb, 1.0);\n"
                "    }\n"
                "    if (inLayer.data1.z >= 0.5) {\n"
                "        paint *= texture2D(inLayer.mask, inUVs).r;\n"
                "    }\n"
                "    paint *= inLayer.data2;\n"
                "    return DoBlend(paint, canvas, inLayer);\n"
                "}\n"
                "vec4 CalculateDiffuseLOD(in Layer inLayer, in vec4 canvas, in vec4 objectColor, in vec2 inUVs, in float lod) {//generated\n"
                "    vec4 paint = objectColor;\n"
                "    if (inLayer.data1.y >= 0.5) {\n"
                "        paint *= texture2DLod(inLayer.texture, inUVs + inLayer.uvModifications, lod);\n"
                "    }else{\n"
                "        paint *= vec4(Material_F0AndID.rgb, 1.0);\n"
                "    }\n"
                "    if (inLayer.data1.z >= 0.5) {\n"
                "        paint *= texture2DLod(inLayer.mask, inUVs, lod).r;\n"
                "    }\n"
                "    paint *= inLayer.data2;\n"
                "    return DoBlend(paint, canvas, inLayer);\n"
                "}\n";
            ShaderHelper::insertStringRightBeforeLineContent(code, calculate_diffuse, "void ProcessComponent(");
        }
    }
#pragma endregion

#pragma region calculate normals
    if (ShaderHelper::sfind(code, "CalculateNormals(") || ShaderHelper::sfind(code, "CalculateNormalsLOD(")) {
        if (!ShaderHelper::sfind(code, "vec3 CalculateNormals(")) {
            const string calculate_normals =
                "vec3 CalculateNormals(in Layer inLayer, in vec3 objectNormals, in vec2 inUVs) {//generated\n"
                "    vec3 outNormals = objectNormals;\n"
                "    if (inLayer.data1.y > 0.9) {\n"
                "        outNormals = CalcBumpedNormal(inUVs + inLayer.uvModifications, inLayer.texture);\n"
                "    }else if (inLayer.data1.y > 0.4) {\n"
                "        outNormals = CalcBumpedNormalCompressed(inUVs + inLayer.uvModifications, inLayer.texture);\n"
                "    }\n"
                "    if (inLayer.data1.z >= 0.5) {\n"
                "        outNormals *= texture2D(inLayer.mask, inUVs).r;\n"
                "    }\n"
                "    outNormals *= inLayer.data2.xyz;\n"
                "    outNormals *= inLayer.data2.w;\n"
                "    return outNormals;\n"
                "}\n"
                "vec3 CalculateNormalsLOD(in Layer inLayer, in vec3 objectNormals, in vec2 inUVs, in float lod) {//generated\n"
                "    vec3 outNormals = objectNormals;\n"
                "    if (inLayer.data1.y > 0.9) {\n"
                "        outNormals = CalcBumpedNormalLOD(inUVs + inLayer.uvModifications, inLayer.texture, lod);\n"
                "    }else if (inLayer.data1.y > 0.4) {\n"
                "        outNormals = CalcBumpedNormalCompressedLOD(inUVs + inLayer.uvModifications, inLayer.texture, lod);\n"
                "    }\n"
                "    if (inLayer.data1.z >= 0.5) {\n"
                "        outNormals *= texture2D(inLayer.mask, inUVs).r;\n"
                "    }\n"
                "    outNormals *= inLayer.data2.xyz;\n"
                "    outNormals *= inLayer.data2.w;\n"
                "    return outNormals;\n"
                "}\n";
            ShaderHelper::insertStringRightBeforeLineContent(code, calculate_normals, "vec4 CalculateDiffuse(");
        }
    }
#pragma endregion


#pragma region calculate glow
    if (ShaderHelper::sfind(code, "CalculateGlow(") || ShaderHelper::sfind(code, "CalculateGlowLOD(")) {
        if (!ShaderHelper::sfind(code, "float CalculateGlow(")) {
            const string calculate_glow =
                "float CalculateGlow(in Layer inLayer, in float objectGlow, in vec2 inUVs) {//generated\n"
                "    float outGlow = objectGlow;\n"
                "    if (inLayer.data1.y >= 0.5) {\n"
                "        outGlow += texture2D(inLayer.texture, inUVs + inLayer.uvModifications).r;\n"
                "    }\n"
                "    if (inLayer.data1.z >= 0.5) {\n"
                "        outGlow *= texture2D(inLayer.mask, inUVs).r;\n"
                "    }\n"
                "    outGlow = clamp(outGlow, inLayer.data2.x, inLayer.data2.y);\n"
                "    outGlow *= inLayer.data2.z;\n"
                "    return outGlow;\n"
                "}\n"
                "float CalculateGlowLOD(in Layer inLayer, in float objectGlow, in vec2 inUVs, in float lod) {//generated\n"
                "    float outGlow = objectGlow;\n"
                "    if (inLayer.data1.y >= 0.5) {\n"
                "        outGlow += texture2DLod(inLayer.texture, inUVs + inLayer.uvModifications, lod).r;\n"
                "    }\n"
                "    if (inLayer.data1.z >= 0.5) {\n"
                "        outGlow *= texture2DLod(inLayer.mask, inUVs, lod).r;\n"
                "    }\n"
                "    outGlow = clamp(outGlow, inLayer.data2.x, inLayer.data2.y);\n"
                "    outGlow *= inLayer.data2.z;\n"
                "    return outGlow;\n"
                "}\n";
            ShaderHelper::insertStringRightBeforeLineContent(code, calculate_glow, "vec3 CalculateNormals(");
        }
    }
#pragma endregion

#pragma region calculate specular
    if (ShaderHelper::sfind(code, "CalculateSpecular(") || ShaderHelper::sfind(code, "CalculateSpecularLOD(")) {
        if (!ShaderHelper::sfind(code, "float CalculateSpecular(")) {
            const string calculate_spec =
                "float CalculateSpecular(in Layer inLayer, in float objectSpecular, in vec2 inUVs) {//generated\n"
                "    float outSpecular = objectSpecular;\n"
                "    if (inLayer.data1.y >= 0.5) {\n"
                "        outSpecular *= texture2D(inLayer.texture, inUVs + inLayer.uvModifications).r;\n"
                "    }\n"
                "    if (inLayer.data1.z >= 0.5) {\n"
                "        outSpecular *= texture2D(inLayer.mask, inUVs).r;\n"
                "    }\n"
                "    outSpecular = clamp(outSpecular, inLayer.data2.x, inLayer.data2.y);\n"
                "    outSpecular *= inLayer.data2.z;\n"
                "    return outSpecular;\n"
                "}\n"
                "float CalculateSpecularLOD(in Layer inLayer, in float objectSpecular, in vec2 inUVs, in float lod) {//generated\n"
                "    float outSpecular = objectSpecular;\n"
                "    if (inLayer.data1.y >= 0.5) {\n"
                "        outSpecular *= texture2DLod(inLayer.texture, inUVs + inLayer.uvModifications, lod).r;\n"
                "    }\n"
                "    if (inLayer.data1.z >= 0.5) {\n"
                "        outSpecular *= texture2DLod(inLayer.mask, inUVs, lod).r;\n"
                "    }\n"
                "    outSpecular = clamp(outSpecular, inLayer.data2.x, inLayer.data2.y);\n"
                "    outSpecular *= inLayer.data2.z;\n"
                "    return outSpecular;\n"
                "}\n";
            ShaderHelper::insertStringRightBeforeLineContent(code, calculate_spec, "float CalculateGlow(");
        }
    }
#pragma endregion


#pragma region calculate ao
    if (ShaderHelper::sfind(code, "CalculateAO(") || ShaderHelper::sfind(code, "CalculateAOLOD(")) {
        if (!ShaderHelper::sfind(code, "float CalculateAO(")) {
            const string calculate_ao =
                "float CalculateAO(in Layer inLayer, in float objectAO, in vec2 inUVs) {//generated\n"
                "    float outAO = objectAO;\n"
                "    if (inLayer.data1.y >= 0.5) {\n"
                "        outAO *= texture2D(inLayer.texture, inUVs + inLayer.uvModifications).r;\n"
                "    }\n"
                "    if (inLayer.data1.z >= 0.5) {\n"
                "        outAO *= texture2D(inLayer.mask, inUVs).r;\n"
                "    }\n"
                "    outAO = clamp(outAO, inLayer.data2.x, inLayer.data2.y);\n"
                "    outAO *= inLayer.data2.z;\n"
                "    return outAO;\n"
                "}\n"
                "float CalculateAOLOD(in Layer inLayer, in float objectAO, in vec2 inUVs, in float lod) {//generated\n"
                "    float outAO = objectAO;\n"
                "    if (inLayer.data1.y >= 0.5) {\n"
                "        outAO *= texture2DLod(inLayer.texture, inUVs + inLayer.uvModifications, lod).r;\n"
                "    }\n"
                "    if (inLayer.data1.z >= 0.5) {\n"
                "        outAO *= texture2DLod(inLayer.mask, inUVs, lod).r;\n"
                "    }\n"
                "    outAO = clamp(outAO, inLayer.data2.x, inLayer.data2.y);\n"
                "    outAO *= inLayer.data2.z;\n"
                "    return outAO;\n"
                "}\n";
            ShaderHelper::insertStringRightBeforeLineContent(code, calculate_ao, "float CalculateSpecular(");
        }
    }
#pragma endregion


#pragma region calculate metalness
    if (ShaderHelper::sfind(code, "CalculateMetalness(") || ShaderHelper::sfind(code, "CalculateMetalnessLOD(")) {
        if (!ShaderHelper::sfind(code, "float CalculateMetalness(")) {
            const string calculate_metalness =
                "float CalculateMetalness(in Layer inLayer, in float objectMetalness, in vec2 inUVs) {//generated\n"
                "    float outMetalness = objectMetalness;\n"
                "    if (inLayer.data1.y >= 0.5) {\n"
                "        outMetalness = texture2D(inLayer.texture, inUVs + inLayer.uvModifications).r;\n"
                "    }\n"
                "    if (inLayer.data1.z >= 0.5) {\n"
                "        outMetalness *= texture2D(inLayer.mask, inUVs).r;\n"
                "    }\n"
                "    outMetalness = clamp(outMetalness, inLayer.data2.x, inLayer.data2.y);\n"
                "    outMetalness *= inLayer.data2.z;\n"
                "    return clamp(outMetalness,0.01,0.99);\n"
                "}\n"
                "float CalculateMetalnessLOD(in Layer inLayer, in float objectMetalness, in vec2 inUVs, in float lod) {//generated\n"
                "    float outMetalness = objectMetalness;\n"
                "    if (inLayer.data1.y >= 0.5) {\n"
                "        outMetalness = texture2DLod(inLayer.texture, inUVs + inLayer.uvModifications, lod).r;\n"
                "    }\n"
                "    if (inLayer.data1.z >= 0.5) {\n"
                "        outMetalness *= texture2DLod(inLayer.mask, inUVs, lod).r;\n"
                "    }\n"
                "    outMetalness = clamp(outMetalness, inLayer.data2.x, inLayer.data2.y);\n"
                "    outMetalness *= inLayer.data2.z;\n"
                "    return clamp(outMetalness,0.01,0.99);\n"
                "}\n";
            ShaderHelper::insertStringRightBeforeLineContent(code, calculate_metalness, "float CalculateAO(");
        }
    }
#pragma endregion


#pragma region calculate smoothness
    if (ShaderHelper::sfind(code, "CalculateSmoothness(") || ShaderHelper::sfind(code, "CalculateSmoothnessLOD(")) {
        if (!ShaderHelper::sfind(code, "float CalculateSmoothness(")) {
            const string calculate_smoothness =
                "float CalculateSmoothness(in Layer inLayer, in float objectSmoothness, in vec2 inUVs) {//generated\n"
                "    float outSmoothness = objectSmoothness;\n"
                "    if (inLayer.data1.y >= 0.5) {\n"
                "        outSmoothness = texture2D(inLayer.texture, inUVs + inLayer.uvModifications).r;\n"
                "    }\n"
                "    if (inLayer.data1.z >= 0.5) {\n"
                "        outSmoothness *= texture2D(inLayer.mask, inUVs).r;\n"
                "    }\n"
                "    outSmoothness = clamp(outSmoothness, inLayer.data2.x, inLayer.data2.y);\n"
                "    outSmoothness *= inLayer.data2.z;\n"
                "    return clamp(outSmoothness,0.01,0.99);\n"
                "}\n"
                "float CalculateSmoothnessLOD(in Layer inLayer, in float objectSmoothness, in vec2 inUVs, in float lod) {//generated\n"
                "    float outSmoothness = objectSmoothness;\n"
                "    if (inLayer.data1.y >= 0.5) {\n"
                "        outSmoothness = texture2DLod(inLayer.texture, inUVs + inLayer.uvModifications, lod).r;\n"
                "    }\n"
                "    if (inLayer.data1.z >= 0.5) {\n"
                "        outSmoothness *= texture2DLod(inLayer.mask, inUVs, lod).r;\n"
                "    }\n"
                "    outSmoothness = clamp(outSmoothness, inLayer.data2.x, inLayer.data2.y);\n"
                "    outSmoothness *= inLayer.data2.z;\n"
                "    return clamp(outSmoothness,0.01,0.99);\n"
                "}\n";
            ShaderHelper::insertStringRightBeforeLineContent(code, calculate_smoothness, "float CalculateMetalness(");
        }
    }
#pragma endregion

#pragma region calculate reflection
    if (ShaderHelper::sfind(code, "CalculateReflection(") || ShaderHelper::sfind(code, "CalculateReflectionLOD(")) {
        if (!ShaderHelper::sfind(code, "vec4 CalculateReflection(")) {
            const string calculate_reflection =
                "vec4 CalculateReflection(in Layer inLayer, in vec2 inUVs,in vec4 inDiffuse, in vec3 inCameraPosition, in vec3 inNormals, in vec3 inWorldPosition){//generated\n"
                //"    inUVs += inLayer.uvModifications;\n"
                "    vec4 r = vec4(0.0);\n"
                "    r = textureCube(inLayer.cubemap,reflect(inNormals,normalize(inCameraPosition - inWorldPosition))) * texture2D(inLayer.mask,inUVs).r;\n"
                "    r.a *= inLayer.data2.x;\n"
                "    r = PaintersAlgorithm(r,inDiffuse);\n"
                "    return r;\n"
                "}\n"
                "vec4 CalculateReflectionLOD(in Layer inLayer, in vec2 inUVs,in vec4 inDiffuse, in vec3 inCameraPosition, in vec3 inNormals, in vec3 inWorldPosition, in float lod){//generated\n"
                //"    inUVs += inLayer.uvModifications;\n"
                "    vec4 r = vec4(0.0);\n"
                "    r = textureCubeLod(inLayer.cubemap,reflect(inNormals,normalize(inCameraPosition - inWorldPosition)), lod) * texture2D(inLayer.mask,inUVs).r;\n"
                "    r.a *= inLayer.data2.x;\n"
                "    r = PaintersAlgorithm(r,inDiffuse);\n"
                "    return r;\n"
                "}\n";
            ShaderHelper::insertStringRightBeforeLineContent(code, calculate_reflection, "float CalculateSmoothness(");
        }
    }
#pragma endregion

#pragma region calculate refraction
    if (ShaderHelper::sfind(code, "CalculateRefraction(") || ShaderHelper::sfind(code, "CalculateRefractionLOD(")) {
        if (!ShaderHelper::sfind(code, "vec4 CalculateRefraction(")) {
            const string calculate_refraction =
                "vec4 CalculateRefraction(in Layer inLayer, in vec2 inUVs,in vec4 inDiffuse, in vec3 inCameraPosition, in vec3 inNormals, in vec3 inWorldPosition){//generated\n"
                //"    inUVs += inLayer.uvModifications;\n"
                "    vec4 r = vec4(0.0);\n"
                "    r = textureCube(inLayer.cubemap,refract(inNormals,normalize(inCameraPosition - inWorldPosition),1.0 / inLayer.data2.y)) * texture2D(inLayer.mask,inUVs).r;\n"
                "    r.a *= inLayer.data2.x;\n"
                "    r = PaintersAlgorithm(r,inDiffuse);\n"
                "    return r;\n"
                "}\n"
                "vec4 CalculateRefractionLOD(in Layer inLayer, in vec2 inUVs,in vec4 inDiffuse, in vec3 inCameraPosition, in vec3 inNormals, in vec3 inWorldPosition, in float lod){//generated\n"
                //"    inUVs += inLayer.uvModifications;\n"
                "    vec4 r = vec4(0.0);\n"
                "    r = textureCubeLod(inLayer.cubemap,refract(inNormals,normalize(inCameraPosition - inWorldPosition),1.0 / inLayer.data2.y), lod) * texture2D(inLayer.mask,inUVs).r;\n"
                "    r.a *= inLayer.data2.x;\n"
                "    r = PaintersAlgorithm(r,inDiffuse);\n"
                "    return r;\n"
                "}\n";
            ShaderHelper::insertStringRightBeforeLineContent(code, calculate_refraction, "vec4 CalculateReflection(");
        }
    }
#pragma endregion

#pragma region calculate parallax 
    if (ShaderHelper::sfind(code, "CalculateParallaxMap(") || ShaderHelper::sfind(code, "CalculateParallaxMapLOD(")) {
        if (!ShaderHelper::sfind(code, "vec2 CalculateParallaxMap(")) {
            const string calculate_parallax =
                "vec2 CalculateParallaxMap(in Layer inLayer, vec3 _ViewDir, in vec2 inUVs){//generated\n"
                //"    inUVs += inLayer.uvModifications;\n"
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
                "vec2 CalculateParallaxMapLOD(in Layer inLayer, vec3 _ViewDir, in vec2 inUVs, in float lod){//generated\n"
                //"    inUVs += inLayer.uvModifications;\n"
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
                "}\n";
            ShaderHelper::insertStringRightBeforeLineContent(code, calculate_parallax, "vec4 CalculateRefraction(");
        }
    }
#pragma endregion


#pragma region do blend
    //https://docs.gimp.org/2.4/en/gimp-concepts-layer-modes.html
    if (ShaderHelper::sfind(code, "DoBlend(")) {
        if (!ShaderHelper::sfind(code, "vec4 DoBlend(")) {
            const string do_blend =
                "vec4 DoBlend(in vec4 paint, in vec4 canvas, in Layer inLayer) {//generated\n"//TODO: complete this
                "    if (inLayer.data1.x       == 0.0) {\n"//default
                "        return PaintersAlgorithm(paint, canvas);\n"
                "    }else if (inLayer.data1.x == 1.0) {\n"//mix
                "        vec4 ret = mix(paint, canvas, 0.5);\n"
                "        return ret;\n"
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
                "    }else if (inLayer.data1.x == 16.0) {\n" //Darkene
                "        float r = min(canvas.r, paint.r);\n"
                "        float g = min(canvas.g, paint.g);\n"
                "        float b = min(canvas.b, paint.b);\n"
                "        float a = min(canvas.a, paint.a);\n"
                "        return vec4(r, g, b, a);\n"
                "    }else if (inLayer.data1.x == 17.0) {\n" //Lighten
                "        float r = max(canvas.r, paint.r);\n"
                "        float g = max(canvas.g, paint.g);\n"
                "        float b = max(canvas.b, paint.b);\n"
                "        float a = max(canvas.a, paint.a);\n"
                "        return vec4(r, g, b, a);\n"
                "    }else if (inLayer.data1.x == 18.0) {\n" //TODO: Hue mode uses the hue of the upper layer and the saturation and value of the lower layer to form the resulting image. However, if the saturation of the upper layer is zero, the hue is taken from the lower layer, too.
                "        return canvas + paint;\n"
                "    }else if (inLayer.data1.x == 18.0) {\n" //TODO: Saturation mode uses the saturation of the upper layer and the hue and value of the lower layer to form the resulting image.
                "        return canvas + paint;\n"
                "    }else if (inLayer.data1.x == 18.0) {\n" //TODO: Color mode uses the hue and saturation of the upper layer and the value of the lower layer to form the resulting image.
                "        return canvas + paint;\n"
                "    }else if (inLayer.data1.x == 18.0) {\n" //TODO: Value mode uses the value of the upper layer and the saturation and hue of the lower layer to form the resulting image
                "        return canvas + paint;\n"
                "    }\n"
                "}\n";
            ShaderHelper::insertStringRightBeforeLineContent(code, do_blend, "vec2 CalculateParallaxMap(");
        }
    }
#pragma endregion

}
