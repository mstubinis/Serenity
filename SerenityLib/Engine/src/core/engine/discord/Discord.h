#pragma once
#ifndef ENGINE_CORE_DISCORD_H
#define ENGINE_CORE_DISCORD_H

#include <string>
#include <core/engine/discord/src/types.h>

namespace Engine {
    namespace epriv {
        class EngineCore;
    };
    namespace Discord {
        class DiscordActivity {
            private:
                discord::Activity m_Activity;
            public:

                const discord::Activity& getActivity() const;

                void setState(const std::string& state);
                void setDetail(const std::string& detail);
                void setPartySizeCurrent(const unsigned int& currentSize);
                void setPartySizeMax(const unsigned int& maxSize);
                void setImageLarge(const std::string& largeImage);
                void setImageLargeText(const std::string& largeImageText);
                void setImageSmall(const std::string& smallImage);
                void setImageSmallText(const std::string& smallImageText);
                void setPartyID(const std::string& partyID);
                void setInstance(const bool& instance);
                void setTimestampStart(const std::int64_t start);
                void setTimestampEnd(const std::int64_t end);
                void setSecretJoin(const std::string& join);
                void setSecretSpectate(const std::string& spectate);
                void setSecretMatch(const std::string& match);
                void setType(const discord::ActivityType& type);

                DiscordActivity();
                ~DiscordActivity();
        };


        //will only update at most 5 times per 20 seconds
        void update_activity(const Engine::Discord::DiscordActivity& activity);

        const bool activate(const discord::ClientId& clientID);


        class DiscordCore {
            friend class  DiscordActivity;
            friend class  Engine::epriv::EngineCore;
            private:
                discord::ClientId  m_ClientID;
                bool               m_LoggingErrors;
                bool               m_Active;


                static const std::string get_result_string_message(const discord::Result& result);
                void update();
            public:
                DiscordCore();
                ~DiscordCore();

                const bool activate(const discord::ClientId& clientID);

                void update_activity(const discord::Activity& activity);

                void log_errors(const bool = true);
        };
    };
};

#endif