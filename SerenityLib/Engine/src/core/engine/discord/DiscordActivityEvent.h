#pragma once
#ifndef ENGINE_DISCORD_ACTIVITY_EVENT_H
#define ENGINE_DISCORD_ACTIVITY_EVENT_H

#include <core/engine/discord/src/types.h>

class DiscordActivityEvent {
    private:
        discord::Activity      m_Activity;
    public:
        DiscordActivityEvent();
        virtual ~DiscordActivityEvent();

        bool post();

        const discord::Activity& getActivity() const;

        void setState(const std::string& state); //the player's current party status

        void setDetail(const std::string& detail); //what the player is currently doing

        void setPartySizeCurrent(unsigned int currentSize); //the current size of the party

        void setPartySizeMax(unsigned int maxSize); //the max possible size of the party

        void setImageLarge(const std::string& largeImage); //keyname of an asset to display

        void setImageLargeText(const std::string& largeImageText); //hover text for the large image

        void setImageSmall(const std::string& smallImage); //keyname of an asset to display

        void setImageSmallText(const std::string& smallImageText); //hover text for the small image

        void setPartyID(const std::string& partyID); //a unique identifier for this party

        void setInstance(bool instance); //whether this activity is an instanced context, like a match

        void setTimestampStartAsNow(); //unix timestamp - set the current starting activity time to this very moment

        void setTimestampStart(std::int64_t start); //unix timestamp - send this to have an "elapsed" timer

        void setTimestampEndSinceNow(std::int64_t end); //specify how many miliseconds from this very moment that the activity will end

        void setTimestampEndSinceNowSeconds(unsigned int end); //specify how many seconds from this very moment that the activity will end

        void setTimestampEnd(std::int64_t end); //unix timestamp - send this to have a "remaining" timer

        void setSecretJoin(const std::string& join); //unique hash for chat invites and Ask to Join

        void setSecretSpectate(const std::string& spectate); //unique hash for Spectate button

        void setSecretMatch(const std::string& match); //unique hash for the given match context

        void setType(const discord::ActivityType& type); //ActivityType is strictly for the purpose of handling events that you receive from Discord; though the SDK/our API will not reject a payload with an ActivityType sent, it will be discarded and will not change anything in the client
};


#endif