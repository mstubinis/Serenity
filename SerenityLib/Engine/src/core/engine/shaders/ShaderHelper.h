#pragma once
#ifndef ENGINE_SHADERS_HELPER_H
#define ENGINE_SHADERS_HELPER_H

namespace Engine::priv {
    class ShaderHelper final {
        public:
            static bool sfind(std::string_view whole, std::string_view part);

            static void insertStringRightBeforeLineContent(std::string& InSrc, const std::string& newContent, const std::string& lineContent);
            static void insertStringAtLine(std::string& InSrc, const std::string& newcontent, const unsigned int line);
            static void insertStringAtAndReplaceLine(std::string& InSrc, const std::string& newcontent, const unsigned int line);
            static void insertStringAtEndOfMainFunc(std::string& InSrc, const std::string& content);
            static void insertStringRightAfterLineContent(std::string& InSrc, const std::string& newContent, const std::string& lineContent);
    };
};

#endif