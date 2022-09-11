
#include <serenity/renderer/opengl/glsl/VersionConversion.h>
#include <serenity/resources/shader/ShaderHelper.h>
#include <serenity/resources/material/MaterialEnums.h>
#include <boost/algorithm/string/replace.hpp>
#include <serenity/renderer/opengl/OpenGLContext.h>
#include <array>

#include <serenity/renderer/opengl/APIStateOpenGL.h>

//TODO: change from [const char* const] to [string] in c++20?
namespace {
    constexpr std::array<const char*, 30> TYPES { {
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
    } };
}
void Engine::priv::opengl::glsl::VersionConversion::convert(std::string& code, uint32_t versionNumber, ShaderType shaderType) {
    //deal with MRT binding points
    if (versionNumber >= 130) {
        if (shaderType == ShaderType::Fragment) {
            boost::replace_all(code, "gl_FragColor", "gl_FragData[0]");
        }
    }
    if (versionNumber >= 130) {
        for (GLint i = 0; i < Engine::priv::APIState<Engine::priv::OpenGL>::getConstants().MAX_DRAW_BUFFERS; ++i) {
            const std::string fragDataStr = "gl_FragData[" + std::to_string(i) + "]";
            if (ShaderHelper::sfind(code, fragDataStr)) {
                const std::string outFragData = "FRAG_COL_" + std::to_string(i);
                if (versionNumber >= 130 && versionNumber < 330) {
                    ShaderHelper::insertStringAtLine(code, "out vec4 " + outFragData + ";", 1);
                } else if (versionNumber >= 330) {
                    ShaderHelper::insertStringAtLine(code, "layout (location = " + std::to_string(i) + ") out vec4 " + outFragData + ";\n", 1);
                }
                boost::replace_all(code, fragDataStr, outFragData);
            }
        }
    }
    if (versionNumber >= 110) {
        //TODO: are these needed?
        /*
        if (shaderType == ShaderType::Vertex) {
            boost::replace_all(code, "highp ", "");
            boost::replace_all(code, "mediump ", "");
            boost::replace_all(code, "lowp ", "");
        } else if (shaderType == ShaderType::Fragment) {
            boost::replace_all(code, "highp ", "");
            boost::replace_all(code, "mediump ", "");
            boost::replace_all(code, "lowp ", "");
        }
        */
    }
    if (versionNumber >= 130) {
        if (shaderType == ShaderType::Vertex) {
            boost::replace_all(code, "varying ", "out ");
        } else if (shaderType == ShaderType::Fragment) {
            boost::replace_all(code, "varying ", "in ");
        }
    }
    if (versionNumber >= 140) {
        if (shaderType == ShaderType::Vertex) {
        } else if (shaderType == ShaderType::Fragment) {
            boost::replace_all(code, "textureCube(", "texture(");
            boost::replace_all(code, "textureCubeLod(", "textureLod(");
            boost::replace_all(code, "texture2DLod(", "textureLod(");
            boost::replace_all(code, "texture2D(", "texture(");
        }
    }
    if (versionNumber >= 150) {
        if (shaderType == ShaderType::Vertex) {
        } else if (shaderType == ShaderType::Fragment) {
        }
    }
    if (versionNumber >= 330) {
        if (shaderType == ShaderType::Vertex) {
            //attribute to layout (location = X) in
            std::istringstream str(code);
            std::string line;
            uint32_t count = 0;
            uint32_t aCount = 0;
            while (std::getline(str, line)) {
                if (ShaderHelper::sfind(line, "attribute")) {
                    for (const auto& type : TYPES) {
                        const size_t found      = line.find(type);
                        const size_t firstFound = line.find("attribute");
                        if (firstFound != std::string::npos && found != std::string::npos) {
                            const std::string part1_ = line.substr(0, firstFound);
                            line.erase(0, found);
                            line = part1_ + "layout (location = " + std::to_string(aCount) + ") in " + line;
                            ShaderHelper::insertStringAtAndReplaceLine(code, std::move(line), count);
                            ++aCount;
                            break;
                        }
                    }
                }
                ++count;
            }
        } else if (shaderType == ShaderType::Fragment) {
        }
    }
    //deal with layout (location = X) in
    if (versionNumber < 330) {
        if (shaderType == ShaderType::Vertex) {
            if (ShaderHelper::sfind(code, "layout") && ShaderHelper::sfind(code, "location") && ShaderHelper::sfind(code, "=")) {
                if (versionNumber > 130) {
                    if (OpenGLContext::supported(OpenGLContext::Extensions::EXT_separate_shader_objects)) {
                        code = "#extension GL_EXT_separate_shader_objects : enable\n" + code;
                    } else if (OpenGLContext::supported(OpenGLContext::Extensions::ARB_separate_shader_objects)) {
                        code = "#extension GL_ARB_separate_shader_objects : enable\n" + code;
                    }if (OpenGLContext::supported(OpenGLContext::Extensions::EXT_explicit_attrib_location)) {
                        code = "#extension GL_EXT_explicit_attrib_location : enable\n" + code;
                    } else if (OpenGLContext::supported(OpenGLContext::Extensions::ARB_explicit_attrib_location)) {
                        code = "#extension GL_ARB_explicit_attrib_location : enable\n" + code;
                    }
                } else {
                    //replace with attribute
                    std::istringstream str(code);
                    std::string line;
                    uint32_t count = 0;
                    while (std::getline(str, line)) {
                        if (ShaderHelper::sfind(line, "layout") && ShaderHelper::sfind(line, "location") && ShaderHelper::sfind(line, "=")) {
                            for (const auto& type : TYPES) {
                                const size_t found      = line.find(type);
                                const size_t firstFound = line.find("layout");
                                if (firstFound != std::string::npos && found != std::string::npos) {
                                    const std::string part1_ = line.substr(0, firstFound);
                                    line.erase(0, found);
                                    line = part1_ + "attribute " + line;
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
    if (OpenGLContext::isBindlessTexturesSupported()) {
        if (shaderType == ShaderType::Fragment) {
            //         ShaderHelper::insertStringAtLine(code, "layout(bindless_sampler) uniform", 1);

            if (OpenGLContext::supported(OpenGLContext::Extensions::ARB_bindless_texture)) {
                code = "#extension GL_ARB_bindless_texture : require\n" + code; //yes this is very much needed
            } else if (OpenGLContext::supported(OpenGLContext::Extensions::NV_bindless_texture)) {
                code = "#extension GL_NV_bindless_texture : require\n" + code; //yes this is very much needed
            }

            if (OpenGLContext::supported(OpenGLContext::Extensions::ARB_gpu_shader_int64)) {
                code = "#extension GL_ARB_gpu_shader_int64 : require\n" + code; // for uint64_t
            } else if (OpenGLContext::supported(OpenGLContext::Extensions::NV_gpu_shader5)) {
                code = "#extension GL_NV_gpu_shader5 : require\n" + code; // for uint64_t
            }
        }
    }
}