#pragma once
#ifndef ENGINE_DISCORD_ACTIVITY_EVENT_H
#define ENGINE_DISCORD_ACTIVITY_EVENT_H

#include <core/engine/discord/src/types.h>

class DiscordActivityEvent {
    private:
        discord::Activity      m_Activity;
    public:
        DiscordActivityEvent();
        virtual ~DiscordActivityEvent() = default;

        bool post() noexcept;

        const discord::Activity& getActivity() const noexcept;

        void setState(const char* state) noexcept; //the player's current party status

        void setDetail(const char* detail) noexcept; //what the player is currently doing

        void setPartySizeCurrent(unsigned int currentSize) noexcept; //the current size of the party

        void setPartySizeMax(unsigned int maxSize) noexcept; //the max possible size of the party

        void setImageLarge(const char* largeImage) noexcept; //keyname of an asset to display

        void setImageLargeText(const char* largeImageText) noexcept; //hover text for the large image

        void setImageSmall(const char* smallImage) noexcept; //keyname of an asset to display

        void setImageSmallText(const char* smallImageText) noexcept; //hover text for the small image

        void setPartyID(const char* partyID) noexcept; //a unique identifier for this party

        void setInstance(bool instance) noexcept; //whether this activity is an instanced context, like a match

        void setTimestampStartAsNow() noexcept; //unix timestamp - set the current starting activity time to this very moment

        void setTimestampStart(std::int64_t start) noexcept; //unix timestamp - send this to have an "elapsed" timer

        void setTimestampEndSinceNow(std::int64_t end) noexcept; //specify how many miliseconds from this very moment that the activity will end

        void setTimestampEndSinceNowSeconds(unsigned int end) noexcept; //specify how many seconds from this very moment that the activity will end

        void setTimestampEnd(std::int64_t end) noexcept; //unix timestamp - send this to have a "remaining" timer

        void setSecretJoin(const char* join) noexcept; //unique hash for chat invites and Ask to Join

        void setSecretSpectate(const char* spectate) noexcept; //unique hash for Spectate button

        void setSecretMatch(const char* match) noexcept; //unique hash for the given match context

        void setType(const discord::ActivityType& type) noexcept; //ActivityType is strictly for the purpose of handling events that you receive from Discord; though the SDK/our API will not reject a payload with an ActivityType sent, it will be discarded and will not change anything in the client
};


#endif