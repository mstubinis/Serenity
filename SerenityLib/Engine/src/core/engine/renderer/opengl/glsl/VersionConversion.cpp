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


void opengl::glsl::VersionConversion::convert(string& code, const unsigned int& versionNumber, const ShaderType::Type& shaderType) {
    vector<string> _types;
    _types.emplace_back("float");  _types.emplace_back("vec2");   _types.emplace_back("vec3");
    _types.emplace_back("vec4");   _types.emplace_back("mat3");   _types.emplace_back("mat4");

    _types.emplace_back("double"); _types.emplace_back("dvec2");  _types.emplace_back("dvec3");
    _types.emplace_back("dvec4");  _types.emplace_back("dmat3");  _types.emplace_back("dmat4");

    _types.emplace_back("int");    _types.emplace_back("ivec2");  _types.emplace_back("ivec3");
    _types.emplace_back("ivec4");  _types.emplace_back("imat3");  _types.emplace_back("imat4");

    _types.emplace_back("bool");   _types.emplace_back("bvec2");  _types.emplace_back("bvec3");
    _types.emplace_back("bvec4");  _types.emplace_back("bmat3");  _types.emplace_back("bmat4");

    _types.emplace_back("uint");   _types.emplace_back("uvec2");  _types.emplace_back("uvec3");
    _types.emplace_back("uvec4");  _types.emplace_back("umat3");  _types.emplace_back("umat4");

    //deal with layout (location = X) in
    if (versionNumber < 330) {
        if (shaderType == ShaderType::Vertex) {
            if (ShaderHelper::sfind(code, "layout") && ShaderHelper::sfind(code, "location") && ShaderHelper::sfind(code, "=")) {
                if (versionNumber > 130) {
                    if (OpenGLExtensions::supported(OpenGLExtensions::EXT_separate_shader_objects)) {
                        ShaderHelper::insertStringAtLine(code, "#extension GL_EXT_seperate_shader_objects : enable", 1);
                    }else if (OpenGLExtensions::supported(OpenGLExtensions::ARB_separate_shader_objects)) {
                        ShaderHelper::insertStringAtLine(code, "#extension GL_ARB_seperate_shader_objects : enable", 1);
                    }if (OpenGLExtensions::supported(OpenGLExtensions::EXT_explicit_attrib_location)) {
                        ShaderHelper::insertStringAtLine(code, "#extension GL_EXT_explicit_attrib_location : enable", 1);
                    }else if (OpenGLExtensions::supported(OpenGLExtensions::ARB_explicit_attrib_location)) {
                        ShaderHelper::insertStringAtLine(code, "#extension GL_ARB_explicit_attrib_location : enable", 1);
                    }
                }else{
                    //replace with attribute
                    istringstream str(code);
                    string line;
                    unsigned int count = 0;
                    while (getline(str, line)) {
                        if (ShaderHelper::sfind(line, "layout") && ShaderHelper::sfind(line, "location") && ShaderHelper::sfind(line, "=")) {
                            for (auto& type : _types) {
                                const size_t found = line.find(type);
                                const size_t firstFound = line.find("layout");
                                if (firstFound != string::npos && found != string::npos) {
                                    const string _part1 = line.substr(0, firstFound);
                                    line.erase(0, found);
                                    line = _part1 + "attribute " + line;
                                    ShaderHelper::insertStringAtAndReplaceLine(code, line, count);
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
            boost::replace_all(code, "flat ", "");
            boost::replace_all(code, "flat", "");
            boost::replace_all(code, "highp ", "");
            boost::replace_all(code, "mediump ", "");
            boost::replace_all(code, "lowp ", "");
        }else if (shaderType == ShaderType::Fragment) {
            boost::replace_all(code, "flat ", "");
            boost::replace_all(code, "flat", "");
            boost::replace_all(code, "highp ", "");
            boost::replace_all(code, "mediump ", "");
            boost::replace_all(code, "lowp ", "");
        }
    }
    if (versionNumber >= 130) {
        if (shaderType == ShaderType::Vertex) {
            boost::replace_all(code, " varying", "out");
            boost::replace_all(code, "varying", "out");
        }else if (shaderType == ShaderType::Fragment) {
            boost::replace_all(code, " varying", "in");
            boost::replace_all(code, "varying", "in");
            boost::replace_all(code, "gl_FragColor", "FRAG_COL");
            ShaderHelper::insertStringAtLine(code, "out vec4 FRAG_COL;", 1);
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
                    for (auto& type : _types) {
                        const size_t found = line.find(type);
                        const size_t firstFound = line.find("attribute");
                        if (firstFound != string::npos && found != string::npos) {
                            const string _part1 = line.substr(0, firstFound);
                            line.erase(0, found);
                            line = _part1 + "layout (location = " + to_string(aCount) + ") in " + line;
                            ShaderHelper::insertStringAtAndReplaceLine(code, line, count);
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
}