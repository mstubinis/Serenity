#include <serenity/renderer/text/TextStringBuilderColor.h>
#include <iomanip>
#include <sstream>

namespace {
    std::stringstream STR_STREAM;
}

TextStringBuilderColor::operator std::string() const {
    std::string res;
    res.reserve(10 + m_Msg.size());
    res += static_cast<char>(TextStringBuilderColorToken::Start);
    for (glm::length_t i = 0; i < m_Color.length(); ++i) {
        STR_STREAM.str({});
        STR_STREAM.clear();
        STR_STREAM << std::hex << std::setw(2) << std::setfill('0') << (unsigned)m_Color[i];
        res += STR_STREAM.str();
    }
    res += m_Msg;
    res += static_cast<char>(TextStringBuilderColorToken::End);
    return res;
}