#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/renderer/opengl/glsl/VersionConversion.h>
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

//TODO: change from string_view to string in c++20?
constexpr std::array<std::string_view, 30> TYPES {
    "float",
    "vec2",
    "vec3",
    "vec4",
    "mat3",
    "mat4",
    "double",
    "dvec2",
    "dvec3",
    "dvec4",
    "dmat3",
    "dmat4",
    "int",
    "ivec2",
    "ivec3",
    "ivec4",
    "imat3",
    "imat4",
    "bool",
    "bvec2",
    "bvec3",
    "bvec4",
    "bmat3",
    "bmat4",
    "uint",
    "uvec2",
    "uvec3",
    "uvec4",
    "umat3",
    "umat4",
};

void opengl::glsl::VersionConversion::convert(string& code, unsigned int versionNumber, ShaderType shaderType) {
    //deal with MRT binding points
    if (versionNumber >= 130) {
        for (unsigned int i = 0; i < 100; ++i) {
            const string fragDataStr = "gl_FragData[" + to_string(i) + "]";
            if (ShaderHelper::sfind(code, fragDataStr)) {
                const string outFragData = "FRAG_COL_" + to_string(i);
                if (versionNumber >= 130 && versionNumber < 330) {
                    ShaderHelper::insertStringAtLine(code, "out vec4 " + outFragData + ";", 1);
                }else if (versionNumber >= 330) {
                    ShaderHelper::insertStringAtLine(code, "layout (location = " + to_string(i) + ") out vec4 " + outFragData + ";", 1);
                }
                boost::replace_all(code, fragDataStr, outFragData);
            }
        }
    }
    if (versionNumber >= 110) {
        if (shaderType == ShaderType::Vertex) {
            boost::replace_all(code, "highp ", "");
            boost::replace_all(code, "mediump ", "");
            boost::replace_all(code, "lowp ", "");
        }else if (shaderType == ShaderType::Fragment) {
            boost::replace_all(code, "highp ", "");
            boost::replace_all(code, "mediump ", "");
            boost::replace_all(code, "lowp ", "");
        }
    }
    if (versionNumber >= 130) {
        if (shaderType == ShaderType::Vertex) {
            boost::replace_all(code, "varying ", "out ");
        }else if (shaderType == ShaderType::Fragment) {
            boost::replace_all(code, "varying ", "in ");
            boost::replace_all(code, "gl_FragColor", "FRAG_COL");
            code = "out vec4 FRAG_COL;\n" + code;
        }
    }
    if (versionNumber >= 140) {
        if (shaderType == ShaderType::Vertex) {
        }else if (shaderType == ShaderType::Fragment) {
            boost::replace_all(code, "textureCube(", "texture(");
            boost::replace_all(code, "textureCubeLod(", "textureLod(");
            boost::replace_all(code, "texture2DLod(", "textureLod(");
            boost::replace_all(code, "texture2D(", "texture(");
        }
    }
    if (versionNumber >= 150) {
        if (shaderType == ShaderType::Vertex) {
        }else if (shaderType == ShaderType::Fragment) {
        }
    }
    if (versionNumber >= 330) {
        if (shaderType == ShaderType::Vertex) {
            //attribute to layout (location = X) in
            istringstream str(code);
            string line;
            unsigned int count = 0;
            unsigned int aCount = 0;
            while (getline(str, line)) {
                if (ShaderHelper::sfind(line, "attribute")) {
                    for (const auto& type : TYPES) {
                        const size_t found = line.find(type);
                        const size_t firstFound = line.find("attribute");
                        if (firstFound != string::npos && found != string::npos) {
                            const string _part1 = line.substr(0, firstFound);
                            line.erase(0, found);
                            line = _part1 + "layout (location = " + to_string(aCount) + ") in " + line;
                            ShaderHelper::insertStringAtAndReplaceLine(code, std::move(line), count);
                            if (!ShaderHelper::sfind(_part1, "//") && !ShaderHelper::sfind(_part1, "/*") && !ShaderHelper::sfind(_part1, "///")) { //do we need to test for triple slashes?
                                ++aCount;
                            }
                            break;
                        }
                    }
                }
                ++count;
            }
        }else if (shaderType == ShaderType::Fragment) {
        }
    }
    //deal with layout (location = X) in
    if (versionNumber < 330) {
        if (shaderType == ShaderType::Vertex) {
            if (ShaderHelper::sfind(code, "layout") && ShaderHelper::sfind(code, "location") && ShaderHelper::sfind(code, "=")) {
                if (versionNumber > 130) {
                    if (OpenGLExtensions::supported(OpenGLExtensions::EXT_separate_shader_objects)) {
                        code = "#extension GL_EXT_separate_shader_objects : enable\n" + code;
                    }else if (OpenGLExtensions::supported(OpenGLExtensions::ARB_separate_shader_objects)) {
                        code = "#extension GL_ARB_separate_shader_objects : enable\n" + code;
                    }if (OpenGLExtensions::supported(OpenGLExtensions::EXT_explicit_attrib_location)) {
                        code = "#extension GL_EXT_explicit_attrib_location : enable\n" + code;
                    }else if (OpenGLExtensions::supported(OpenGLExtensions::ARB_explicit_attrib_location)) {
                        code = "#extension GL_ARB_explicit_attrib_location : enable\n" + code;
                    }
                }else{
                    //replace with attribute
                    istringstream str(code);
                    string line;
                    unsigned int count = 0;
                    while (getline(str, line)) {
                        if (ShaderHelper::sfind(line, "layout") && ShaderHelper::sfind(line, "location") && ShaderHelper::sfind(line, "=")) {
                            for (const auto& type : TYPES) {
                                const size_t found = line.find(type);
                                const size_t firstFound = line.find("layout");
                                if (firstFound != string::npos && found != string::npos) {
                                    const string _part1 = line.substr(0, firstFound);
                                    line.erase(0, found);
                                    line = _part1 + "attribute " + line;
                                    ShaderHelper::insertStringAtAndReplaceLine(code, std::move(line), count);
                                    break;
                                }
                            }
                        }
                        ++count;
                    }
                }
            }
        }
    }
    //deal with bindless textures
    if (OpenGLExtensions::isBindlessTexturesSupported()) {
        if (shaderType == ShaderType::Fragment) {
            //         ShaderHelper::insertStringAtLine(code, "layout(bindless_sampler) uniform", 1);

            if (OpenGLExtensions::supported(OpenGLExtensions::ARB_bindless_texture)) {
                code = "#extension GL_ARB_bindless_texture : require\n" + code; //yes this is very much needed
            }else if (OpenGLExtensions::supported(OpenGLExtensions::NV_bindless_texture)) {
                code = "#extension GL_NV_bindless_texture : require\n" + code; //yes this is very much needed
            }

            if (OpenGLExtensions::supported(OpenGLExtensions::ARB_gpu_shader_int64)) {
                code = "#extension GL_ARB_gpu_shader_int64 : require\n" + code; // for uint64_t
            }else if (OpenGLExtensions::supported(OpenGLExtensions::NV_gpu_shader5)) {
                code = "#extension GL_NV_gpu_shader5 : require\n" + code; // for uint64_t
            }
        }
    }
}