#pragma once
#ifndef ENGINE_SHADERS_HELPER_H
#define ENGINE_SHADERS_HELPER_H

#include <string>

namespace Engine {
    namespace epriv {
        class ShaderHelper final {
            public:
                static const bool sfind(const std::string& whole, const std::string& part);
                static void insertStringAtLine(std::string& src, const std::string& newcontent, const unsigned int line);
                static void insertStringAtAndReplaceLine(std::string& src, const std::string& newcontent, const unsigned int line);
                static void insertStringAtEndOfMainFunc(std::string& src, const std::string& content);
                static void insertStringRightAfterLineContent(std::string& src, const std::string& newContent, const std::string& lineContent);
        };
    };
};

#endif