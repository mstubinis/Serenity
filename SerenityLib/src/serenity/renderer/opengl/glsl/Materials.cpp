
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
            void SUBMIT_DIFFUSE(float r, float g, float b, float a){
                gl_FragData[0] = vec4(r, g, b, a);
            }
            void SUBMIT_DIFFUSE(vec4 color){
                gl_FragData[0] = color;
            }
        )");
    }
    if (ShaderHelper::lacksDefinition(code, "SUBMIT_NORMALS(", "void SUBMIT_NORMALS(")) {
        ShaderHelper::insertStringRightBeforeMainFunc(code, R"(
            void SUBMIT_NORMALS(float x, float y, float z){
                gl_FragData[1].xy = EncodeOctahedron(vec3(x, y, z));
            }
            void SUBMIT_NORMALS(vec3 normals){
                gl_FragData[1].xy = EncodeOctahedron(normals);
            }
            void SUBMIT_NORMALS(float xCompressed, float yCompressed){
                gl_FragData[1].xy = vec2(xCompressed, yCompressed);
            }
            void SUBMIT_NORMALS(vec2 compressedNormals){
                gl_FragData[1].xy = compressedNormals;
            }
        )");
    }
    if (ShaderHelper::lacksDefinition(code, "SUBMIT_GLOW(", "void SUBMIT_GLOW(")) {
        ShaderHelper::insertStringRightBeforeMainFunc(code, R"(
            void SUBMIT_GLOW(float glow){
                gl_FragData[1].a = glow;
            }
        )");
    }
    if (ShaderHelper::lacksDefinition(code, "SUBMIT_SPECULAR(", "void SUBMIT_SPECULAR(")) {
        ShaderHelper::insertStringRightBeforeMainFunc(code, R"(
            void SUBMIT_SPECULAR(float specular){
                gl_FragData[2].g = specular;
            }
        )");
    }
    if (ShaderHelper::lacksDefinition(code, "SUBMIT_MATERIAL_ID_AND_AO(", "void SUBMIT_MATERIAL_ID_AND_AO(")) {
        ShaderHelper::insertStringRightBeforeMainFunc(code, R"(
            void SUBMIT_MATERIAL_ID_AND_AO(float matID, float ao){
                gl_FragData[1].b = matID + ao;
            }
            void SUBMIT_MATERIAL_ID_AND_AO(float matIDAndAO){
                gl_FragData[1].b = matIDAndAO;
            }
        )");
    }
    if (ShaderHelper::lacksDefinition(code, "SUBMIT_GOD_RAYS_COLOR(", "void SUBMIT_GOD_RAYS_COLOR(")) {
        ShaderHelper::insertStringRightBeforeMainFunc(code, R"(
            void SUBMIT_GOD_RAYS_COLOR(float godRaysR, float godRaysG, float godRaysB){
                float GodRaysRG = Pack2NibblesInto8BitChannel(godRaysR, godRaysG);
                gl_FragData[2].ba = vec2(GodRaysRG, godRaysB);
            }
            void SUBMIT_GOD_RAYS_COLOR(float godRaysPackedRG, float godRaysB){
                gl_FragData[2].ba = vec2(godRaysPackedRG, godRaysB);
            }
        )");
    }
    if (ShaderHelper::lacksDefinition(code, "SUBMIT_METALNESS_AND_SMOOTHNESS(", "void SUBMIT_METALNESS_AND_SMOOTHNESS(")) {
        ShaderHelper::insertStringRightBeforeMainFunc(code, R"(
            void SUBMIT_METALNESS_AND_SMOOTHNESS(float inMetal, float inSmooth){
                float PackedMetalSmooth = Pack2NibblesInto8BitChannel(inMetal, inSmooth);
                gl_FragData[2].r = PackedMetalSmooth;
            }
            void SUBMIT_METALNESS_AND_SMOOTHNESS(float inPackedMetalSmooth){
                gl_FragData[2].r = inPackedMetalSmooth;
            }
        )");
    }
#pragma endregion

#pragma region process component
    if (ShaderHelper::sfind(code, "ProcessComponent(") || ShaderHelper::sfind(code, "ProcessComponentLOD(")) {
        if (!ShaderHelper::sfind(code, "void ProcessComponent(")) {
            ShaderHelper::insertStringRightBeforeLineContent(code, R"(
                void ProcessComponent(in Component inComponent, inout InData data) {
                    if (inComponent.componentData.y == 0) {
                        for (int i = 0; i < inComponent.componentData.x; ++i) {
                            data.diffuse = CalculateDiffuse(inComponent.layers[i], data.diffuse, data.objectColor, data.uv);
                       }
                    }else if (inComponent.componentData.y == 1) {
                        for (int i = 0; i < inComponent.componentData.x; ++i) {
                            data.normals = CalculateNormals(inComponent.layers[i], data.normals, data.uv);
                        }
                    }else if (inComponent.componentData.y == 2) {
                        for (int i = 0; i < inComponent.componentData.x; ++i) {
                            data.glow = CalculateGlow(inComponent.layers[i], data.glow, data.uv);
                        }
                    }else if (inComponent.componentData.y == 3) {
                        for (int i = 0; i < inComponent.componentData.x; ++i) {
                            data.specular = CalculateSpecular(inComponent.layers[i], data.specular, data.uv);
                        }
                    }else if (inComponent.componentData.y == 4) {
                        for (int i = 0; i < inComponent.componentData.x; ++i) {
                            data.ao = CalculateAO(inComponent.layers[i], data.ao, data.uv);
                        }
                    }else if (inComponent.componentData.y == 5) {
                        for (int i = 0; i < inComponent.componentData.x; ++i) {
                            data.metalness = CalculateMetalness(inComponent.layers[i], data.metalness, data.uv);
                        }
                    }else if (inComponent.componentData.y == 6) {
                        for (int i = 0; i < inComponent.componentData.x; ++i) {
                            data.smoothness = CalculateSmoothness(inComponent.layers[i], data.smoothness, data.uv);
                        }
                    }else if (inComponent.componentData.y == 7) {
                        for (int i = 0; i < inComponent.componentData.x; ++i) {
                            data.diffuse = CalculateReflection(inComponent.layers[i], data.uv, data.diffuse, CamPosition, data.normals, data.worldPosition);
                        }
                    }else if (inComponent.componentData.y == 8) {
                        for (int i = 0; i < inComponent.componentData.x; ++i) {
                            data.diffuse = CalculateRefraction(inComponent.layers[i], data.uv, data.diffuse, CamPosition, data.normals, data.worldPosition);
                        }
                    }else if (inComponent.componentData.y == 9) {
                        for (int i = 0; i < inComponent.componentData.x; ++i) {
                            vec3 ViewDir = normalize(TangentCameraPos - TangentFragPos);
                            data.uv = CalculateParallaxMap(inComponent.layers[i], ViewDir, data.uv);
                        }
                    }
                }
                void ProcessComponentLOD(in Component inComponent, inout InData data, in float lod) {
                    if (inComponent.componentData.y == 0) {
                        for (int i = 0; i < inComponent.componentData.x; ++i) {
                            data.diffuse = CalculateDiffuseLOD(inComponent.layers[i], data.diffuse, data.objectColor, data.uv, lod);
                       }
                    }else if (inComponent.componentData.y == 1) {
                        for (int i = 0; i < inComponent.componentData.x; ++i) {
                            data.normals = CalculateNormalsLOD(inComponent.layers[i], data.normals, data.uv, lod);
                        }
                    }else if (inComponent.componentData.y == 2) {
                        for (int i = 0; i < inComponent.componentData.x; ++i) {
                            data.glow = CalculateGlowLOD(inComponent.layers[i], data.glow, data.uv, lod);
                        }
                    }else if (inComponent.componentData.y == 3) {
                        for (int i = 0; i < inComponent.componentData.x; ++i) {
                            data.specular = CalculateSpecularLOD(inComponent.layers[i], data.specular, data.uv, lod);
                        }
                    }else if (inComponent.componentData.y == 4) {
                        for (int i = 0; i < inComponent.componentData.x; ++i) {
                            data.ao = CalculateAOLOD(inComponent.layers[i], data.ao, data.uv, lod);
                        }
                    }else if (inComponent.componentData.y == 5) {
                        for (int i = 0; i < inComponent.componentData.x; ++i) {
                            data.metalness = CalculateMetalnessLOD(inComponent.layers[i], data.metalness, data.uv, lod);
                        }
                    }else if (inComponent.componentData.y == 6) {
                        for (int i = 0; i < inComponent.componentData.x; ++i) {
                            data.smoothness = CalculateSmoothnessLOD(inComponent.layers[i], data.smoothness, data.uv, lod);
                        }
                    }else if (inComponent.componentData.y == 7) {
                        for (int i = 0; i < inComponent.componentData.x; ++i) {
                            data.diffuse = CalculateReflectionLOD(inComponent.layers[i], data.uv, data.diffuse, CamPosition, data.normals, data.worldPosition, lod);
                        }
                    }else if (inComponent.componentData.y == 8) {
                        for (int i = 0; i < inComponent.componentData.x; ++i) {
                            data.diffuse = CalculateRefractionLOD(inComponent.layers[i], data.uv, data.diffuse, CamPosition, data.normals, data.worldPosition, lod);
                        }
                    }else if (inComponent.componentData.y == 9) {
                        for (int i = 0; i < inComponent.componentData.x; ++i) {
                            vec3 ViewDir = normalize(TangentCameraPos - TangentFragPos);
                            data.uv = CalculateParallaxMapLOD(inComponent.layers[i], ViewDir, data.uv, lod);
                        }
                    }
                }
            )", " main(");
        }
    }
#pragma endregion

#pragma region calculate diffuse

    if (ShaderHelper::sfind(code, "CalculateDiffuse(") || ShaderHelper::sfind(code, "CalculateDiffuseLOD(")) {
        if (!ShaderHelper::sfind(code, "vec4 CalculateDiffuse(")) {
            ShaderHelper::insertStringRightBeforeLineContent(code, R"(
                vec4 CalculateDiffuse(in Layer inLayer, in vec4 canvas, in vec4 objectColor, in vec2 inUVs) {
                    vec4 paint = objectColor;
                    if (inLayer.data1.y >= 0.5) {
                        paint *= texture2D(inLayer.texture, inUVs * inLayer.uvModifications.zw + inLayer.uvModifications.xy);
                    }else{
                        paint *= vec4(Material_F0AndID.rgb, 1.0);
                    }
                    if (inLayer.data1.z >= 0.5) {
                        paint *= texture2D(inLayer.mask, inUVs).r;
                    }
                    paint *= inLayer.data2;
                    return DoBlend(paint, canvas, inLayer);
                }
                vec4 CalculateDiffuseLOD(in Layer inLayer, in vec4 canvas, in vec4 objectColor, in vec2 inUVs, in float lod) {
                    vec4 paint = objectColor;
                    if (inLayer.data1.y >= 0.5) {
                        paint *= texture2DLod(inLayer.texture, inUVs * inLayer.uvModifications.zw + inLayer.uvModifications.xy, lod);
                    }else{
                        paint *= vec4(Material_F0AndID.rgb, 1.0);
                    }
                    if (inLayer.data1.z >= 0.5) {
                        paint *= texture2DLod(inLayer.mask, inUVs, lod).r;
                    }
                    paint *= inLayer.data2;
                    return DoBlend(paint, canvas, inLayer);
                }
            )", "void ProcessComponent(");
        }
    }
#pragma endregion

#pragma region calculate normals
    if (ShaderHelper::sfind(code, "CalculateNormals(") || ShaderHelper::sfind(code, "CalculateNormalsLOD(")) {
        if (!ShaderHelper::sfind(code, "vec3 CalculateNormals(")) {
            ShaderHelper::insertStringRightBeforeLineContent(code, R"(
                vec3 CalculateNormals(in Layer inLayer, in vec3 objectNormals, in vec2 inUVs) {
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
                vec3 CalculateNormalsLOD(in Layer inLayer, in vec3 objectNormals, in vec2 inUVs, in float lod) {
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
            )", "vec4 CalculateDiffuse(");
        }
    }
#pragma endregion

#pragma region calculate glow
    if (ShaderHelper::sfind(code, "CalculateGlow(") || ShaderHelper::sfind(code, "CalculateGlowLOD(")) {
        if (!ShaderHelper::sfind(code, "float CalculateGlow(")) {
            ShaderHelper::insertStringRightBeforeLineContent(code, R"(
                float CalculateGlow(in Layer inLayer, in float objectGlow, in vec2 inUVs) {
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
                float CalculateGlowLOD(in Layer inLayer, in float objectGlow, in vec2 inUVs, in float lod) {
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
            )", "vec3 CalculateNormals(");
        }
    }
#pragma endregion

#pragma region calculate specular
    if (ShaderHelper::sfind(code, "CalculateSpecular(") || ShaderHelper::sfind(code, "CalculateSpecularLOD(")) {
        if (!ShaderHelper::sfind(code, "float CalculateSpecular(")) {
            ShaderHelper::insertStringRightBeforeLineContent(code, R"(
                float CalculateSpecular(in Layer inLayer, in float objectSpecular, in vec2 inUVs) {
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
                float CalculateSpecularLOD(in Layer inLayer, in float objectSpecular, in vec2 inUVs, in float lod) {
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
            )", "float CalculateGlow(");
        }
    }
#pragma endregion

#pragma region calculate ao
    if (ShaderHelper::sfind(code, "CalculateAO(") || ShaderHelper::sfind(code, "CalculateAOLOD(")) {
        if (!ShaderHelper::sfind(code, "float CalculateAO(")) {
            ShaderHelper::insertStringRightBeforeLineContent(code, R"(
                float CalculateAO(in Layer inLayer, in float objectAO, in vec2 inUVs) {
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
                float CalculateAOLOD(in Layer inLayer, in float objectAO, in vec2 inUVs, in float lod) {
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
            )", "float CalculateSpecular(");
        }
    }
#pragma endregion

#pragma region calculate metalness
    if (ShaderHelper::sfind(code, "CalculateMetalness(") || ShaderHelper::sfind(code, "CalculateMetalnessLOD(")) {
        if (!ShaderHelper::sfind(code, "float CalculateMetalness(")) {
            ShaderHelper::insertStringRightBeforeLineContent(code, R"(
                float CalculateMetalness(in Layer inLayer, in float objectMetalness, in vec2 inUVs) {
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
                float CalculateMetalnessLOD(in Layer inLayer, in float objectMetalness, in vec2 inUVs, in float lod) {
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
            )", "float CalculateAO(");
        }
    }
#pragma endregion

#pragma region calculate smoothness
    if (ShaderHelper::sfind(code, "CalculateSmoothness(") || ShaderHelper::sfind(code, "CalculateSmoothnessLOD(")) {
        if (!ShaderHelper::sfind(code, "float CalculateSmoothness(")) {
            ShaderHelper::insertStringRightBeforeLineContent(code, R"(
                float CalculateSmoothness(in Layer inLayer, in float objectSmoothness, in vec2 inUVs) {
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
                float CalculateSmoothnessLOD(in Layer inLayer, in float objectSmoothness, in vec2 inUVs, in float lod) {
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
            )", "float CalculateMetalness(");
        }
    }
#pragma endregion

#pragma region calculate reflection
    if (ShaderHelper::sfind(code, "CalculateReflection(") || ShaderHelper::sfind(code, "CalculateReflectionLOD(")) {
        if (!ShaderHelper::sfind(code, "vec4 CalculateReflection(")) {
            ShaderHelper::insertStringRightBeforeLineContent(code, R"(
                vec4 CalculateReflection(in Layer inLayer, in vec2 inUVs, in vec4 inDiffuse, in vec3 inCameraPosition, in vec3 inNormals, in vec3 inWorldPosition){
                //    inUVs *= inLayer.uvModifications.zw;
                //    inUVs += inLayer.uvModifications.xy;
                    vec4 r = vec4(0.0);
                    r = textureCube(inLayer.cubemap, reflect(inNormals,normalize(inCameraPosition - inWorldPosition))) * texture2D(inLayer.mask, inUVs).r;
                    r.a *= inLayer.data2.x;
                    r = PaintersAlgorithm(r, inDiffuse);
                    return r;
                }
                vec4 CalculateReflectionLOD(in Layer inLayer, in vec2 inUVs, in vec4 inDiffuse, in vec3 inCameraPosition, in vec3 inNormals, in vec3 inWorldPosition, in float lod){
                //    inUVs *= inLayer.uvModifications.zw;
                //    inUVs += inLayer.uvModifications.xy;
                    vec4 r = vec4(0.0);
                    r = textureCubeLod(inLayer.cubemap, reflect(inNormals,normalize(inCameraPosition - inWorldPosition)), lod) * texture2D(inLayer.mask, inUVs).r;
                    r.a *= inLayer.data2.x;
                    r = PaintersAlgorithm(r, inDiffuse);
                    return r;
                }
            )", "float CalculateSmoothness(");
        }
    }
#pragma endregion

#pragma region calculate refraction
    if (ShaderHelper::sfind(code, "CalculateRefraction(") || ShaderHelper::sfind(code, "CalculateRefractionLOD(")) {
        if (!ShaderHelper::sfind(code, "vec4 CalculateRefraction(")) {
            ShaderHelper::insertStringRightBeforeLineContent(code, R"(
                vec4 CalculateRefraction(in Layer inLayer, in vec2 inUVs, in vec4 inDiffuse, in vec3 inCameraPosition, in vec3 inNormals, in vec3 inWorldPosition){
                //    inUVs *= inLayer.uvModifications.zw;
                //    inUVs += inLayer.uvModifications.xy;
                    vec4 r = vec4(0.0);
                    r = textureCube(inLayer.cubemap,refract(inNormals,normalize(inCameraPosition - inWorldPosition),1.0 / inLayer.data2.y)) * texture2D(inLayer.mask,inUVs).r;
                    r.a *= inLayer.data2.x;
                    r = PaintersAlgorithm(r, inDiffuse);
                    return r;
                }
                vec4 CalculateRefractionLOD(in Layer inLayer, in vec2 inUVs, in vec4 inDiffuse, in vec3 inCameraPosition, in vec3 inNormals, in vec3 inWorldPosition, in float lod){
                //    inUVs *= inLayer.uvModifications.zw;
                //    inUVs += inLayer.uvModifications.xy;
                    vec4 r = vec4(0.0);
                    r = textureCubeLod(inLayer.cubemap,refract(inNormals, normalize(inCameraPosition - inWorldPosition), 1.0 / inLayer.data2.y), lod) * texture2D(inLayer.mask, inUVs).r;
                    r.a *= inLayer.data2.x;
                    r = PaintersAlgorithm(r, inDiffuse);
                    return r;
                }
            )", "vec4 CalculateReflection(");
        }
    }
#pragma endregion

#pragma region calculate parallax 
    if (ShaderHelper::sfind(code, "CalculateParallaxMap(") || ShaderHelper::sfind(code, "CalculateParallaxMapLOD(")) {
        if (!ShaderHelper::sfind(code, "vec2 CalculateParallaxMap(")) {
            ShaderHelper::insertStringRightBeforeLineContent(code, 
                "vec2 CalculateParallaxMap(in Layer inLayer, vec3 _ViewDir, in vec2 inUVs){\n"
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
                "vec2 CalculateParallaxMapLOD(in Layer inLayer, vec3 _ViewDir, in vec2 inUVs, in float lod){\n"
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
            , "vec4 CalculateRefraction(");
        }
    }
#pragma endregion

#pragma region do blend
    //https://docs.gimp.org/2.4/en/gimp-concepts-layer-modes.html
    if (ShaderHelper::sfind(code, "DoBlend(")) {
        if (!ShaderHelper::sfind(code, "vec4 DoBlend(")) {
            ShaderHelper::insertStringRightBeforeLineContent(code, 
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
                "}\n"
            , "vec2 CalculateParallaxMap(");
        }
    }
#pragma endregion

}
