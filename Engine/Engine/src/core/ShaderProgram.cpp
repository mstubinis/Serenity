#include "core/ShaderProgram.h"
#include "core/engine/Engine.h"
#include "core/Camera.h"
#include "core/Scene.h"

#include <boost/filesystem.hpp>
#include <regex>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/iostreams/stream.hpp>
#include <iostream>

using namespace Engine;
using namespace std;

typedef boost::iostreams::stream<boost::iostreams::mapped_file_source> boost_stream_mapped_file;

bool sfind(string whole,string part){ if(whole.find(part) != string::npos) return true; return false; }
void insertStringAtLine(string& src, const string& newcontent,uint line){   
    if(line == 0){src=newcontent+"\n"+src;}else{istringstream str(src);string l; vector<string> lines;uint c=0;
        while(getline(str,l)){lines.push_back(l+"\n");if(c==line){lines.push_back(newcontent+"\n");}++c;}src="";for(auto& ln:lines){src+=ln;}
    }
}
void insertStringAtAndReplaceLine(string& src, const string& newcontent,uint line){
    istringstream str(src);string l;vector<string> lines;while(getline(str,l)){lines.push_back(l+"\n");}
    uint c = 0;src="";for(auto& ln:lines){if(c==line)ln=newcontent+"\n";src+=ln;++c;}
}
void insertStringAtEndOfMainFunc(string& src, const string& content){
    uint p=src.size()-1;while(p>0){char c=src[p];--p;if(c=='}'){break;}}src.insert(p,content);
}
void insertStringRightAfterLineContent(string& src, const string& newContent,const string& lineContent){
    istringstream str(src);string l; vector<string> lines; bool a = false;
    while(getline(str,l)){lines.push_back(l+"\n");if(sfind(l,lineContent) && !a){lines.push_back(newContent+"\n"); a=true;}}src="";for(auto& ln:lines){src+=ln;}
}
//this needs some work...?
string getLogDepthFunctions(){
    string res =  "\n"
        "vec3 GetWorldPosition(vec2 _uv,float _near, float _far){//generated\n"
        "    float depth = texture2D(gDepthMap, _uv).r;\n"
        "    float position_w = pow(2.0, depth * log2(_far + 1.0)) - 1.0;\n"
        "    float a = _far / (_far - _near);\n"
        "    float b = _far * _near / (_near - _far);\n"
        "    float linear = a + b / position_w;\n"
        "    vec4 wpos = CameraInvViewProj * (vec4(_uv, linear, 1.0) * 2.0 - 1.0);\n"
        "    return wpos.xyz / wpos.w;\n"
        "}\n"
        "vec3 GetViewPosition(vec2 _uv,float _near, float _far){//generated\n"
        "    float depth = texture2D(gDepthMap, _uv).r;\n"
        "    float position_w = pow(2.0, depth * log2(_far + 1.0)) - 1.0;\n"
        "    float a = _far / (_far - _near);\n"
        "    float b = _far * _near / (_near - _far);\n"
        "    float linear = a + b / position_w;\n"
        "    vec4 wpos = CameraInvProj * (vec4(_uv, linear, 1.0) * 2.0 - 1.0);\n"
        "    return wpos.xyz / wpos.w;\n"
        "}\n";
    return res;
}


//this is working great right now, do not modify
string getNormalDepthFunctions(){
    string res = "\n"
        "vec3 GetWorldPosition(vec2 _uv,float _near, float _far){//generated\n"
        "    float depth = texture2D(gDepthMap, _uv).r * 2.0 - 1.0;\n"
        "	 vec4 space = vec4(_uv * 2.0 - 1.0, depth, 1.0);\n"
        "	 space = CameraInvViewProj * space;\n"
        "	 return space.xyz / space.w;\n"
        "}\n"
        "vec3 GetViewPosition(vec2 _uv,float _near, float _far){//generated\n"
        "    float depth = texture2D(gDepthMap, _uv).r * 2.0 - 1.0;\n"
        "	 vec4 space = vec4(_uv * 2.0 - 1.0, depth, 1.0);\n"
        "	 space = CameraInvProj * space;\n"
        "	 return space.xyz / space.w;\n"
        "}\n";
    return res;
}

UniformBufferObject* UniformBufferObject::UBO_CAMERA = nullptr;

namespace Engine{
    namespace epriv{
        struct DefaultShaderBindFunctor{void operator()(EngineResource* r) const {
            Scene* scene = Resources::getCurrentScene();  if(!scene) return;
            Camera* camera = scene->getActiveCamera();    if(!camera) return;
            Camera& c = *camera;

            float fcoeff = (2.0f / glm::log2(c.getFar() + 1.0f)) * 0.5f;
            Renderer::sendUniform1Safe("fcoeff",fcoeff);

            //yes this is needed
            if(RenderManager::GLSL_VERSION < 140){
                Renderer::sendUniformMatrix4Safe("CameraViewProj",c.getViewProjection());
            }
        }};
        struct DefaultShaderUnbindFunctor{void operator()(EngineResource* r) const {
        }};
    };
};
GLint UniformBufferObject::MAX_UBO_BINDINGS;
uint UniformBufferObject::CUSTOM_UBO_AUTOMATIC_COUNT = 0;

Shader::Shader(string filenameOrCode, ShaderType::Type shaderType,bool fromFile){
    m_FileName = filenameOrCode;
    m_Type = shaderType;
    m_FromFile = fromFile;
    if (fromFile) {
        setName(filenameOrCode);
        m_FileName = filenameOrCode;
        m_Code = "";
    }else{
        setName("NULL");
        m_FileName = "";
        m_Code = filenameOrCode;
    }
}
Shader::~Shader(){ }
ShaderType::Type Shader::type(){ return m_Type; }
string Shader::data(){ return m_Code; }
bool Shader::fromFile(){ return m_FromFile; }

ShaderP::ShaderP(string _name, Shader& vs, Shader& fs):
m_VertexShader(vs), m_FragmentShader(fs){
    m_LoadedGPU = m_LoadedCPU = false;

    setCustomBindFunctor(epriv::DefaultShaderBindFunctor());
    setCustomUnbindFunctor(epriv::DefaultShaderUnbindFunctor());
    setName(_name);

    string& name_ = name();
    if (vs.name() == "NULL") vs.setName(name_ + ".vert");
    if (fs.name() == "NULL") fs.setName(name_ + ".frag");
    load();
    registerEvent(EventType::WindowFullscreenChanged);
}
ShaderP::~ShaderP(){ 
    unregisterEvent(EventType::WindowFullscreenChanged);
    unload(); 
}
void ShaderP::_convertCode(string& vCode, string& fCode, ShaderP& super) {
    _convertCode(vCode, m_VertexShader, super);
    _convertCode(fCode, m_FragmentShader, super);
    m_VertexShader.m_Code = vCode;
    m_FragmentShader.m_Code = fCode;
}
void ShaderP::_convertCode(string& _d, Shader& shader, ShaderP& super) {
    istringstream str(_d);

    //see if we actually have a version line
    string versionLine;
    if (sfind(_d, "#version ")) {
        //use the found one
        while (true) {
            getline(str, versionLine); if (sfind(versionLine, "#version ")) { break; }
        }
    }else{
        //generate one
        string core = "";
        if (epriv::RenderManager::GLSL_VERSION >= 330) core = " core";
        versionLine = "#version " + boost::lexical_cast<string>(epriv::RenderManager::GLSL_VERSION) + core + "\n";
        insertStringAtLine(_d, versionLine, 0);
    }
    string versionNumberString = regex_replace(versionLine, regex("([^0-9])"), "");
    uint versionNumber = boost::lexical_cast<uint>(versionNumberString);

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

    //check for normal map texture extraction
    //refer to mesh.cpp dirCorrection comment about using an uncompressed normal map and not reconstructing z
    if (sfind(_d, "CalcBumpedNormal(") || sfind(_d, "CalcBumpedNormalCompressed(")) {
        if (!sfind(_d, "vec3 CalcBumpedNormal(")) {
            if (sfind(_d, "varying mat3 TBN;")) {
                boost::replace_all(_d, "varying mat3 TBN;", "");
            }
            string normal_map =
                "varying mat3 TBN;\n"
                "vec3 CalcBumpedNormal(vec2 _uv,sampler2D _inTexture){//generated\n"
                "    vec3 _t = (texture2D(_inTexture, _uv).xyz) * 2.0 - 1.0;\n"
                "    return normalize(TBN * _t);\n"
                "}\n"
                "vec3 CalcBumpedNormalCompressed(vec2 _uv,sampler2D _inTexture){//generated\n"
                "    vec2 _t = (texture2D(_inTexture, _uv).yx) * 2.0 - 1.0;\n" //notice the yx flip, its needed
                "    float _z = sqrt(1.0 - _t.x * _t.x - _t.y * _t.y);\n"
                "    vec3 normal = vec3(_t.xy, _z);\n"//recalc z in the shader
                "    return normalize(TBN * normal);\n"
                "}\n";
            insertStringAtLine(_d, normal_map, 1);
        }
    }
    //check for compression functions

    //designed to work with floats from 0 to 1, packing them into an 8 bit component of a render target
    if (sfind(_d, "Pack2NibblesInto8BitChannel(")) {
        if (!sfind(_d, "float Pack2NibblesInto8BitChannel(")) {
            string pack_nibble = "\n"
                "float Pack2NibblesInto8BitChannel(float x,float y){\n"
                "    float xF = round(x / 0.0666);\n"
                "    float yF = round(y / 0.0666) * 16.0;\n"
                "    return (xF + yF) / 255.0;\n"
                "}\n";
            insertStringAtLine(_d, pack_nibble, 1);
        }
    }
    if (sfind(_d, "Unpack2NibblesFrom8BitChannel(")) {
        if (!sfind(_d, "vec2 Unpack2NibblesFrom8BitChannel(")) {
            string unpack_nibble = "\n"
                "vec2 Unpack2NibblesFrom8BitChannel(float data){\n"
                "    float d = data * 255.0;\n"
                "    float y = fract(d / 16.0);\n"
                "    float x = (d - (y * 16.0));\n"
                "    return vec2(y, x / 255.0);\n"
                "}\n";
            insertStringAtLine(_d, unpack_nibble, 1);
        }
    }
    //check for painters algorithm
    if (sfind(_d, "PaintersAlgorithm(")) {
        if (!sfind(_d, "vec4 PaintersAlgorithm(")) {
            string painters = "\n"
                "vec4 PaintersAlgorithm(vec4 paintbrush, vec4 canvas){//generated\n"
                "    float paintA = paintbrush.a;\n"
                "    float canvasA = canvas.a;\n"
                "    float Alpha = paintA + canvasA * (1.0 - paintA);\n"
                "    vec3 r = (paintbrush.rgb * paintA + canvas.rgb * canvasA * (1.0 - paintA)) / Alpha;\n"
                "    return vec4(r,Alpha);\n"
                "}\n";
            insertStringAtLine(_d, painters, 1);
        }
    }
    //see if we need a UBO for the camera
    if (sfind(_d, "CameraView") || 
    sfind(_d, "CameraProj") || 
    sfind(_d, "CameraViewProj") || 
    sfind(_d, "CameraPosition") ||
    sfind(_d, "CameraInvView") || 
    sfind(_d, "CameraInvProj") ||
    sfind(_d, "CameraInvViewProj") || 
    sfind(_d, "CameraNear") || 
    sfind(_d, "CameraFar") || 
    sfind(_d, "CameraInfo1") ||
    sfind(_d, "CameraInfo2") || 
    sfind(_d, "CameraViewVector") || 
    sfind(_d, "CameraRealPosition") || 
    sfind(_d, "CameraInfo3")) {
        string uboCameraString;
        if (versionNumber >= 140) { //UBO
            if (!sfind(_d, "layout (std140) uniform Camera //generated")) {
                uboCameraString = "\n"
                    "layout (std140) uniform Camera //generated\n"
                    "{\n"
                    "    mat4 CameraView;\n"
                    "    mat4 CameraProj;\n"
                    "    mat4 CameraViewProj;\n"
                    "    mat4 CameraInvView;\n"
                    "    mat4 CameraInvProj;\n"
                    "    mat4 CameraInvViewProj;\n"
                    "    vec4 CameraInfo1;\n"
                    "    vec4 CameraInfo2;\n"
                    "    vec4 CameraInfo3;\n"
                    "};\n"
                    "vec3 CameraPosition = CameraInfo1.xyz;\n"
                    "vec3 CameraViewVector = CameraInfo2.xyz;\n"
                    "vec3 CameraRealPosition = CameraInfo3.xyz;\n"
                    "float CameraNear = CameraInfo1.w;\n"
                    "float CameraFar = CameraInfo2.w;\n"
                    "\n";
                insertStringAtLine(_d, uboCameraString, 1);
            }
        }else{ //no UBO's, just add a uniform struct
            if (!sfind(_d, "uniform mat4 CameraView;//generated")) {
                uboCameraString = "\n"
                    "uniform mat4 CameraView;//generated;\n"
                    "uniform mat4 CameraProj;\n"
                    "uniform mat4 CameraViewProj;\n"
                    "uniform mat4 CameraInvView;\n"
                    "uniform mat4 CameraInvProj;\n"
                    "uniform mat4 CameraInvViewProj;\n"
                    "uniform vec4 CameraInfo1;\n"
                    "uniform vec4 CameraInfo2;\n"
                    "uniform vec4 CameraInfo3;\n"
                    "vec3 CameraPosition = CameraInfo1.xyz;\n"
                    "vec3 CameraViewVector = CameraInfo2.xyz;\n"
                    "vec3 CameraRealPosition = CameraInfo3.xyz;\n"
                    "float CameraNear = CameraInfo1.w;\n"
                    "float CameraFar = CameraInfo2.w;\n"
                    "\n";
                insertStringAtLine(_d, uboCameraString, 1);
            }
        }
    }
    //check for log depth - vertex
    if (sfind(_d, "USE_LOG_DEPTH_VERTEX") && !sfind(_d, "//USE_LOG_DEPTH_VERTEX") && shader.type() == ShaderType::Vertex) {
        boost::replace_all(_d, "USE_LOG_DEPTH_VERTEX", "");
        #ifndef ENGINE_FORCE_NO_LOG_DEPTH
        string log_vertex_code = "\n"
            "uniform float fcoeff;\n"
            "flat varying float FC;\n"
            "varying float logz_f;\n"
            "\n";
        insertStringAtLine(_d, log_vertex_code, 1);
        log_vertex_code = "\n"
            "logz_f = 1.0 + gl_Position.w;\n"
            "gl_Position.z = (log2(max(0.000001, logz_f)) * fcoeff - 1.0) * gl_Position.w;\n" //this line is optional i think... since gl_FragDepth may be written manually
            "FC = fcoeff;\n"
            "\n";
        insertStringAtEndOfMainFunc(_d, log_vertex_code);
        #endif
    }

    //check for view space normals from world (not sure if this is proper)
    if (sfind(_d, "GetViewNormalsFromWorld(")) {
        if (!sfind(_d, "vec4 GetViewNormalsFromWorld(")) {
            string viewNormals = "\n"
                "vec3 GetViewNormalsFromWorld(vec3 worldNormals,mat4 camView){//generated\n"
                "    return (camView * vec4(worldNormals,0.0)).xyz;\n"
                "}\n";
            insertStringAtLine(_d, viewNormals, 1);
        }
    }
    //check for world space normals from view (this works perfectly)
    if (sfind(_d, "GetWorldNormalsFromView(")) {
        if (!sfind(_d, "vec4 GetWorldNormalsFromView(")) {
            string viewNormals = "\n"
                "vec3 GetWorldNormalsFromView(vec3 viewNormals,mat4 camView){//generated\n"
                "    return (transpose(camView) * vec4(viewNormals,0.0)).xyz;\n"
                "}\n";
            insertStringAtLine(_d, viewNormals, 1);
        }
    }

    //check for log depth - fragment
    if (sfind(_d, "USE_LOG_DEPTH_FRAGMENT") && !sfind(_d, "//USE_LOG_DEPTH_FRAGMENT") && shader.type() == ShaderType::Fragment) {
        boost::replace_all(_d, "USE_LOG_DEPTH_FRAGMENT", "");
        #ifndef ENGINE_FORCE_NO_LOG_DEPTH
        string log_frag_code = "\n"
            "flat varying float FC;\n"
            "varying float logz_f;\n"
            "\n";
        insertStringAtLine(_d, log_frag_code, 1);
        log_frag_code = "\n"
            "gl_FragDepth = log2(logz_f) * FC;\n"
            "\n";
        insertStringAtEndOfMainFunc(_d, log_frag_code);
        #endif
        if (sfind(_d, "GetWorldPosition(") || sfind(_d, "GetViewPosition(")) {
            if (!sfind(_d, "vec3 GetWorldPosition(")) {
                #ifndef ENGINE_FORCE_NO_LOG_DEPTH
                    insertStringRightAfterLineContent(_d, getLogDepthFunctions(), "uniform sampler2D gDepthMap;");
                #else
                    insertStringRightAfterLineContent(_d, getNormalDepthFunctions(), "uniform sampler2D gDepthMap;");
                #endif
            }
        }
    }else{
        if (sfind(_d, "GetWorldPosition(") || sfind(_d, "GetViewPosition(")) {
            if (!sfind(_d, "vec3 GetWorldPosition(")) {
                if (sfind(_d, "USE_LOG_DEPTH_FRAG_WORLD_POSITION") && !sfind(_d, "//USE_LOG_DEPTH_FRAG_WORLD_POSITION") && shader.type() == ShaderType::Fragment) {
                    //log
                    boost::replace_all(_d, "USE_LOG_DEPTH_FRAG_WORLD_POSITION", "");
                    #ifndef ENGINE_FORCE_NO_LOG_DEPTH
                        insertStringRightAfterLineContent(_d, getLogDepthFunctions(), "uniform sampler2D gDepthMap;");
                    #else
                        insertStringRightAfterLineContent(_d, getNormalDepthFunctions(), "uniform sampler2D gDepthMap;");
                    #endif
                }else{
                    //normal
                    insertStringRightAfterLineContent(_d, getNormalDepthFunctions(), "uniform sampler2D gDepthMap;");
                }
            }
        }
    }
    //deal with layout (location = X) in
    if (versionNumber < 330) {
        if (shader.type() == ShaderType::Vertex) {
            if (sfind(_d, "layout") && sfind(_d, "location") && sfind(_d, "=")) {
                if (versionNumber > 130) {
                    if (epriv::OpenGLExtensionEnum::supported(epriv::OpenGLExtensionEnum::EXT_separate_shader_objects)) {
                        insertStringAtLine(_d, "#extension GL_EXT_seperate_shader_objects : enable", 1);
                    }else if (epriv::OpenGLExtensionEnum::supported(epriv::OpenGLExtensionEnum::ARB_separate_shader_objects)) {
                        insertStringAtLine(_d, "#extension GL_ARB_seperate_shader_objects : enable", 1);
                    }if (epriv::OpenGLExtensionEnum::supported(epriv::OpenGLExtensionEnum::EXT_explicit_attrib_location)) {
                        insertStringAtLine(_d, "#extension GL_EXT_explicit_attrib_location : enable", 1);
                    }else if (epriv::OpenGLExtensionEnum::supported(epriv::OpenGLExtensionEnum::ARB_explicit_attrib_location)) {
                        insertStringAtLine(_d, "#extension GL_ARB_explicit_attrib_location : enable", 1);
                    }
                }else{
                    //replace with attribute
                    istringstream str(_d); string line; uint count = 0;
                    while (getline(str, line)) {
                        if (sfind(line, "layout") && sfind(line, "location") && sfind(line, "=")) {
                            for (auto& type : _types) {
                                size_t found = line.find(type);
                                size_t firstFound = line.find("layout");
                                if (firstFound != string::npos && found != string::npos) {
                                    string _part1 = line.substr(0, firstFound);
                                    line.erase(0, found);
                                    line = _part1 + "attribute " + line;
                                    insertStringAtAndReplaceLine(_d, line, count);
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
        for (uint i = 0; i < 100; ++i) {
            const string fragDataStr = "gl_FragData[" + to_string(i) + "]";
            if (sfind(_d, fragDataStr)) {
                const string outFragData = "FRAG_COL_" + to_string(i);
                if (versionNumber >= 130 && versionNumber < 330) {
                    insertStringAtLine(_d, "out vec4 " + outFragData + ";", 1);
                }else if (versionNumber >= 330) {
                    insertStringAtLine(_d, "layout (location = " + to_string(i) + ") out vec4 " + outFragData + ";", 1);
                }
                boost::replace_all(_d, fragDataStr, outFragData);
            }
        }
    }
    if (versionNumber >= 110) {
        if (shader.type() == ShaderType::Vertex) {
            boost::replace_all(_d, "flat", "");
            boost::replace_all(_d, "highp ", "");
            boost::replace_all(_d, "mediump ", "");
            boost::replace_all(_d, "lowp ", "");
        }else if (shader.type() == ShaderType::Fragment) {
            boost::replace_all(_d, "flat", "");
            boost::replace_all(_d, "highp ", "");
            boost::replace_all(_d, "mediump ", "");
            boost::replace_all(_d, "lowp ", "");
        }
    }
    if (versionNumber >= 130) {
        if (shader.type() == ShaderType::Vertex) {
            boost::replace_all(_d, "varying", "out");
        }else if (shader.type() == ShaderType::Fragment) {
            boost::replace_all(_d, "varying", "in");
            boost::replace_all(_d, "gl_FragColor", "FRAG_COL");
            insertStringAtLine(_d, "out vec4 FRAG_COL;", 1);
        }
    }
    if (versionNumber >= 140) {
        if (shader.type() == ShaderType::Vertex) {
        }else if (shader.type() == ShaderType::Fragment) {
            boost::replace_all(_d, "textureCube(", "texture(");
            boost::replace_all(_d, "textureCubeLod(", "textureLod(");
            boost::replace_all(_d, "texture2DLod(", "textureLod(");
            boost::replace_all(_d, "texture2D(", "texture(");
        }
    }
    if (versionNumber >= 150) {
        if (shader.type() == ShaderType::Vertex) {
        }else if (shader.type() == ShaderType::Fragment) {
        }
    }
    if (versionNumber >= 330) {
        if (shader.type() == ShaderType::Vertex) {
            //attribute to layout (location = X) in
            istringstream str(_d);
            string line;
            uint count = 0;
            uint aCount = 0;
            while (getline(str, line)) {
                if (sfind(line, "attribute")) {
                    for (auto& type : _types) {
                        size_t found = line.find(type);
                        size_t firstFound = line.find("attribute");
                        if (firstFound != string::npos && found != string::npos) {
                            string _part1 = line.substr(0, firstFound);
                            line.erase(0, found);
                            line = _part1 + "layout (location = " + to_string(aCount) + ") in " + line;
                            insertStringAtAndReplaceLine(_d, line, count);
                            if (!sfind(_part1, "//") && !sfind(_part1, "/*") && !sfind(_part1, "///")) { //do we need to test for triple slashes?
                                ++aCount;
                            }
                            break;
                        }
                    }
                }
                ++count;
            }
        }else if (shader.type() == ShaderType::Fragment) {
        }
    }
}
void ShaderP::_load_CPU(ShaderP& super) {
    _unload_CPU(super);
    if (!m_LoadedCPU) {
        string VertexCode, FragmentCode = "";
        //load initial code
        if (m_VertexShader.fromFile()) {
            boost_stream_mapped_file str(m_VertexShader.m_FileName);
            for (string line; getline(str, line, '\n');) { VertexCode += "\n" + line; }
        }else { VertexCode = m_VertexShader.data(); }
        if (m_FragmentShader.fromFile()) {
            boost_stream_mapped_file str(m_FragmentShader.m_FileName);
            for (string line; getline(str, line, '\n');) { FragmentCode += "\n" + line; }
        }else { FragmentCode = m_FragmentShader.data(); }
        //convert the code
        _convertCode(VertexCode, FragmentCode, super);
        m_LoadedCPU = true;
    }
}
void ShaderP::_unload_CPU(ShaderP& super) {
    if (m_LoadedCPU) {
        m_LoadedCPU = false;
    }
}
void ShaderP::_load_GPU(ShaderP& super) {
    _unload_GPU(super);
    if (!m_LoadedGPU) {
        string& VertexCode = m_VertexShader.m_Code; string& FragmentCode = m_FragmentShader.m_Code;
        GLuint vid = glCreateShader(GL_VERTEX_SHADER); GLuint fid = glCreateShader(GL_FRAGMENT_SHADER);
        GLint res = GL_FALSE; int ll;
        // Compile Vertex Shader
        char const* vss = VertexCode.c_str(); glShaderSource(vid, 1, &vss, NULL); glCompileShader(vid);
        // Check Vertex Shader
        glGetShaderiv(vid, GL_COMPILE_STATUS, &res); glGetShaderiv(vid, GL_INFO_LOG_LENGTH, &ll); vector<char>ve(ll);
        glGetShaderInfoLog(vid, ll, NULL, &ve[0]);
        if (res == GL_FALSE) {
            if (m_VertexShader.fromFile()) { cout << "VertexShader Log (" + m_VertexShader.m_FileName + "): " << endl; }
            else { cout << "VertexShader Log (" + m_VertexShader.name() + "): " << endl; }
            cout << &ve[0] << endl;
        }
        // Compile Fragment Shader
        char const* fss = FragmentCode.c_str(); glShaderSource(fid, 1, &fss, NULL); glCompileShader(fid);
        // Check Fragment Shader
        glGetShaderiv(fid, GL_COMPILE_STATUS, &res); glGetShaderiv(fid, GL_INFO_LOG_LENGTH, &ll); vector<char>fe(ll);
        glGetShaderInfoLog(fid, ll, NULL, &fe[0]);
        if (res == GL_FALSE) {
            if (m_FragmentShader.fromFile()) { cout << "FragmentShader Log (" + m_FragmentShader.m_FileName + "): " << endl; }
            else { cout << "FragmentShader Log (" + m_FragmentShader.name() + "): " << endl; }
            cout << &fe[0] << endl;
        }
        // Link the program id
        m_ShaderProgram = glCreateProgram();
        glAttachShader(m_ShaderProgram, vid); glAttachShader(m_ShaderProgram, fid);

        for (uint i = 0; i < 100; ++i) {
            string outFragCol = "out vec4 FRAG_COL_" + to_string(i) + ";//130";
            if (sfind(FragmentCode, outFragCol)) {
                glBindFragDataLocation(m_ShaderProgram, i, string("FRAG_COL_" + to_string(i)).c_str());
            }
        }

        glLinkProgram(m_ShaderProgram);
        glDetachShader(m_ShaderProgram, vid); glDetachShader(m_ShaderProgram, fid);
        glDeleteShader(vid); glDeleteShader(fid);
        // Check the program
        glGetProgramiv(m_ShaderProgram, GL_LINK_STATUS, &res); glGetProgramiv(m_ShaderProgram, GL_INFO_LOG_LENGTH, &ll); vector<char>pe(std::max(ll, int(1)));
        glGetProgramInfoLog(m_ShaderProgram, ll, NULL, &pe[0]);
        if (res == GL_FALSE) { cout << "ShaderProgram Log : " << endl; cout << &pe[0] << endl; }
        //populate uniform table
        if (res == GL_TRUE) {
            GLint _i, _count, _size; GLenum _type;
            const GLsizei _bufSize = 256; // maximum name length
            GLchar _name[_bufSize]; // variable name in GLSL
            GLsizei _length; // name length
            glGetProgramiv(m_ShaderProgram, GL_ACTIVE_UNIFORMS, &_count);
            for (_i = 0; _i < _count; ++_i) {
                glGetActiveUniform(m_ShaderProgram, (GLuint)_i, _bufSize, &_length, &_size, &_type, _name);
                if (_length > 0) {
                    string _name1((char*)_name, _length);
                    GLint _loc = glGetUniformLocation(m_ShaderProgram, _name);
                    m_UniformLocations.emplace(_name1, _loc);
                }
            }
        }
        //link UBO's
        if (sfind(VertexCode, "layout (std140) uniform Camera //generated") || sfind(FragmentCode, "layout (std140) uniform Camera //generated")) {
            UniformBufferObject::UBO_CAMERA->attachToShader(super);
        }
        m_LoadedGPU = true;
    }
}
void ShaderP::_unload_GPU(ShaderP& super) {
    if (m_LoadedGPU) {
        m_UniformLocations.clear();
        m_AttachedUBOs.clear();
        glDeleteProgram(m_ShaderProgram);
        m_LoadedGPU = false;
    }
}
GLuint ShaderP::program(){ return m_ShaderProgram; }

void epriv::InternalShaderProgramPublicInterface::LoadCPU(ShaderP& shaderP){
    //if(!shaderP.isLoaded()){
        shaderP._load_CPU(shaderP);
    //}
}
void epriv::InternalShaderProgramPublicInterface::LoadGPU(ShaderP& shaderP){
    //if(!shaderP.isLoaded()){
        shaderP._load_GPU(shaderP);
        shaderP.EngineResource::load();
    //}
}
void epriv::InternalShaderProgramPublicInterface::UnloadCPU(ShaderP& shaderP){
    //if(shaderP.isLoaded()){
        shaderP._unload_CPU(shaderP);
        shaderP.EngineResource::unload();
    //}
}
void epriv::InternalShaderProgramPublicInterface::UnloadGPU(ShaderP& shaderP){
    //if(shaderP.isLoaded()){
        shaderP._unload_GPU(shaderP);        
    //}
}
void ShaderP::load(){
    if(!isLoaded()){
        auto& _this = *this;
        _load_CPU(_this);
        _load_GPU(_this);
        cout << "(Shader Program) ";
        EngineResource::load();
    }
}
void ShaderP::unload(){
    if(isLoaded() /*&& useCount() == 0*/){
        auto& _this = *this;
        _unload_GPU(_this);
        _unload_CPU(_this);
        cout << "(Shader Program) ";
        EngineResource::unload();
    }
}
void ShaderP::bind(){ epriv::Core::m_Engine->m_RenderManager._bindShaderProgram(this); }
void ShaderP::unbind(){ epriv::Core::m_Engine->m_RenderManager._unbindShaderProgram(); }
const unordered_map<string,GLint>& ShaderP::uniforms() const { return m_UniformLocations; }
void ShaderP::onEvent(const Event& e){
    if(e.type == EventType::WindowFullscreenChanged){
    }
}



UniformBufferObject::UniformBufferObject(const char* _nameInShader,uint _sizeofStruct,int _globalBindingPointNumber){ 
    m_NameInShader = _nameInShader;
    if (epriv::RenderManager::GLSL_VERSION < 140) return;
    if (_globalBindingPointNumber == -1) {
        //automatic assignment
        m_GlobalBindingPointNumber = (UniformBufferObject::MAX_UBO_BINDINGS - 1) - UniformBufferObject::CUSTOM_UBO_AUTOMATIC_COUNT;
        ++UniformBufferObject::CUSTOM_UBO_AUTOMATIC_COUNT;
        if (m_GlobalBindingPointNumber < 0) {
            cout << "Warning: Max UBO Limit reached!" << std::endl;
            m_GlobalBindingPointNumber = 0;
        }
    }else{
        m_GlobalBindingPointNumber = _globalBindingPointNumber;
    }
    m_SizeOfStruct = _sizeofStruct;
    _load_CPU();
    _load_GPU();
    registerEvent(EventType::WindowFullscreenChanged);
}
UniformBufferObject::~UniformBufferObject(){ 
    //unregisterEvent(EventType::WindowFullscreenChanged);
    _unload_GPU();
    _unload_CPU();
}
void UniformBufferObject::_load_CPU() {
    if (epriv::RenderManager::GLSL_VERSION < 140) return;
    _unload_CPU();
}
void UniformBufferObject::_unload_CPU() {
    if (epriv::RenderManager::GLSL_VERSION < 140) return;
}
void UniformBufferObject::_load_GPU() {
    if (epriv::RenderManager::GLSL_VERSION < 140) return;
    _unload_GPU();
    glGenBuffers(1, &m_UBOObject);
    glBindBuffer(GL_UNIFORM_BUFFER, m_UBOObject);//gen and bind buffer
    glBufferData(GL_UNIFORM_BUFFER, m_SizeOfStruct, NULL, GL_DYNAMIC_DRAW); //create buffer data storage
    glBindBufferBase(GL_UNIFORM_BUFFER, m_GlobalBindingPointNumber, m_UBOObject);//link UBO to it's global numerical index
}
void UniformBufferObject::_unload_GPU() {
    if (epriv::RenderManager::GLSL_VERSION < 140) return;
    glDeleteBuffers(1, &m_UBOObject);
}
void UniformBufferObject::updateData(void* _data){
    if (epriv::RenderManager::GLSL_VERSION < 140) return;
    glBindBuffer(GL_UNIFORM_BUFFER, m_UBOObject);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, m_SizeOfStruct, _data);
}
void UniformBufferObject::attachToShader(ShaderP& _shaderProgram){ 
    GLuint program = _shaderProgram.program();
    if (epriv::RenderManager::GLSL_VERSION < 140 || _shaderProgram.m_AttachedUBOs.count(m_UBOObject)) return;
    uint programBlockIndex = glGetUniformBlockIndex(program, m_NameInShader);
    glUniformBlockBinding(program, programBlockIndex, m_GlobalBindingPointNumber);
    _shaderProgram.m_AttachedUBOs.emplace(m_UBOObject, true);
}
GLuint UniformBufferObject::address(){ return m_UBOObject; }
void UniformBufferObject::onEvent(const Event& e){
    if(e.type == EventType::WindowFullscreenChanged){
        _load_GPU();
    }
}