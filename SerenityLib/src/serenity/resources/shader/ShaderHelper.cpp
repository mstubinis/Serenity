#include <serenity/resources/shader/ShaderHelper.h>
#include <sstream>
#include <vector>
#include <serenity/utils/Utils.h>

bool Engine::priv::ShaderHelper::sfind(std::string_view whole, std::string_view part) noexcept {
    return whole.find(part) != std::string::npos;
}
bool Engine::priv::ShaderHelper::lacksDefinition(std::string_view inSrc, std::string_view searchedCode, std::string_view definition) {
    return ShaderHelper::sfind(inSrc, searchedCode) && !ShaderHelper::sfind(inSrc, definition);
}
void Engine::priv::ShaderHelper::insertStringAtLine(std::string& InSrc, std::string&& newContent, uint32_t linenumber) noexcept {
    if (linenumber == 0) {
        InSrc = std::move(newContent) + InSrc;
    } else {
        std::stringstream str{ InSrc };
        std::string line;
        auto lines = Engine::create_and_reserve<std::vector<std::string>>(50);
        uint32_t count = 0;
        while (std::getline(str, line)) {
            lines.emplace_back(line + "\n");
            if (count == linenumber) {
                lines.emplace_back(std::move(newContent));
            }
            ++count;
        }
        InSrc.clear();
        for (const auto& ln : lines) {
            InSrc += ln;
        }
    }
}
void Engine::priv::ShaderHelper::insertStringAtAndReplaceLine(std::string& InSrc, std::string&& newContent, uint32_t linenumber) noexcept {
    std::stringstream str{ InSrc };
    std::string line;
    auto lines = Engine::create_and_reserve<std::vector<std::string>>(50);
    while (std::getline(str, line)) {
        lines.emplace_back(line + "\n");
    }
    uint32_t count = 0;
    InSrc.clear();
    for (auto& lineItr : lines) {
        if (count == linenumber) {
            lineItr = std::move(newContent);
        }
        InSrc += lineItr;
        ++count;
    }
}
void Engine::priv::ShaderHelper::insertStringRightBeforeMainFunc(std::string& InSrc, std::string&& newContent) noexcept {
    std::stringstream str(InSrc);
    std::string line;
    auto lines = Engine::create_and_reserve<std::vector<std::string>>(50);
    bool done = false;
    while (std::getline(str, line)) {
        lines.emplace_back(line + "\n");
        if (!done && ShaderHelper::sfind(line, " main(")) {
            lines.insert(lines.cend() - 1, std::move(newContent));
            done = true;
        }
    }
    InSrc.clear();
    for (auto& lineItr : lines) {
        InSrc += lineItr;
    }
}
void Engine::priv::ShaderHelper::insertStringAtEndOfMainFunc(std::string& InSrc, std::string&& newContent) noexcept {
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
void Engine::priv::ShaderHelper::insertStringRightBeforeLineContent(std::string& InSrc, std::string&& newContent, std::string_view lineContent) noexcept {
    std::stringstream str{ InSrc };
    std::string line;
    auto lines = Engine::create_and_reserve<std::vector<std::string>>(50);
    bool done = false;
    uint32_t count = 0;
    while (std::getline(str, line)) {
        lines.emplace_back(line + "\n");
        if (!done && ShaderHelper::sfind(line, lineContent)) {
            lines.insert(lines.cend() - 1, std::move(newContent));
            done = true;
        }
        ++count;
    }
    InSrc.clear();
    for (auto& lineItr : lines) {
        InSrc += lineItr;
    }
}
void Engine::priv::ShaderHelper::insertStringRightAfterLineContent(std::string& InSrc, std::string&& newContent, std::string_view lineContent) noexcept {
    std::stringstream str{ InSrc };
    std::string line;
    auto lines = Engine::create_and_reserve<std::vector<std::string>>(50);
    bool done = false;
    while (std::getline(str, line)) {
        lines.emplace_back(line + "\n");
        if (!done && ShaderHelper::sfind(line, lineContent)) {
            lines.emplace_back(std::move(newContent));
            done = true;
        }
    }
    InSrc.clear();
    for (auto& lineItr : lines) {
        InSrc += lineItr;
    }
}