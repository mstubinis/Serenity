#if !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <serenity/discord/src/application_manager.h>
#include <serenity/discord/src/core.h>
#include <serenity/system/Macros.h>

#include <cstring>
#include <memory>

namespace discord {

void ApplicationManager::ValidateOrExit(std::function<void(Result)> callback)
{
    static auto wrapper = [](void* callbackData, EDiscordResult result) -> void {
        std::unique_ptr<std::function<void(Result)>> cb(
          reinterpret_cast<std::function<void(Result)>*>(callbackData));
        if (!cb || !(*cb)) {
            return;
        }
        (*cb)(static_cast<Result>(result));
    };
    std::unique_ptr<std::function<void(Result)>> cb{};
    cb.reset(NEW std::function<void(Result)>(std::move(callback)));
    internal_->validate_or_exit(internal_, cb.release(), wrapper);
}

void ApplicationManager::GetCurrentLocale(char locale[128])
{
    if (!locale) {
        return;
    }

    internal_->get_current_locale(internal_, reinterpret_cast<DiscordLocale*>(locale));
}

void ApplicationManager::GetCurrentBranch(char branch[4096])
{
    if (!branch) {
        return;
    }

    internal_->get_current_branch(internal_, reinterpret_cast<DiscordBranch*>(branch));
}

void ApplicationManager::GetOAuth2Token(std::function<void(Result, OAuth2Token const&)> callback)
{
    static auto wrapper =
      [](void* callbackData, EDiscordResult result, DiscordOAuth2Token* oauth2Token) -> void {
        std::unique_ptr<std::function<void(Result, OAuth2Token const&)>> cb(
          reinterpret_cast<std::function<void(Result, OAuth2Token const&)>*>(callbackData));
        if (!cb || !(*cb)) {
            return;
        }
        (*cb)(static_cast<Result>(result), *reinterpret_cast<OAuth2Token const*>(oauth2Token));
    };
    std::unique_ptr<std::function<void(Result, OAuth2Token const&)>> cb{};
    cb.reset(NEW std::function<void(Result, OAuth2Token const&)>(std::move(callback)));
    internal_->get_oauth2_token(internal_, cb.release(), wrapper);
}

void ApplicationManager::GetTicket(std::function<void(Result, const char*)> callback)
{
    static auto wrapper = [](void* callbackData, EDiscordResult result, const char* data) -> void {
        std::unique_ptr<std::function<void(Result, const char*)>> cb(
          reinterpret_cast<std::function<void(Result, const char*)>*>(callbackData));
        if (!cb || !(*cb)) {
            return;
        }
        (*cb)(static_cast<Result>(result), static_cast<const char*>(data));
    };
    std::unique_ptr<std::function<void(Result, const char*)>> cb{};
    cb.reset(NEW std::function<void(Result, const char*)>(std::move(callback)));
    internal_->get_ticket(internal_, cb.release(), wrapper);
}

} // namespace discord
