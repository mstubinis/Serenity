#include "core/engine/utils/PrecompiledHeader.h"
#include <core/engine/shaders/ShaderHelper.h>

using namespace Engine;
using namespace Engine::priv;
using namespace std;

bool ShaderHelper::sfind(string_view whole, string_view part) {
    return (whole.find(part) != string::npos);
}
void ShaderHelper::insertStringAtLine(string& InSrc, const string& newcontent, const unsigned int linenumber) {
    if (linenumber == 0) {
        InSrc = newcontent + "\n" + InSrc;
    }else{
        stringstream str(InSrc);
        string line = "";
        vector<string> lines;
        unsigned int count = 0;
        while (std::getline(str, line)) {
            lines.emplace_back(line + "\n");
            if (count == linenumber) {
                lines.emplace_back(newcontent + "\n");
            }
            ++count;
        }
        InSrc = "";
        for (const auto& ln : lines) { 
            InSrc += ln;
        }
    }
}
void ShaderHelper::insertStringAtAndReplaceLine(string& InSrc, const string& newcontent, const unsigned int linenumber) {
    stringstream str(InSrc);
    string line = "";
    vector<string> lines; 
    while (std::getline(str, line)) {
        lines.emplace_back(line + "\n");
    }
    unsigned int count = 0; 
    InSrc = "";
    for (auto& lineItr : lines) { 
        if (count == linenumber) {
            lineItr = newcontent + "\n";
        }
        InSrc += lineItr;
        ++count;
    }
}
void ShaderHelper::insertStringRightBeforeMainFunc(string& InSrc, const string& content) {
    stringstream str(InSrc);
    string line = "";
    vector<string> lines;
    bool done = false;
    while (std::getline(str, line)) {
        lines.emplace_back(line + "\n");
        if (!done && ShaderHelper::sfind(line, " main(")) {
            lines.insert(lines.cend() - 1, content + "\n");
            done = true;
        }
    }
    InSrc = "";
    for (auto& lineItr : lines) {
        InSrc += lineItr;
    }
}
void ShaderHelper::insertStringAtEndOfMainFunc(string& InSrc, const string& content) {
    auto position = InSrc.size() - 1;
    while (position > 0) {
        const char& character = InSrc[position];
        --position;
        if (character == '}') {
            break; 
        } 
    }
    InSrc.insert(position, content);
}
void ShaderHelper::insertStringRightBeforeLineContent(string& InSrc, const string& newContent, const string& lineContent) {
    stringstream str(InSrc);
    string line = "";
    vector<string> lines;
    bool done = false;
    unsigned int count = 0;
    while (std::getline(str, line)) {
        lines.emplace_back(line + "\n");
        if (ShaderHelper::sfind(line, lineContent) && !done) {
            lines.insert(lines.cend() - 1, newContent + "\n");
            done = true;
        }
        ++count;
    }
    InSrc = "";
    for (auto& lineItr : lines) {
        InSrc += lineItr;
    }
}
void ShaderHelper::insertStringRightAfterLineContent(string& InSrc, const string& newContent, const string& lineContent) {
    stringstream str(InSrc);
    string line = "";
    vector<string> lines; 
    bool done = false;
    while (std::getline(str, line)) {
        lines.emplace_back(line + "\n");
        if (ShaderHelper::sfind(line, lineContent) && !done) {
            lines.emplace_back(newContent + "\n");
            done = true;
        } 
    }
    InSrc = "";
    for (auto& lineItr : lines) {
        InSrc += lineItr;
    }
}