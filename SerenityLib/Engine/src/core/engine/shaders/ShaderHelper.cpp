#include <core/engine/shaders/ShaderHelper.h>

#include <vector>
#include <sstream>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

const bool ShaderHelper::sfind(const string& whole, const string& part) {
    return whole.find(part) != string::npos ? true : false;
}
void ShaderHelper::insertStringAtLine(string& src, const string& newcontent, const unsigned int linenumber) {
    if (linenumber == 0) {
        src = newcontent + "\n" + src; 
    }else{
        istringstream str(src);
        string line;
        vector<string> lines;
        unsigned int count = 0;
        while (getline(str, line)) {
            lines.push_back(line + "\n");
            if (count == linenumber) {
                lines.push_back(newcontent + "\n"); 
            }
            ++count;
        }
        src = ""; 
        for (auto& ln : lines) { 
            src += ln; 
        }
    }
}
void ShaderHelper::insertStringAtAndReplaceLine(string& src, const string& newcontent, const unsigned int linenumber) {
    istringstream str(src); 
    string line;
    vector<string> lines; 
    while (getline(str, line)) {
        lines.push_back(line + "\n");
    }
    unsigned int count = 0; 
    src = ""; 
    for (auto& lineItr : lines) { 
        if (count == linenumber)
            lineItr = newcontent + "\n";
        src += lineItr;
        ++count;
    }
}
void ShaderHelper::insertStringAtEndOfMainFunc(string& src, const string& content) {
    auto position = src.size() - 1; 
    while (position > 0) {
        const char& character = src[position];
        --position;
        if (character == '}') {
            break; 
        } 
    }
    src.insert(position, content);
}
void ShaderHelper::insertStringRightBeforeLineContent(string& src, const string& newContent, const string& lineContent) {
    istringstream str(src);
    string line;
    vector<string> lines;
    bool done = false;
    unsigned int count = 0;
    while (getline(str, line)) {
        lines.push_back(line + "\n");
        if (ShaderHelper::sfind(line, lineContent) && !done) {
            lines.insert(lines.cend() - 1, newContent + "\n");
            done = true;
        }
        ++count;
    }
    src = "";
    for (auto& lineItr : lines) {
        src += lineItr;
    }
}
void ShaderHelper::insertStringRightAfterLineContent(string& src, const string& newContent, const string& lineContent) {
    istringstream str(src); 
    string line;
    vector<string> lines; 
    bool done = false;
    while (getline(str, line)) {
        lines.push_back(line + "\n");
        if (ShaderHelper::sfind(line, lineContent) && !done) {
            lines.push_back(newContent + "\n"); 
            done = true;
        } 
    }
    src = ""; 
    for (auto& lineItr : lines) {
        src += lineItr;
    }
}