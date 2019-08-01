#include <core/engine/shaders/Shader.h>

using namespace std;

Shader::Shader(const string& filenameOrCode, const ShaderType::Type& shaderType, const bool& fromFile) {
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
Shader::~Shader() {
}
const ShaderType::Type& Shader::type() const {
    return m_Type;
}
const string& Shader::data() const {
    return m_Code;
}
const bool& Shader::fromFile() const {
    return m_FromFile;
}