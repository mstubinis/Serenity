#include <serenity/renderer/text/TextStringBuilderColor.h>
#include <iomanip>
#include <sstream>


TextStringBuilderColor::operator std::string() {
    std::string res;
    res.reserve(10 + m_Msg.size());
    res += static_cast<char>(TextStringBuilderColorToken::Start);
    for (int i = 0; i < 4; ++i) {
        std::stringstream strm;
        strm << std::hex << std::setw(2) << std::setfill('0') << (unsigned)m_Color[i];
        res += strm.str();
    }
    res += m_Msg;
    res += static_cast<char>(TextStringBuilderColorToken::End);
    return res;
}