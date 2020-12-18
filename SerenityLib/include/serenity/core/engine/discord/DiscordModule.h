#pragma once
#ifndef ENGINE_DISCORD_MODULE_H
#define ENGINE_DISCORD_MODULE_H

class DiscordActivityEvent;
namespace discord {
    class Core;
};

#include <serenity/core/engine/discord/src/types.h>
#include <string>

namespace Engine::priv {
    class DiscordModule {
        private:
            discord::Core*     m_DiscordCore   = nullptr;
            discord::ClientId  m_ClientID      = 0;
            bool               m_LoggingErrors = false;
            bool               m_Active        = false;

            std::string internal_get_result_string_message(const discord::Result& result);
        public:
            DiscordModule();
            virtual ~DiscordModule();

            inline constexpr discord::ClientId get_client_id() const noexcept { return m_ClientID; }
            bool activate(discord::ClientId clientID);
            bool update_activity(const discord::Activity& activity);
            bool clear_activity();
            void log_errors(bool logErrors = true);
            bool update();

    };
};
namespace Engine::Discord {
    //will only update at most 5 times per 20 seconds
    discord::ClientId getClientID();
    bool update_activity(const DiscordActivityEvent& activity);
    bool clear_activity();
    bool activate(discord::ClientId clientID);
};

#endif