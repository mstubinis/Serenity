#pragma once
#ifndef ENGINE_SHADERS_HELPER_H
#define ENGINE_SHADERS_HELPER_H

namespace Engine::priv {
    class ShaderHelper final {
        public:
            static bool sfind(std::string_view whole, std::string_view part) noexcept {
                return (whole.find(part) != std::string::npos);
            }
            static void insertStringAtLine(std::string& InSrc, std::string&& newContent, unsigned int linenumber) noexcept {
                if (linenumber == 0) {
                    InSrc = std::move(newContent) + InSrc;
                }else{
                    std::stringstream str(InSrc);
                    std::string line = "";
                    std::vector<std::string> lines;
                    lines.reserve(50);
                    unsigned int count = 0;
                    while (std::getline(str, line)) {
                        lines.emplace_back(line + "\n");
                        if (count == linenumber) {
                            lines.emplace_back(std::move(newContent));
                        }
                        ++count;
                    }
                    InSrc = "";
                    for (const auto& ln : lines) {
                        InSrc += ln;
                    }
                }
            }
            static void insertStringAtAndReplaceLine(std::string& InSrc, std::string&& newContent, unsigned int linenumber) noexcept {
                std::stringstream str(InSrc);
                std::string line = "";
                std::vector<std::string> lines;
                lines.reserve(50);
                while (std::getline(str, line)) {
                    lines.emplace_back(line + "\n");
                }
                unsigned int count = 0;
                InSrc = "";
                for (auto& lineItr : lines) {
                    if (count == linenumber) {
                        lineItr = std::move(newContent);
                    }
                    InSrc += lineItr;
                    ++count;
                }
            }
            static void insertStringRightBeforeMainFunc(std::string& InSrc, std::string&& newContent) noexcept {
                std::stringstream str(InSrc);
                std::string line = "";
                std::vector<std::string> lines;
                lines.reserve(50);
                bool done = false;
                while (std::getline(str, line)) {
                    lines.emplace_back(line + "\n");
                    if (!done && ShaderHelper::sfind(line, " main(")) {
                        lines.insert(lines.cend() - 1, std::move(newContent));
                        done = true;
                    }
                }
                InSrc = "";
                for (auto& lineItr : lines) {
                    InSrc += lineItr;
                }
            }
            static void insertStringAtEndOfMainFunc(std::string& InSrc, std::string&& newContent) noexcept {
                auto position = InSrc.size() - 1;
                while (position > 0) {
                    char character = InSrc[position];
                    --position;
                    if (character == '}') {
                        break;
                    }
                }
                InSrc.insert(position, std::move(newContent));
            }
            static void insertStringRightBeforeLineContent(std::string& InSrc, std::string&& newContent, std::string_view lineContent) noexcept {
                std::stringstream str(InSrc);
                std::string line = "";
                std::vector<std::string> lines;
                lines.reserve(50);
                bool done = false;
                unsigned int count = 0;
                while (std::getline(str, line)) {
                    lines.emplace_back(line + "\n");
                    if (!done && ShaderHelper::sfind(line, lineContent)) {
                        lines.insert(lines.cend() - 1, std::move(newContent));
                        done = true;
                    }
                    ++count;
                }
                InSrc = "";
                for (auto& lineItr : lines) {
                    InSrc += lineItr;
                }
            }
            static void insertStringRightAfterLineContent(std::string& InSrc, std::string&& newContent, std::string_view lineContent) noexcept {
                std::stringstream str(InSrc);
                std::string line = "";
                std::vector<std::string> lines;
                lines.reserve(50);
                bool done = false;
                while (std::getline(str, line)) {
                    lines.emplace_back(line + "\n");
                    if (!done && ShaderHelper::sfind(line, lineContent)) {
                        lines.emplace_back(std::move(newContent));
                        done = true;
                    }
                }
                InSrc = "";
                for (auto& lineItr : lines) {
                    InSrc += lineItr;
                }
            }
    };
};

#endif