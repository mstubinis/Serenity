#pragma once
#ifndef ENGINE_CORE_DISCORD_H
#define ENGINE_CORE_DISCORD_H

#include <string>
#include <core/engine/discord/src/types.h>

namespace Engine {
    namespace priv {
        class EngineCore;
    };
    namespace Discord {
        class DiscordActivity {
            private:
                discord::Activity m_Activity;
            public:

                const discord::Activity& getActivity() const;

                //the player's current party status
                void setState(const std::string& state);

                //what the player is currently doing
                void setDetail(const std::string& detail);

                //the current size of the party
                void setPartySizeCurrent(const unsigned int& currentSize);

                //the max possible size of the party
                void setPartySizeMax(const unsigned int& maxSize);

                //keyname of an asset to display
                void setImageLarge(const std::string& largeImage);

                //hover text for the large image
                void setImageLargeText(const std::string& largeImageText);

                //keyname of an asset to display
                void setImageSmall(const std::string& smallImage);

                //hover text for the small image
                void setImageSmallText(const std::string& smallImageText);

                //a unique identifier for this party
                void setPartyID(const std::string& partyID);

                //whether this activity is an instanced context, like a match
                void setInstance(const bool& instance);

                //unix timestamp - set the current starting activity time to this very moment
                void setTimestampStartAsNow();

                //unix timestamp - send this to have an "elapsed" timer
                void setTimestampStart(const std::int64_t start);

                //specify how many miliseconds from this very moment that the activity will end
                void setTimestampEndSinceNow(const std::int64_t end);

                //specify how many seconds from this very moment that the activity will end
                void setTimestampEndSinceNowSeconds(const unsigned int& end);

                //unix timestamp - send this to have a "remaining" timer
                void setTimestampEnd(const std::int64_t end);

                //unique hash for chat invites and Ask to Join
                void setSecretJoin(const std::string& join);

                //unique hash for Spectate button
                void setSecretSpectate(const std::string& spectate);

                //unique hash for the given match context
                void setSecretMatch(const std::string& match);

                //ActivityType is strictly for the purpose of handling events that you receive from Discord; though the SDK/our API will not reject a payload with an ActivityType sent, it will be discarded and will not change anything in the client
                void setType(const discord::ActivityType& type);

                DiscordActivity();
                ~DiscordActivity();
        };


        //will only update at most 5 times per 20 seconds
        void update_activity(const Engine::Discord::DiscordActivity& activity);
        void clear_activity();

        const bool activate(const discord::ClientId& clientID);


        class DiscordCore {
            friend class  DiscordActivity;
            friend class  Engine::priv::EngineCore;
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
                void clear_activity();

                void log_errors(const bool = true);
        };
    };
};

#endif