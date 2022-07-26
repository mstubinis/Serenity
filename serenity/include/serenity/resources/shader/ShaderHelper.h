#pragma once
#ifndef ENGINE_SHADERS_HELPER_H
#define ENGINE_SHADERS_HELPER_H

#include <string_view>
#include <string>

namespace Engine::priv {
    class ShaderHelper final {
        public:
            [[nodiscard]] static bool sfind(std::string_view whole, std::string_view part) noexcept;
            [[nodiscard]] static bool lacksDefinition(std::string_view inSrc, std::string_view searchedCode, std::string_view definition);

            static void insertStringAtLine(std::string& InSrc, std::string&& newContent, uint32_t linenumber) noexcept;
            static void insertStringAtAndReplaceLine(std::string& InSrc, std::string&& newContent, uint32_t linenumber) noexcept;
            static void insertStringRightBeforeMainFunc(std::string& InSrc, std::string&& newContent) noexcept;
            static void insertStringAtEndOfMainFunc(std::string& InSrc, std::string&& newContent) noexcept;
            static void insertStringRightBeforeLineContent(std::string& InSrc, std::string&& newContent, std::string_view lineContent) noexcept;
            static void insertStringRightAfterLineContent(std::string& InSrc, std::string&& newContent, std::string_view lineContent) noexcept;
    };
};

#endif