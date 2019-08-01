#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/Engine.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/renderer/opengl/UniformBufferObject.h>
#include <core/engine/materials/MaterialEnums.h>

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/lexical_cast.hpp>

#include <core/engine/renderer/opengl/glsl/Common.h>
#include <core/engine/shaders/ShaderHelper.h>
#include <core/engine/shaders/Shader.h>


#include <regex>
#include <iostream>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

typedef boost::iostreams::stream<boost::iostreams::mapped_file_source> boost_stream_mapped_file;

//this needs some work...? seems to be ok for now
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
        "    float depth = texture2D(gDepthMap, _uv).x;\n"
        "    vec4 space = CameraInvProj * (vec4(_uv, depth, 1.0) * 2.0 - 1.0);\n"
        "    return space.xyz / space.w;\n"
        "}\n";
    return res;
}

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

ShaderProgram::ShaderProgram(string _name, Shader& vs, Shader& fs):m_VertexShader(vs), m_FragmentShader(fs){
    m_LoadedGPU = m_LoadedCPU = false;

    setCustomBindFunctor(epriv::DefaultShaderBindFunctor());
    setCustomUnbindFunctor(epriv::DefaultShaderUnbindFunctor());
    setName(_name);

    const string& name_ = name();
    if (vs.name() == "NULL") vs.setName(name_ + ".vert");
    if (fs.name() == "NULL") fs.setName(name_ + ".frag");
    load();
}
ShaderProgram::~ShaderProgram(){ 
    unload(); 
}
void ShaderProgram::_convertCode(string& vCode, string& fCode) {
    _convertCode(vCode, m_VertexShader);
    _convertCode(fCode, m_FragmentShader);
    m_VertexShader.m_Code = vCode;
    m_FragmentShader.m_Code = fCode;
}
void ShaderProgram::_convertCode(string& _d, Shader& shader) {
    istringstream str(_d);

    //see if we actually have a version line
    string versionLine;
    if (ShaderHelper::sfind(_d, "#version ")) {
        //use the found one
        while (true) {
            getline(str, versionLine); if (ShaderHelper::sfind(versionLine, "#version ")) { break; }
        }
    }else{
        //generate one
        string core = "";
        if (epriv::RenderManager::GLSL_VERSION >= 330) core = " core";
        versionLine = "#version " + to_string(epriv::RenderManager::GLSL_VERSION) + core + "\n";
        ShaderHelper::insertStringAtLine(_d, versionLine, 0);
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

    //common code
    epriv::opengl::glsl::Common::convert(_d);


    //see if we need a UBO for the camera
    if (ShaderHelper::sfind(_d, "CameraView") ||
    ShaderHelper::sfind(_d, "CameraProj") ||
    ShaderHelper::sfind(_d, "CameraViewProj") ||
    ShaderHelper::sfind(_d, "CameraPosition") ||
    ShaderHelper::sfind(_d, "CameraInvView") ||
    ShaderHelper::sfind(_d, "CameraInvProj") ||
    ShaderHelper::sfind(_d, "CameraInvViewProj") ||
    ShaderHelper::sfind(_d, "CameraNear") ||
    ShaderHelper::sfind(_d, "CameraFar") ||
    ShaderHelper::sfind(_d, "CameraInfo1") ||
    ShaderHelper::sfind(_d, "CameraInfo2") ||
    ShaderHelper::sfind(_d, "CameraViewVector") ||
    ShaderHelper::sfind(_d, "CameraRealPosition") ||
    ShaderHelper::sfind(_d, "CameraInfo3")) {
        string uboCameraString;
        if (versionNumber >= 140) { //UBO
            if (!ShaderHelper::sfind(_d, "layout (std140) uniform Camera //generated")) {
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
                ShaderHelper::insertStringAtLine(_d, uboCameraString, 1);
            }
        }else{ //no UBO's, just add a uniform struct
            if (!ShaderHelper::sfind(_d, "uniform mat4 CameraView;//generated")) {
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
                ShaderHelper::insertStringAtLine(_d, uboCameraString, 1);
            }
        }
    }
    //check for material limits
    if (ShaderHelper::sfind(_d, "USE_MAX_MATERIAL_LAYERS_PER_COMPONENT") && !ShaderHelper::sfind(_d, "//USE_MAX_MATERIAL_LAYERS_PER_COMPONENT") && shader.type() == ShaderType::Fragment) {
        boost::replace_all(_d, "USE_MAX_MATERIAL_LAYERS_PER_COMPONENT", "#define MAX_MATERIAL_LAYERS_PER_COMPONENT " + std::to_string(MAX_MATERIAL_LAYERS_PER_COMPONENT) + "\n");
    }
    if (ShaderHelper::sfind(_d, "USE_MAX_MATERIAL_COMPONENTS") && !ShaderHelper::sfind(_d, "//USE_MAX_MATERIAL_COMPONENTS") && shader.type() == ShaderType::Fragment) {
        boost::replace_all(_d, "USE_MAX_MATERIAL_COMPONENTS", "#define MAX_MATERIAL_COMPONENTS " + std::to_string(MAX_MATERIAL_COMPONENTS) + "\n");
    }

    //check for log depth - vertex
    if (ShaderHelper::sfind(_d, "USE_LOG_DEPTH_VERTEX") && !ShaderHelper::sfind(_d, "//USE_LOG_DEPTH_VERTEX") && shader.type() == ShaderType::Vertex) {
        boost::replace_all(_d, "USE_LOG_DEPTH_VERTEX", "");
        #ifndef ENGINE_FORCE_NO_LOG_DEPTH
            string log_vertex_code = "\n"
                "uniform float fcoeff;\n"
                "flat varying float FC;\n"
                "varying float logz_f;\n"
                "\n";
            ShaderHelper::insertStringAtLine(_d, log_vertex_code, 1);
            log_vertex_code = "\n"
                "logz_f = 1.0 + gl_Position.w;\n"
                "gl_Position.z = (log2(max(0.000001, logz_f)) * fcoeff - 1.0) * gl_Position.w;\n" //this line is optional i think... since gl_FragDepth may be written manually
                "FC = fcoeff;\n"
                "\n";
            ShaderHelper::insertStringAtEndOfMainFunc(_d, log_vertex_code);
        #endif
    }

    //check for view space normals from world (not sure if this is proper)
    if (ShaderHelper::sfind(_d, "GetViewNormalsFromWorld(")) {
        if (!ShaderHelper::sfind(_d, "vec4 GetViewNormalsFromWorld(")) {
            string viewNormals = "\n"
                "vec3 GetViewNormalsFromWorld(vec3 worldNormals,mat4 camView){//generated\n"
                "    return (camView * vec4(worldNormals,0.0)).xyz;\n"
                "}\n";
            ShaderHelper::insertStringAtLine(_d, viewNormals, 1);
        }
    }
    //check for world space normals from view (this works perfectly)
    if (ShaderHelper::sfind(_d, "GetWorldNormalsFromView(")) {
        if (!ShaderHelper::sfind(_d, "vec4 GetWorldNormalsFromView(")) {
            string viewNormals = "\n"
                "vec3 GetWorldNormalsFromView(vec3 viewNormals,mat4 camView){//generated\n"
                "    return (transpose(camView) * vec4(viewNormals,0.0)).xyz;\n"
                "}\n";
            ShaderHelper::insertStringAtLine(_d, viewNormals, 1);
        }
    }

    //check for log depth - fragment
    if (ShaderHelper::sfind(_d, "USE_LOG_DEPTH_FRAGMENT") && !ShaderHelper::sfind(_d, "//USE_LOG_DEPTH_FRAGMENT") && shader.type() == ShaderType::Fragment) {
        boost::replace_all(_d, "USE_LOG_DEPTH_FRAGMENT", "");
        #ifndef ENGINE_FORCE_NO_LOG_DEPTH
            string log_frag_code = "\n"
                "flat varying float FC;\n"
                "varying float logz_f;\n"
                "\n";
            ShaderHelper::insertStringAtLine(_d, log_frag_code, 1);
            log_frag_code = "\n"
                "gl_FragDepth = log2(logz_f) * FC;\n"
                "\n";
            ShaderHelper::insertStringAtEndOfMainFunc(_d, log_frag_code);
        #endif
        if (ShaderHelper::sfind(_d, "GetWorldPosition(") || ShaderHelper::sfind(_d, "GetViewPosition(")) {
            if (!ShaderHelper::sfind(_d, "vec3 GetWorldPosition(")) {
                #ifndef ENGINE_FORCE_NO_LOG_DEPTH
                ShaderHelper::insertStringRightAfterLineContent(_d, getLogDepthFunctions(), "uniform sampler2D gDepthMap;");
                #else
                ShaderHelper::insertStringRightAfterLineContent(_d, getNormalDepthFunctions(), "uniform sampler2D gDepthMap;");
                #endif
            }
        }
    }else{
        if (ShaderHelper::sfind(_d, "GetWorldPosition(") || ShaderHelper::sfind(_d, "GetViewPosition(")) {
            if (!ShaderHelper::sfind(_d, "vec3 GetWorldPosition(")) {
                if (ShaderHelper::sfind(_d, "USE_LOG_DEPTH_FRAG_WORLD_POSITION") && !ShaderHelper::sfind(_d, "//USE_LOG_DEPTH_FRAG_WORLD_POSITION") && shader.type() == ShaderType::Fragment) {
                    //log
                    boost::replace_all(_d, "USE_LOG_DEPTH_FRAG_WORLD_POSITION", "");
                    #ifndef ENGINE_FORCE_NO_LOG_DEPTH
                    ShaderHelper::insertStringRightAfterLineContent(_d, getLogDepthFunctions(), "uniform sampler2D gDepthMap;");
                    #else
                    ShaderHelper::insertStringRightAfterLineContent(_d, getNormalDepthFunctions(), "uniform sampler2D gDepthMap;");
                    #endif
                }else{
                    #ifndef ENGINE_FORCE_NO_LOG_DEPTH
                    ShaderHelper::insertStringRightAfterLineContent(_d, getLogDepthFunctions(), "uniform sampler2D gDepthMap;");
                    #else
                    ShaderHelper::insertStringRightAfterLineContent(_d, getNormalDepthFunctions(), "uniform sampler2D gDepthMap;");
                    #endif
                }
            }
        }
    }
    //deal with layout (location = X) in
    if (versionNumber < 330) {
        if (shader.type() == ShaderType::Vertex) {
            if (ShaderHelper::sfind(_d, "layout") && ShaderHelper::sfind(_d, "location") && ShaderHelper::sfind(_d, "=")) {
                if (versionNumber > 130) {
                    if (epriv::OpenGLExtension::supported(epriv::OpenGLExtension::EXT_separate_shader_objects)) {
                        ShaderHelper::insertStringAtLine(_d, "#extension GL_EXT_seperate_shader_objects : enable", 1);
                    }else if (epriv::OpenGLExtension::supported(epriv::OpenGLExtension::ARB_separate_shader_objects)) {
                        ShaderHelper::insertStringAtLine(_d, "#extension GL_ARB_seperate_shader_objects : enable", 1);
                    }if (epriv::OpenGLExtension::supported(epriv::OpenGLExtension::EXT_explicit_attrib_location)) {
                        ShaderHelper::insertStringAtLine(_d, "#extension GL_EXT_explicit_attrib_location : enable", 1);
                    }else if (epriv::OpenGLExtension::supported(epriv::OpenGLExtension::ARB_explicit_attrib_location)) {
                        ShaderHelper::insertStringAtLine(_d, "#extension GL_ARB_explicit_attrib_location : enable", 1);
                    }
                }else{
                    //replace with attribute
                    istringstream str(_d); string line; uint count = 0;
                    while (getline(str, line)) {
                        if (ShaderHelper::sfind(line, "layout") && ShaderHelper::sfind(line, "location") && ShaderHelper::sfind(line, "=")) {
                            for (auto& type : _types) {
                                size_t found = line.find(type);
                                size_t firstFound = line.find("layout");
                                if (firstFound != string::npos && found != string::npos) {
                                    string _part1 = line.substr(0, firstFound);
                                    line.erase(0, found);
                                    line = _part1 + "attribute " + line;
                                    ShaderHelper::insertStringAtAndReplaceLine(_d, line, count);
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
            if (ShaderHelper::sfind(_d, fragDataStr)) {
                const string outFragData = "FRAG_COL_" + to_string(i);
                if (versionNumber >= 130 && versionNumber < 330) {
                    ShaderHelper::insertStringAtLine(_d, "out vec4 " + outFragData + ";", 1);
                }else if (versionNumber >= 330) {
                    ShaderHelper::insertStringAtLine(_d, "layout (location = " + to_string(i) + ") out vec4 " + outFragData + ";", 1);
                }
                boost::replace_all(_d, fragDataStr, outFragData);
            }
        }
    }
    if (versionNumber >= 110) {
        if (shader.type() == ShaderType::Vertex) {
            boost::replace_all(_d, "flat ",    "");
            boost::replace_all(_d, "flat",     "");
            boost::replace_all(_d, "highp ",   "");
            boost::replace_all(_d, "mediump ", "");
            boost::replace_all(_d, "lowp ",    "");
        }else if (shader.type() == ShaderType::Fragment) {
            boost::replace_all(_d, "flat ",    "");
            boost::replace_all(_d, "flat",     "");
            boost::replace_all(_d, "highp ",   "");
            boost::replace_all(_d, "mediump ", "");
            boost::replace_all(_d, "lowp ",    "");
        }
    }
    if (versionNumber >= 130) {
        if (shader.type() == ShaderType::Vertex) {
            boost::replace_all(_d, " varying", "out");
            boost::replace_all(_d, "varying", "out");
        }else if (shader.type() == ShaderType::Fragment) {
            boost::replace_all(_d, " varying", "in");
            boost::replace_all(_d, "varying", "in");
            boost::replace_all(_d, "gl_FragColor", "FRAG_COL");
            ShaderHelper::insertStringAtLine(_d, "out vec4 FRAG_COL;", 1);
        }
    }
    if (versionNumber >= 140) {
        if (shader.type() == ShaderType::Vertex) {
        }else if (shader.type() == ShaderType::Fragment) {
            boost::replace_all(_d, "textureCube(",     "texture(");
            boost::replace_all(_d, "textureCubeLod(",  "textureLod(");
            boost::replace_all(_d, "texture2DLod(",    "textureLod(");
            boost::replace_all(_d, "texture2D(",       "texture(");
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
                if (ShaderHelper::sfind(line, "attribute")) {
                    for (auto& type : _types) {
                        size_t found = line.find(type);
                        size_t firstFound = line.find("attribute");
                        if (firstFound != string::npos && found != string::npos) {
                            string _part1 = line.substr(0, firstFound);
                            line.erase(0, found);
                            line = _part1 + "layout (location = " + to_string(aCount) + ") in " + line;
                            ShaderHelper::insertStringAtAndReplaceLine(_d, line, count);
                            if (!ShaderHelper::sfind(_part1, "//") && !ShaderHelper::sfind(_part1, "/*") && !ShaderHelper::sfind(_part1, "///")) { //do we need to test for triple slashes?
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
void ShaderProgram::_load_CPU() {
    _unload_CPU();
    if (!m_LoadedCPU) {
        string VertexCode, FragmentCode = "";
        //load initial code
        if (m_VertexShader.fromFile()) {
            boost_stream_mapped_file str(m_VertexShader.m_FileName);
            for (string line; getline(str, line, '\n');) { 
                VertexCode += "\n" + line; 
            }
        }else { 
            VertexCode = m_VertexShader.data(); 
        }
        if (m_FragmentShader.fromFile()) {
            boost_stream_mapped_file str(m_FragmentShader.m_FileName);
            for (string line; getline(str, line, '\n');) { 
                FragmentCode += "\n" + line; 
            }
        }else { 
            FragmentCode = m_FragmentShader.data(); 
        }
        //convert the code
        _convertCode(VertexCode, FragmentCode);
        m_LoadedCPU = true;
    }
}
void ShaderProgram::_unload_CPU() {
    if (m_LoadedCPU) {
        m_LoadedCPU = false;
    }
}
void ShaderProgram::_load_GPU() {
    _unload_GPU();
    if (!m_LoadedGPU) {
        string& VertexCode = m_VertexShader.m_Code; 
        string& FragmentCode = m_FragmentShader.m_Code;
        GLuint vid = glCreateShader(GL_VERTEX_SHADER);
        GLuint fid = glCreateShader(GL_FRAGMENT_SHADER);
        GLint res = GL_FALSE;
        int ll;
        // Compile Vertex Shader
        char const* vss = VertexCode.c_str();
        glShaderSource(vid, 1, &vss, NULL);
        glCompileShader(vid);
        // Check Vertex Shader
        glGetShaderiv(vid, GL_COMPILE_STATUS, &res);
        glGetShaderiv(vid, GL_INFO_LOG_LENGTH, &ll);
        vector<char>ve(ll);
		if (ve.size() > 0) {
			glGetShaderInfoLog(vid, ll, NULL, &ve[0]);
		}
        if (res == GL_FALSE) {
            if (m_VertexShader.fromFile()) { 
                cout << "VertexShader Log (" + m_VertexShader.m_FileName + "): " << endl; 
            }else{ 
                cout << "VertexShader Log (" + m_VertexShader.name() + "): " << endl; 
            }
            cout << &ve[0] << endl;
        }
        // Compile Fragment Shader
        char const* fss = FragmentCode.c_str();
        glShaderSource(fid, 1, &fss, NULL);
        glCompileShader(fid);
        // Check Fragment Shader
        glGetShaderiv(fid, GL_COMPILE_STATUS, &res);
        glGetShaderiv(fid, GL_INFO_LOG_LENGTH, &ll);
        vector<char>fe(ll);
		if (fe.size() > 0) {
			glGetShaderInfoLog(fid, ll, NULL, &fe[0]);
		}
        if (res == GL_FALSE) {
            if (m_FragmentShader.fromFile()) { 
                cout << "FragmentShader Log (" + m_FragmentShader.m_FileName + "): " << endl; 
            }else{ 
                cout << "FragmentShader Log (" + m_FragmentShader.name() + "): " << endl; 
            }
            cout << &fe[0] << endl;
        }
        // Link the program id
        m_ShaderProgram = glCreateProgram();
        glAttachShader(m_ShaderProgram, vid);
        glAttachShader(m_ShaderProgram, fid);

        for (uint i = 0; i < 100; ++i) {
            string outFragCol = "out vec4 FRAG_COL_" + to_string(i) + ";";
            if (ShaderHelper::sfind(FragmentCode, outFragCol)) {
                glBindFragDataLocation(m_ShaderProgram, i, string("FRAG_COL_" + to_string(i)).c_str());
            }
        }

        glLinkProgram(m_ShaderProgram);
        glDetachShader(m_ShaderProgram, vid);
        glDetachShader(m_ShaderProgram, fid);
        glDeleteShader(vid);
        glDeleteShader(fid);
        // Check the program
        glGetProgramiv(m_ShaderProgram, GL_LINK_STATUS, &res);
        glGetProgramiv(m_ShaderProgram, GL_INFO_LOG_LENGTH, &ll);
        vector<char>pe(std::max(ll, static_cast<int>(1)));
        glGetProgramInfoLog(m_ShaderProgram, ll, NULL, &pe[0]);
        if (res == GL_FALSE) { 
            cout << "ShaderProgram Log : " << endl; cout << &pe[0] << endl; 
        }
        //populate uniform table
        if (res == GL_TRUE) {
            GLint _i, _count, _size;
            GLenum _type;
            const GLsizei _bufSize = 256; // maximum name length
            GLchar _name[_bufSize]; // variable name in GLSL
            GLsizei _length; // name length
            glGetProgramiv(m_ShaderProgram, GL_ACTIVE_UNIFORMS, &_count);
            for (_i = 0; _i < _count; ++_i) {
                glGetActiveUniform(m_ShaderProgram, static_cast<GLuint>(_i), _bufSize, &_length, &_size, &_type, _name);
                if (_length > 0) {
                    string _name1(static_cast<char*>(_name), _length);
                    GLint _loc = glGetUniformLocation(m_ShaderProgram, _name);
                    m_UniformLocations.emplace(_name1, _loc);
                }
            }
        }
        //link UBO's
        if (ShaderHelper::sfind(VertexCode, "layout (std140) uniform Camera //generated") || ShaderHelper::sfind(FragmentCode, "layout (std140) uniform Camera //generated")) {
            UniformBufferObject::UBO_CAMERA->attachToShader(*this);
        }
        m_LoadedGPU = true;
    }
}
void ShaderProgram::_unload_GPU() {
    if (m_LoadedGPU) {
        m_UniformLocations.clear();
        m_AttachedUBOs.clear();
        glDeleteProgram(m_ShaderProgram);
        m_LoadedGPU = false;
    }
}
const GLuint& ShaderProgram::program() const {
    return m_ShaderProgram; 
}

void epriv::InternalShaderProgramPublicInterface::LoadCPU(ShaderProgram& shaderP){
    shaderP._load_CPU();
}
void epriv::InternalShaderProgramPublicInterface::LoadGPU(ShaderProgram& shaderP){
    shaderP._load_GPU();
    shaderP.EngineResource::load();
}
void epriv::InternalShaderProgramPublicInterface::UnloadCPU(ShaderProgram& shaderP){
    shaderP._unload_CPU();
    shaderP.EngineResource::unload();
}
void epriv::InternalShaderProgramPublicInterface::UnloadGPU(ShaderProgram& shaderP){
    shaderP._unload_GPU();        
}
void ShaderProgram::load(){
    if(!isLoaded()){
        _load_CPU();
        _load_GPU();
        cout << "(Shader Program) ";
        EngineResource::load();
    }
}
void ShaderProgram::unload(){
    if(isLoaded() /*&& useCount() == 0*/){
        _unload_GPU();
        _unload_CPU();
        cout << "(Shader Program) ";
        EngineResource::unload();
    }
}
void ShaderProgram::bind(){ 
    epriv::Core::m_Engine->m_RenderManager._bindShaderProgram(this); 
}
void ShaderProgram::unbind(){ 
    epriv::Core::m_Engine->m_RenderManager._unbindShaderProgram(); 
}
const unordered_map<string,GLint>& ShaderProgram::uniforms() const { 
    return m_UniformLocations; 
}
