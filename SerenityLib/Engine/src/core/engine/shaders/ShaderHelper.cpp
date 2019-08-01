#include <core/engine/shaders/ShaderHelper.h>

#include <vector>
#include <sstream>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

const bool ShaderHelper::sfind(const string& whole, const string& part) {
    return whole.find(part) != string::npos ? true : false;
}
void ShaderHelper::insertStringAtLine(string& src, const string& newcontent, const unsigned int line) {
    if (line == 0) { src = newcontent + "\n" + src; }
    else {
        istringstream str(src); string l; vector<string> lines; unsigned int c = 0;
        while (getline(str, l)) { lines.push_back(l + "\n"); if (c == line) { lines.push_back(newcontent + "\n"); }++c; }src = ""; for (auto& ln : lines) { src += ln; }
    }
}
void ShaderHelper::insertStringAtAndReplaceLine(string& src, const string& newcontent, const unsigned int line) {
    istringstream str(src); string l; vector<string> lines; while (getline(str, l)) { lines.push_back(l + "\n"); }
    unsigned int c = 0; src = ""; for (auto& ln : lines) { if (c == line)ln = newcontent + "\n"; src += ln; ++c; }
}
void ShaderHelper::insertStringAtEndOfMainFunc(string& src, const string& content) {
    unsigned int p = src.size() - 1; while (p > 0) { char c = src[p]; --p; if (c == '}') { break; } }src.insert(p, content);
}
void ShaderHelper::insertStringRightAfterLineContent(string& src, const string& newContent, const string& lineContent) {
    istringstream str(src); string l; vector<string> lines; bool a = false;
    while (getline(str, l)) { lines.push_back(l + "\n"); if (sfind(l, lineContent) && !a) { lines.push_back(newContent + "\n"); a = true; } }src = ""; for (auto& ln : lines) { src += ln; }
}